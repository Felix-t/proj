#include "include.h"

#include "battery.h"
#include "cfg.h"
#include "util.h"
#include "ADS1256.h"
#include "bcm2835.h"

static uint8_t ad_board_setup();
static void set_acq_time(int32_t volt, double *value);
static uint8_t check_battery(int32_t volt);

/* Starting point for the thread managing power of the system
 * Use the ADS1256 interface to capture voltage and extrapolate remaining battery power
 * Params : None
 * Return : None
*/
void *battery(void *arg)
{
	int32_t volt = -1;
	int32_t adc;
	int32_t retval;
	int32_t i;
	printf("Battery thread ID : %i\n", syscall(__NR_gettid));
	pthread_cond_t *end = arg;

	if(!ad_board_setup())
	{
		printf("init failed\n");
		pthread_exit((void *) retval);
	}
	while(volt == -1 || check_battery(volt) )
	{
		adc = 0;
		for (i = 0;i< NB_ITERATION; i++)
		{
			while((ADS1256_Scan() == 0));
			adc += (int32_t) ADS1256_GetAdc(CH_NUM);
			
			sleep(ITERATION_TIME);
		}
		adc = (int32_t) adc/NB_ITERATION;
		volt = (adc * 100) / 167;
		if(volt < 0)
			volt = -volt;
		sleep(SLEEP_TIME);
	}
	
	pthread_cond_broadcast(end);
	set_next_startup(100); //@TODO : 100 for testing
	bcm2835_spi_end();
	retval = 1;
	pthread_exit((void *)retval);
}

/*
 * check_battery : Read and update config to decide whether to shutdown or not
 * Param : The measured voltage
 * Return : 0 if system should be shutdown, 1 otherwise
 */
static uint8_t check_battery(int32_t volt)
{
	static double value[4] = {0,0,0,0};

	static time_t start_time = NULL;
	if(!start_time)
	{
		printf("start time init\n");
		time(&start_time);
	}
	
	//Initialize value[] with the variables from the configuration file
	if (value[MAX_VOLT] == 0)
	{
		char *str[4] = {"MAX_VOLT", "TRESHOLD", "LAST_DISCHARGE","ACQ_TIME"};
		get_cfg(value, str,  4);
		printf("MAX_VOLT : %f, TRESHOLD : %f%\n", value[0], value[1]*100);
		set_acq_time(volt, value);
	}
	//If voltage is less than fixed threshold, shutdown the raspberry pi
	if (volt/1000000 < value[MAX_VOLT]*value[TRESHOLD])
	{
		char *str[1] = {"ACQ_TIME"};
		value[ACQ_TIME] = difftime(time(NULL), start_time); 
		set_cfg(str, &value[ACQ_TIME], 1);
		return 0;
	}
	// Update configuration MAX_VALUE if actual voltage is >	
	else if (volt/1000000 > value[MAX_VOLT]) 
	{
		value[MAX_VOLT] = (double) volt/1000000;
		char *str[1] = {"MAX_VOLT"};
		set_cfg(str, &value[MAX_VOLT], 1); 
	}

	//@TODO : save config

	// If Acq_time is over, shutdown the raspberry pi
	if(difftime(time(NULL), start_time) > value[ACQ_TIME]*0.97) //097?
		return 0;

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
			       value[TRESHOLD]*value[MAX_VOLT];
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
