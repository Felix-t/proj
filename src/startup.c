/*
 * startup.c:
 *	Program launched on startup 
 *	 - Check remaining battery power
 *	 - Schedule next startup and shutdown 
 *	 - Start acquisitions subroutines
 *  @TODO : Logging system, see syslog
 */


#include "headers.h"

#include "ADS1256.h"
#include "cfg.h"
#include "util.h"
#include "accelerometer.h"
#include "battery.h"
#include "Main_Acquisition_Opsens_WLX2.h"

static uint8_t start_AD_acq(pthread_t *battery_thread);
static uint8_t start_WLX2_acq(pthread_t *WLX2_thread);
static uint8_t start_Accelerometer_acq(pthread_t *accelerometer_thread);


// @TODO : ...
void *placeholder(void* arg)
{}

/* Function start_AD_acq :
 * Create the thread used for power management
 * Params : The pid of the thread
 * Return : Success/failure code
*/
static uint8_t start_AD_acq(pthread_t *battery_thread)
{

	pthread_create(battery_thread, NULL, battery, NULL);
	
//	free(status);
	return 1;
}

/* Function start_Accelerometer_acq :
 * Create the thread used for accelerometer and gyrometer acquisition
 * Params : Pid of the thread, condition signaling the end of the thread
 * Return : Success/failure code
*/
static uint8_t start_Accelerometer_acq(pthread_t *accelerometer_thread)
{

	pthread_create(accelerometer_thread, NULL, acq_GYR_ACC, NULL);
	
//	free(status);
	return 1;
}



/* Function start_WLX2_acq :
 * Create the thread used for optic fiber acquisition with the WLX2 module
 * Params : Pid of the thread, condition signal to end of the thread
 * Return : Success/failure code
*/
static uint8_t start_WLX2_acq(pthread_t *WLX2_thread)
{
	//@TODO : start dhcp server with correct ip address

	pthread_create(WLX2_thread, NULL, acq_WLX2, NULL);
	
//	free(status);
	return 1;
}

/* Function : pour tester le multithreading
 * Params :
 * Return :
*/
static void *test()
{

	printf("Thread test created id : %i\n", syscall(__NR_gettid));
	uint8_t i;
	//for(i=0;i<10;i++)
	//{
		//sleep(1);
		//printf("tour n%i", i);
	//}
	sleep(5);
	end_program = 1;
	pthread_exit((void *) 1);
}


uint8_t end_program = 0;
int  main()
{
	int32_t status;
	uint8_t i = 0;
	pthread_t *threads = (pthread_t *) malloc((ACC_GYR + WLX2 + 1)*sizeof(pthread_t));
 
	printf("Main thread ID : %i\n", syscall(__NR_gettid));

	if(!bcm2835_init())
		return 0;

	//Start battery management
	if(!start_AD_acq(&threads[i++]))
	//if(pthread_create(&threads[i++], NULL, test, NULL))
	{
		printf("AD thread creation failed\n");
		return 0;
	}
	sleep(1);
	if(WLX2 && !start_WLX2_acq(&threads[i++]))
	{
		printf("WLX2 thread creation failed\n");
		return 0;
	}
	if(ACC_GYR && !start_Accelerometer_acq(&threads[i++]))
	{
		printf("LSM9D0 thread creation failed\n");
		return 0;
	}	
	for(i=0;i< ACC_GYR + WLX2 + 1;i++)
	{
		sleep(5);
		pthread_join(threads[i], NULL);
		end_program = 1;
	}

	free(threads);
	printf("Acquisition ended with code %d\n", status);	
	bcm2835_close();

    return 0;
} 


