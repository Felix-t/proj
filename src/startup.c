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
#include "sigfox.h"
#include <math.h>

static uint8_t start_AD_acq(pthread_t *battery_thread, _Atomic uint8_t *alive);
static uint8_t start_WLX2_acq(pthread_t *WLX2_thread, _Atomic uint8_t *alive);
static uint8_t start_Accelerometer_acq(pthread_t *accelerometer_thread, _Atomic uint8_t *alive);
static uint8_t start_Sigfox(pthread_t *sigfox_thread, _Atomic uint8_t *alive);

_Atomic uint8_t end_program = 0;
_Atomic uint8_t *alive;

/* Function start_AD_acq :
 * Create the thread used for power management
 * Params : The pid of the thread
 * Return : Success/failure code
 */
static uint8_t start_AD_acq(pthread_t *battery_thread, _Atomic uint8_t *alive)
{
	if(pthread_create(battery_thread, NULL, battery, (void*) 0))	
		return 0;
	return 1;

}


/*
static uint8_t start_Sigfox(pthread_t *sigfox_thread, _Atomic uint8_t *alive)
{
	if(pthread_create(sigfox_thread, NULL, sigfox, (void*)alive))
		return 0;

	return 1;
}
*/


/* Function start_Accelerometer_acq :
 * Create the thread used for accelerometer and gyrometer acquisition
 * Params : Pid of the thread, condition signaling the end of the thread
 * Return : Success/failure code
 */
/*
static uint8_t start_Accelerometer_acq(pthread_t *accelerometer_thread, _Atomic uint8_t *alive)
{
	// i2c should have been set up by witty pi (outside this program),
	// in case it didn't :
	if(!bcm2835_i2c_begin())
		return 0;

	if(pthread_create(accelerometer_thread, NULL, acq_GYR_ACC, (void*)alive))
		return 0;
	return 1;
}
*/


/* Function : start_WLX2_acq
 * Create the thread used for optic fiber acquisition with the WLX2 module
 * Params : Pid of the thread, condition signal to end of the thread
 * Return : Success/failure code
 */

/*
static uint8_t start_WLX2_acq(pthread_t *WLX2_thread, _Atomic uint8_t *alive)
{
	//@TODO : Lancer WLX2 via une activation d'un port gpio
	//@TODO : start dhcp server with correct ip address
	if(!start_dhcp_server())
	{
		printf("dhcp server creation failed\n");
		return 0;
	}

	if(pthread_create(WLX2_thread, NULL, acq_WLX2, (void*)alive))
		return 0;
	// @TODO : traitements erreurs	
	return 1;
}
*/
/* Function : pour tester le multithreading, remplace battery management
 * Params :
 * Return :
 */
static void *test()
{
	printf("Thread test created id : %li\n", syscall(__NR_gettid));

	pthread_t thread;
	pthread_attr_t attr;

	unsigned int i;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	struct sgf_data data_to_send = {
		.id = AD_CONVERTER,
		.min = 1,
		.max = 2,
		.std_dev = 5,
		.mean = 5};

	if(SGF_ENABLE)
		pthread_create(&thread, &attr, send_sigfox, (void *) &data_to_send);
	for(i = 0; i < 2;i++)
	{	
		get_cpu_usage();
		get_temp();
		sleep(60);
	}

	end_program = 1;
	pthread_attr_destroy(&attr);
	pthread_exit((void *) 1);
}

int  main()
{	
	sleep(20);
	uint8_t i = 0, j = 0;
	uint8_t nb_threads = SGF_ENABLE + LSM9DS0_ENABLE + WLX2_ENABLE +1;
	pthread_t *threads = malloc(nb_threads*sizeof(pthread_t));
	alive = malloc(sizeof(uint8_t) * 13);
	memset(alive, 0, 13);
	i = 0;

	pthread_mutex_init(&sgf_msg.mutex, NULL);

	printf("Main thread ID : %li\n", syscall(__NR_gettid));

	if(!bcm2835_init())
		return 0;

	//Start battery management
	if(!start_AD_acq(&threads[i], &alive[AD_CONVERTER]) || !++i)
		//if(pthread_create(&threads[i++], NULL, test, NULL))
	{
		printf("AD thread creation failed\n");
		end_program = 1;
	}
	sleep(10); // Wait for first battery check
	if(end_program)
	{
		printf("Not enough battery to start program\n");
		pthread_join(threads[0], NULL);
	}
	else
	{
		if(SGF_ENABLE && pthread_create(&threads[i++], NULL, sigfox, 
					(void*) 0))
			printf("Sigfox thread creation failed\n");
		else if(SGF_ENABLE)
		{
			while(alive[SGF] != 1 && j < DOWNLINK_TIMEOUT)
			{		sleep(1);
				j++;
				if(alive[SGF] == 2)
				{
					printf("Sigfox failed\n");
					break;
				}
			}
		}
		//Continue even if sigfox failed : non critical
		if(!bcm2835_i2c_begin())
			printf("i2c initialization failed\n");
		else if(pthread_create(&threads[i], NULL, acq_GYR_ACC,
					(void*) 0)
				|| !++i)
			printf("LSM9D0 thread creation failed\n");
		else if(!start_dhcp_server())
			printf("dhcp server creation failed\n");
		else if(pthread_create(&threads[i], NULL, acq_WLX2,
					(void*) 0)
				|| !++i)
			printf("WLX2 thread creation failed\n");


		for(i = 0; i < nb_threads; i++)
		{
			sleep(5);
			pthread_join(threads[i], NULL);
		}
	}
	pthread_mutex_destroy(&sgf_msg.mutex);
	free(threads);
	if(!move_logs() || !archive_data())
		return 0;	
	if(SHUTDOWN && !program_shutdown(3))
		return 0;
	bcm2835_close();
	return 0;
} 


