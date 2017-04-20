#include <math.h>
#include "headers.h"
#include "accelerometer.h"

_Atomic uint8_t end_program = 0;
int main()
{
	//Setup tests
	int i;
	double sum_acc = 0, sum_gyr = 0;
	float *read_data[3];

	read_data[0] = malloc(sizeof(float)*3*3);
	read_data[1] = read_data[0] + sizeof(float)*3;	
	read_data[2] = read_data[1] + sizeof(float)*3;	

	bcm2835_init();
	bcm2835_i2c_begin();


	//Test setup of the card and single measurement
	//    ------------
	if(!setup_all())
	{
		printf("Fail test acc : setup failed\n");
		return 0;
	}
	sleep(1);
	if (!read_all(read_data))
	{
		printf("Fail test acc : read failed\n");
		return 0;
	}

	for(i=0;i<3;i++)
	{
		sum_acc += pow(read_data[0][i], 2);
		sum_gyr += read_data[1][i];
	}

	if(sqrt(sum_acc)/1000 < 0.9 || sqrt(sum_acc)/1000 > 1.1)
	{
		printf("Fail test acc : =! 1 while sensor is still\n");
		return 0;
	}
	if(sum_gyr/3000 < -0.5 || sum_gyr/3000 > 0.5)
	{
		printf("Fail test acc : gyr failed\n");
		return 0;
	}

	printf("Test acc single measurement success\n");


	//Test acquisition and print : nm mode test
	//    -----------
	
	float val_ref[4];
	pthread_t thread_1;
	uint8_t alive = 0;
	pthread_mutex_init(&sgf_msg.mutex, NULL);
	pthread_create(&thread_1, NULL, acq_GYR_ACC, (void *) &alive);
	sleep(1);
	if (alive != 1)
		printf("Fail test acc : didn't set up alive pointer");

	sleep(30);
	if(sgf_msg.write_allowed != 0 || sgf_msg.id != LSM9DS0)
	{
		printf("Fail test acc : didn't set up msg_sgf properly");
		return 0;
	}
	sgf_msg.write_allowed = 1;
	val_ref[0] = sgf_msg.min;
	val_ref[1] = sgf_msg.max;
	val_ref[2] = sgf_msg.mean;
	val_ref[3] = sgf_msg.std_dev;
	for (i = 0; i< 4; i++)
	printf("%f\n", val_ref[i]);
	sleep(30);
	if(sgf_msg.write_allowed != 0 || sgf_msg.id != LSM9DS0)
	{
		printf("Fail test acc : didn't set up msg_sgf properly");
		return 0;
	}

	//@TODO : test print
	end_program = 1;
	pthread_join(thread_1, NULL);
	pthread_mutex_destroy(&sgf_msg.mutex);
	printf("b\n");;

	//Test change scale
	//    -----------
/*
	
	float **val[100];
	val[0] = malloc(sizeof(float *)*100*2);
	val[0][0] = malloc(sizeof(float)*3*2*100);
	for(i=0;i<100;i++)
	{
		val[i] = val[0] + i*2*sizeof(float *);
		val[i][0] = val[0][0] + 3*2*i*sizeof(float); 
		val[i][1] = val[0][0] + 3+3*2*i*sizeof(float); 
	}

	for(i=0;i<50;i++)
	{

		for(i=0;i<100;i++)
		{
			read_all(val[i]);
			stat(val[i], 0);
			
		}

	}

*/
	printf("End test\n");
	free(read_data[0]);
	bcm2835_i2c_end();
	bcm2835_close();
	return 1;
}
