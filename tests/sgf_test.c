#include "headers.h"
#include "sigfox.h"

_Atomic uint8_t end_program = 0;
int main()
{
	// Test setup
	uint8_t i = 0, j = 0;
	uint8_t nb_threads = SGF_ENABLE + LSM9DS0_ENABLE + WLX2_ENABLE +1;
	_Atomic uint8_t alive[nb_threads];
	pthread_t thread;
	pthread_create(&thread, NULL, sigfox, (void*) alive);
	for(i=0;i<nb_threads;i++)
	{
		alive[i] = 1;
	}
	//End of test setup 

	//Start of test
	struct sgf_data tmp_struct = 
	{
		.min = 12.0,
		.max = 256.14,
		.mean = 54.6,
		.std_dev = 27.456,
		.id = WLX2
	};

	sleep(1);
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_t send_data[10];
	pthread_create(&send_data[0], &attr, send_sigfox, (void*) &tmp_struct);

	sleep(5);

	tmp_struct.id = AD_CONVERTER;
	tmp_struct.min = 24.0;
	tmp_struct.max = 876.94;
	tmp_struct.mean = 270.45;
	tmp_struct.std_dev = 0.2543;
	tmp_struct.time = time(NULL);

	pthread_create(&send_data[1], &attr, send_sigfox, (void*) &tmp_struct);
	sleep(2);
	end_program = 1;
	pthread_join(send_data[0], NULL);
	pthread_join(send_data[1], NULL);
	pthread_join(thread, NULL);
	pthread_mutex_destroy(&sgf_msg.mutex);

	pthread_attr_destroy(&attr);

	//Verify correcteness of results
	uint8_t result, ref_result;
	FILE *fp, *fp_ref;
	fp_ref = fopen("tests/ref_sgf", "r");
	if((fp = fopen("logs/sigfox", "r")) == NULL)
	{
		printf("Echec test SGF : can't open logs/sigfox\n");
		return 0;
	}

	for(j=0;j<4;j++)
	{

		for(i=0;i<12;i++)
		{
			fscanf(fp, "%hhx", &result);
			fscanf(fp_ref, "%hhx", &ref_result);
			if(i > 3 && result != ref_result)
			{
				printf("Fail test SGF : unexpected values");
				return 0;
			}
		}
	}
	printf("Success test SGF");

	return 1;
}

