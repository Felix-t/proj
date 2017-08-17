#include "battery.h"
#include "sigfox.h"
#include <math.h>

static uint8_t ad_board_setup();
//static void set_acq_time(int32_t volt, double *value);
static uint8_t check_battery(int32_t volt);
static void battery_cleanup(void * arg);
static void stats(float volt, float min_volt, float max_volt);

typedef struct cleanup_args{
	FILE * fp;
} cleanup_args;


static void battery_cleanup(void * arg)
{
	struct cleanup_args *args = arg;
	printf("Battery cleanup\n");
	bcm2835_spi_end();
	end_program = 1;
	fclose(args->fp);
	alive[AD_CONVERTER] = 0;
}


/* Thread :
 *  - Use the ADS1256 interface to capture voltage, keep statistics
 *  - Depending on the configuration and the voltage, decide when to shutdown
 *   the raspberry pi 
 * Param : not used
*/
void *battery(void *arg)
{
	printf("Battery thread ID : %li\n", syscall(__NR_gettid));

	int32_t i, adc, volt = -1;
	time_t t;

	alive[AD_CONVERTER] = 1;
	cleanup_args args;
	args.fp = fopen(PATH_VOLT_LOGS, "a+");
	
	pthread_cleanup_push(battery_cleanup, (void*) &args);

	if(!ad_board_setup())
	{
		printf("init failed\n");
		pthread_exit((void *) 0);
	}
	do
	{
		// Check battery immediately at start
		if(volt != -1)
			sleep(INTERVAL);
		adc = 0;
		for (i = 0;i< NB_MEASURES; i++)
		{
            // Scan() to make the converter update the data,
            // GetAdc to read the data
			while((ADS1256_Scan() == 0));
			adc += (int32_t) (double)ADS1256_GetAdc(CH_NUM)
			       		/ (double) NB_MEASURES;
		}

		volt = (adc * 100) / 167;
		t = time(NULL);
		fprintf(args.fp, "%s, %i.%03i %03i V \r\n", ctime(&t), 
				volt /1000000, (volt%1000000)/1000, volt%1000); 
		fflush(args.fp); // To fprint immediately
		if(volt < 0)
			volt = -volt;
		
	}while(check_battery(volt));
	
	pthread_cleanup_pop(1);
	pthread_exit((void *) 1);
}


/* Function : 
 *  - Calculates the minimum, maximum, mean and standard 
 *  deviation of the successive volt values passed.
 *  - Every SGF_INTERVAL, send these statistics to the sigfox thread, then
 *  reset them
 * Params : The measured volt value,
 *          min_volt and max_volt are the lowest and highest possible voltage 
 *          These two values are used to calculate the mean in percentage.
 */
static void stats(float volt, float min_volt, float max_volt)
{
	pthread_t thread;
	static uint32_t count = 0;
	static float min = 0, max = 0, sum = 0, sum_square = 0;
	static struct sgf_data data_to_send = {
		.id = AD_CONVERTER };

    // Battery data is not sent to sigfox immediately, as the sigfox may not
    // yet be initialised 
    // We start sending data 120s after stats() is first called 
	static time_t new_cycle = 0;
	if(!new_cycle)
		new_cycle = time(NULL) - SGF_INTERVAL + 120;

    // Update min, max sum.
	if(volt > max)
		max = volt;
	else if (volt < min)
		min = volt;
	sum += volt;
	sum_square += volt;
	count++;

	if(difftime(time(NULL), new_cycle) > SGF_INTERVAL)
	{
		data_to_send.min = min;
		data_to_send.max = max;
        
        // Data to send is a percentage between min_volt and max_volt, 
        // and cap at 255/255 (100%)
		data_to_send.mean = 255.0 * (float) ( sum/((float)count) - min_volt) /
			(float) (max_volt - min_volt);
		data_to_send.mean = data_to_send.mean > 255.0 ?
		       	data_to_send.mean : 255.0;

		data_to_send.std_dev = sqrt(sum_square/count - 
				data_to_send.mean*data_to_send.mean);

        // Attributes to create our threads detached, so that we don't have to 
        // manually do it
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        
        if(alive[SGF] == 1)
            pthread_create(&thread, &attr, send_sigfox, (void*) &data_to_send);

        pthread_attr_destroy(&attr);

		sum = 0;
		sum_square = 0;
		count = 0;
		new_cycle = time(NULL);
		min = 16000.0;
		max = -16000.0;
	}
}


/*
 * Function : Read and update config to decide whether to shutdown the program
 *          or not
 * Param : The measured voltage
 * Return : 0 if system should be shutdown, 1 otherwise
 */
static uint8_t check_battery(int32_t volt)
{
	static double value[NB_CFG_BATTERY] = {0,0,0,0,0};
	static int32_t start_volt = 0;
	static time_t start_time = 0;
	static double threshold = 0.0;

	if(!start_volt)
		start_volt = volt;

	if(!start_time)
		time(&start_time);

	//Initialize value[] with the variables from the configuration file
	if (value[MAX_VOLT] == 0)
	{
		//Order of string here should be same order as in cfg.h enum
		char *str[NB_CFG_BATTERY] = {"MAX_VOLT", "MIN_VOLT",
			"THRESHOLD", "LAST_DISCHARGE","ACQ_TIME"};
		get_cfg(value, str,  NB_CFG_BATTERY);

        threshold = value[MIN_VOLT] + 
			(value[MAX_VOLT] - value[MIN_VOLT])*value[THRESHOLD];

        printf("Configuration read in file :\n");
		printf("\tmin : %f\n\tmax : %f\n\tthreshold : %f\n",
                value[MIN_VOLT], value[MAX_VOLT], threshold);
	}

	// If voltage is less than fixed threshold or acq_time has been reached,
	// save config and shutdown the raspberry pi
	if ((volt/1000000.0) < threshold
			|| difftime(time(NULL), start_time) > value[ACQ_TIME])
	{
		printf("Difference entre les deux : %f\n", threshold - (volt/1000000));
		char *str[2] = {"ACQ_TIME", "LAST_DISCHARGE"};
		double tmp_value[2];

		tmp_value[0] = value[ACQ_TIME];

		tmp_value[1] = (volt - start_volt)/1000000.0;
		set_cfg(str, tmp_value, 2);

		return 0;
	}

	stats(volt / 1000000.0, 3, 4);
	return 1;
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


/* Function : Change how long should the raspberry pi be kept powered depending
 * 		on last acquisition duration and discharge
 * Params : measured voltage, configuration values
*/
/*
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
*/


