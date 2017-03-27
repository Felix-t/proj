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

static uint8_t start_AD_acq(pthread_t *battery_thread, _Atomic uint8_t *alive);
static uint8_t start_WLX2_acq(pthread_t *WLX2_thread, _Atomic uint8_t *alive);
static uint8_t start_Accelerometer_acq(pthread_t *accelerometer_thread, _Atomic uint8_t *alive);
static uint8_t start_sigfox(pthread_t *sigfox_thread, _Atomic uint8_t alive);

_Atomic uint8_t end_program = 0;

// @TODO : placeholder for file sigfox.c etc
void *sigfox(void* arg)
{
	_Atomic uint8_t *alive = arg;
	printf("Thread sigfox created, id : %i\n",
			syscall(__NR_gettid));
	FILE *fp;
	if(!(fp = fopen("logs/sigfox", "w+")))
	{
		printf("Can't open file sigfox");
		pthread_exit((void *) 0);
	}
	while(!end_program)
	{
	fprintf(fp, "thread AD : %u\nthread lsmd9ds0 : %uthread sigfox : %u",
			alive[0], alive[1], alive[2]);
	fflush(fp); // To fprint immediately
	sleep(5);
	}
	fclose(fp);
}

/* Function start_AD_acq :
 * Create the thread used for power management
 * Params : The pid of the thread
 * Return : Success/failure code
*/
static uint8_t start_AD_acq(pthread_t *battery_thread, _Atomic uint8_t *alive)
{
	printf("b");

	pthread_create(battery_thread, NULL, battery, (void*)alive);
	
//	free(status);
	return 1;
}

static uint8_t start_Sigfox(pthread_t *sigfox_thread, _Atomic uint8_t *alive)
{

	pthread_create(sigfox_thread, NULL, sigfox, (void*)alive);
	
//	free(status);
	return 1;
}



/* Function start_Accelerometer_acq :
 * Create the thread used for accelerometer and gyrometer acquisition
 * Params : Pid of the thread, condition signaling the end of the thread
 * Return : Success/failure code
*/
static uint8_t start_Accelerometer_acq(pthread_t *accelerometer_thread, _Atomic uint8_t *alive)
{

	pthread_create(accelerometer_thread, NULL, acq_GYR_ACC, (void*)alive);
	
//	free(status);
	return 1;
}



/* Function start_WLX2_acq :
 * Create the thread used for optic fiber acquisition with the WLX2 module
 * Params : Pid of the thread, condition signal to end of the thread
 * Return : Success/failure code
*/
static uint8_t start_WLX2_acq(pthread_t *WLX2_thread, _Atomic uint8_t *alive)
{
	//@TODO : start dhcp server with correct ip address

	pthread_create(WLX2_thread, NULL, acq_WLX2, (void*)alive);
	
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


int  main()
{
	int32_t status;
	uint8_t i = 0;
	uint8_t nb_threads = SGF + ACC_GYR + WLX2 +1;
	pthread_t *threads = malloc(nb_threads*sizeof(pthread_t));
 	_Atomic uint8_t alive[nb_threads];

	printf("Main thread ID : %i\n", syscall(__NR_gettid));

	if(!bcm2835_init())
		return 0;

	//Start battery management
	if(!start_AD_acq(&threads[i], &alive[i++]))
	//if(pthread_create(&threads[i++], NULL, test, NULL))
	{
		printf("AD thread creation failed\n");
		return 0;
	}
	sleep(1);
	if(WLX2 && !start_WLX2_acq(&threads[i], &alive[i++]))
	{
		printf("WLX2 thread creation failed\n");
		return 0;
	}
	if(ACC_GYR && !start_Accelerometer_acq(&threads[i++], &alive[i++]))
	{
		printf("LSM9D0 thread creation failed\n");
		return 0;
	}	
	if(SGF && !start_Sigfox(&threads[i], alive))
	{
		printf("Sigfox thread creation failed\n");
		return 0;
	}	
	for(i = 0; i < nb_threads; i++)
	{
		sleep(5);
		pthread_join(threads[i], NULL);
	}

	free(threads);
	printf("Acquisition ended with code %d\n", status);	
	bcm2835_close();

    return 0;
} 
