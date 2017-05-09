#include "battery.h"
#include "sigfox.h"
#include <math.h>

static uint8_t ad_board_setup();
static void set_acq_time(int32_t volt, double *value);
static uint8_t check_battery(int32_t volt);
static void battery_cleanup(void * arg);
static void stats(float volt, float min_volt, float max_volt);

typedef struct cleanup_args{
	FILE * fp;
	_Atomic uint8_t *alive;
} cleanup_args;

static void battery_cleanup(void * arg)
{
	struct cleanup_args *args = arg;
	printf("battery cleanup");
	bcm2835_spi_end();
	end_program = 1;
	fclose(args->fp);
	*args->alive = 0;
}

/* Starting point for the thread managing power of the system
 * Use the ADS1256 interface to capture voltage
 * Using the configuration file, decide when to shutdown the raspberry pi 
 * and update the config
*/
void *battery(void *arg)
{

	int32_t volt = -1;
	int32_t adc;
	int32_t i;
	FILE *fp;
	printf("Battery thread ID : %li\n", syscall(__NR_gettid));
	time_t t;
	//struct timespec tt;
	//tt.tv_sec = 0;
	//tt.tv_nsec = (long) 1000000000.0/MEASURE_FREQUENCY;
	cleanup_args args;
	args.alive = arg;
	*(args.alive) = 1;
	fp = fopen(PATH_VOLT_LOGS, "a+");
	args.fp = fp;
	
	pthread_cleanup_push(battery_cleanup, (void*) &args);

	if(!ad_board_setup())
	{
		printf("init failed\n");
		pthread_exit((void *) 0);
	}
	while(volt == -1 || check_battery(volt) )
	{
		adc = 0;
		for (i = 0;i< NB_MEASURES; i++)
		{
			while((ADS1256_Scan() == 0));
			adc += (int32_t) (double)ADS1256_GetAdc(CH_NUM)
			       		/ (double) NB_MEASURES;
		}
		volt = (adc * 100) / 167;
		t = time(NULL);
		fprintf(fp, "%s, %i.%03i %03i V \r\n", ctime(&t), 
				volt /1000000, (volt%1000000)/1000, volt%1000); 
		fflush(fp); // To fprint immediately
		if(volt < 0)
			volt = -volt;
		sleep(INTERVAL);
	}
	
//	set_next_startup(100); //@TODO : 100 for testing
	pthread_cleanup_pop(1);
	pthread_exit((void *) 1);
}

void stats(float volt, float min_volt, float max_volt)
{
	pthread_t thread;
	static uint32_t count;
	static float min, max, sum, sum_square;
	struct sgf_data data_to_send = {
		.id = AD_CONVERTER };

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);


	static time_t new_cycle;
	if(!new_cycle)
		new_cycle = time(NULL);

	if(volt > max)
		max = volt;
	else if (volt < min)
		min = volt;
	sum += volt;
	sum_square += volt;
	count++;

	if(difftime(time(NULL), new_cycle) > SGF_SEND_PERIOD/10)
	{
		data_to_send.min = min;
		data_to_send.max = max;
		data_to_send.mean = (sum/count - min_volt) /
			(max_volt - min_volt);
		data_to_send.std_dev = sqrt(sum_square/count - 
				data_to_send.mean*data_to_send.mean);
		pthread_create(&thread, &attr, send_sigfox, (void*) &data_to_send);
		sum = 0;
		sum_square = 0;
		count = 0;
	}

	pthread_attr_destroy(&attr);
}

/*
 * check_battery : Read and update config to decide whether to shutdown or not
 * Param : The measured voltage
 * Return : 0 if system should be shutdown, 1 otherwise
 */
