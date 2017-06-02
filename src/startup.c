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
#include "gps.h"

static uint8_t start_AD_acq(pthread_t *thread_id);
static uint8_t start_WLX2_acq(pthread_t *thread_id);
static uint8_t start_Accelerometer_acq(pthread_t *thread_id);
static uint8_t start_Sigfox(pthread_t *thread_id);
static uint8_t start_GPS_acq(pthread_t *thread_id);

_Atomic uint8_t end_program = 0;
_Atomic uint8_t *alive;

/* Function start_AD_acq :
 * Creates the thread used for power management
 * Params : The pid of the thread
 * Return : Success/failure code
 */
static uint8_t start_AD_acq(pthread_t *thread_id)
{
	if(!pthread_create(thread_id, NULL, battery, (void*) 0))	
		return 1;

	printf("Analog to Digital thread creation failed\n");
	end_program = 1;
	return 0;
}


/* Function : Creates the thread used for GPS acquisition
 * Params : Pid of the thread
 * Return : Success/failure bool
 */
static uint8_t start_GPS_acq(pthread_t *thread_id)
{
	if(!pthread_create(thread_id, NULL, gps, (void*) 0))
		return 1;

	printf("GPS thread creation failed\n");
	return 0;
}


/* Function : Creates the thread used for sigfox management
 * Params : Pid of the thread
 * Return : Success/failure bool
 */
static uint8_t start_Sigfox(pthread_t *thread_id)
{
	if(!pthread_create(thread_id, NULL, sigfox, (void*) 0))
		return 1;

	printf("Sigfox thread creation failed\n");
	return 0;
}


/* Function : Creates the thread used for accelerometer and gyrometer acquisition
 * Params : Pid of the thread
 * Return : Success/failure bool
 */
static uint8_t start_Accelerometer_acq(pthread_t *thread_id)
{
	// i2c should have been set up by witty pi (outside this program),
	// in case it didn't :
	if(!bcm2835_i2c_begin())
	{
		printf("Error initaliazing i2c, accelerometer thread failed\n");
		return 0;		
	}

	if(!pthread_create(thread_id, NULL, acq_GYR_ACC, (void*) 0))
		return 1;
	printf("Accelerometer thread creation failed\n");
	return 0;
}


/* Function : Creates the thread used for optic fiber acquisition
 * 	with the WLX2 module
 * Params : Pid of the thread
 * Return : Success/failure bool
 */
static uint8_t start_WLX2_acq(pthread_t *thread_id)
{
	//@TODO : Lancer WLX2 via une activation d'un port gpio
	//@TODO : start dhcp server with correct ip address
	if(!start_dhcp_server())
	{
		printf("Dhcp server creation failed\n");
		return 0;
	}

	if(!pthread_create(thread_id, NULL, acq_WLX2, (void*) 0))
		return 1;

	printf("WLX2 thread creation failed\n");
	// @TODO : traitements erreurs	
	return 0;
}


/* Function : pour tester le multithreading, remplace battery management
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

	if(alive[SGF])
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
	sleep(3);

	printf("Main thread ID : %li\n", syscall(__NR_gettid));

	uint8_t i = 0, j = 0;
	uint8_t nb_threads = SGF_ENABLE + LSM9DS0_ENABLE + WLX2_ENABLE + 
		GPS_ENABLE + 1;

	if(!bcm2835_init())
	{
		printf("Failed initialising bcm2835 library\n");
		return 0;
	}

	pthread_t *threads = malloc(nb_threads*sizeof(pthread_t));

	alive = malloc(sizeof(uint8_t) * 14);
	memset(alive, 0, 14);

	pthread_mutex_init(&sgf_msg.mutex, NULL);

	// Start battery management
	//  --> Real battery management
	//i += start_AD_acq(&threads[i]);

	// --> Fake battery management for debugging

	if(pthread_create(&threads[i++], NULL, test, NULL)) // To debug
	{
		printf("AD thread creation failed\n");
		end_program = 1;
	}

	sleep(10); // Wait for first battery check

	if(end_program)
	{
		printf("Not enough battery to start program\n");
		pthread_join(threads[--i], NULL);
	}
	else
	{
		if(SGF_ENABLE)
		{
			i += start_Sigfox(&threads[i]);
			// Wait for sigfox to init
			while(alive[SGF] != 1 && j < DOWNLINK_TIMEOUT*2)
			{		sleep(1);
				j++;
				if(alive[SGF] == 2)
				{
					printf("Sigfox failed\n");
					break;
				}
			}
		}

		if(LSM9DS0_ENABLE)
			i += start_Accelerometer_acq(&threads[i]);

		if(WLX2_ENABLE)
			i += start_WLX2_acq(&threads[i]);

		if(GPS_ENABLE)
			i += start_GPS_acq(&threads[i]);

		while(!end_program)
			sleep(5);
	}
	while(i != 0)
	{
		pthread_join(threads[--i], NULL);
	}

	pthread_mutex_destroy(&sgf_msg.mutex);
	free(threads);
	free(alive);
/*
	move_logs();
       	archive_data();

	if(SHUTDOWN)
		program_shutdown(3);
*/
	bcm2835_close();
	return 0;
} 