static uint8_t check_battery(int32_t volt)
{
	static double value[NB_CFG_BATTERY] = {0,0,0,0,0};
	static int32_t start_volt = 0;
	static time_t start_time = 0;
	static double treshold;

	if(start_volt == 0)
		start_volt = volt;

	if(!start_time)
	{
		printf("start time init\n");
		time(&start_time);
	}
	
	//Initialize value[] with the variables from the configuration file
	if (value[MAX_VOLT] == 0)
	{
		//Order of string here should be same order as in cfg.h enum
		char *str[NB_CFG_BATTERY] = {"MAX_VOLT", "MIN_VOLT",
			"THRESHOLD", "LAST_DISCHARGE","ACQ_TIME"};
		get_cfg((double *) value, str,  NB_CFG_BATTERY);
		
		// Uncomment to have duration of acquisition dependent on 
		// last acquisition :
		//set_acq_time(volt, value);
		treshold = value[MIN_VOLT] + 
			(value[MAX_VOLT] - value[MAX_VOLT])*value[THRESHOLD];
	}

	// Update configuration MAX_VALUE if actual voltage is >	
	else if (volt/1000000 > value[MAX_VOLT]) 
	{
		value[MAX_VOLT] = (double) volt/1000000;
		char *str[1] = {"MAX_VOLT"};
		set_cfg(str, &value[MAX_VOLT], 1); 
	}

	//If voltage is less than fixed threshold or acq_time has been reached,
	//save config and shutdown the raspberry pi
	if (volt/1000000 < treshold
			|| difftime(time(NULL), start_time) > value[ACQ_TIME])
	{
		char *str[2] = {"ACQ_TIME", "LAST_DISCHARGE"};
		double tmp_value[2];
		tmp_value[0] = difftime(time(NULL), start_time);
		tmp_value[1] = (volt - start_volt)/1000000;
		set_cfg(str, tmp_value, 2);
		set_next_startup((int32_t) (24*3600-tmp_value[0]));
		return 0;
	}

	stats(volt / 1000000, value[MIN_VOLT], value[MAX_VOLT]);
	return 1;
}


//@TODO : Check what happens when the program runs 24h
/* Function : Change how long should the raspberry pi be kept powered depending
 * 		on last acquisition duration and discharge
 * Params : measured voltage, configuration values
*/
static void set_acq_time(int32_t volt, double *value)
{
	char *str[1] = {"ACQ_TIME"};
	double expected_volt = volt/1000000 -
	       		       value[LAST_DISCHARGE] - 
			       value[THRESHOLD]*value[MAX_VOLT];
	if(expected_volt > 0)
		value[ACQ_TIME] = value[ACQ_TIME] * 1.1;
	else if(expected_volt <  0)
		value[ACQ_TIME] = value[ACQ_TIME] * 0.9;
	else if(expected_volt < -2)  	//@TODO : 2 placeholder for hard cap
		value[ACQ_TIME] = value[ACQ_TIME] * 1.1;

	set_cfg(str, &value[ACQ_TIME], 1);
}

/* Function : Setup the options for SPI communications, and start scanning 
 * 		the analog board
 * Return : 0 if failure, 1 otherwise
*/
static uint8_t ad_board_setup()
{

	input_type input = SINGLE;
	if (!bcm2835_spi_begin())
		return 0;
	bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_LSBFIRST );//default
	bcm2835_spi_setDataMode(BCM2835_SPI_MODE1);              //default
	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_1024); //default
	bcm2835_gpio_fsel(SPICS, BCM2835_GPIO_FSEL_OUTP);//
	bcm2835_gpio_write(SPICS, HIGH);
	bcm2835_gpio_fsel(DRDY, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_set_pud(DRDY, BCM2835_GPIO_PUD_UP);    	

	int id = ADS1256_ReadChipID();

	printf("ASD1256 Chip ID = 0x%d\r\n", (int)id);

	printf("Acquisition started on channel %d\n", (int)g_tADS1256.Channel);

	ADS1256_CfgADC(ADS1256_GAIN_1, ADS1256_15SPS);
	ADS1256_StartScan(input, CH_NUM);

	return 1;	
}
