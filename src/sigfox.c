#include "headers.h"
#include "sigfox.h"

static void build_message(uint8_t msg[12], uint32_t t, float val1, float val2, 
		_Atomic uint8_t *alive, identity id, uint8_t msg_type);
static time_t get_ref_time();

void * send_sigfox(void *args)
{
	printf("Thread send data created, id : %li\n",
			syscall(__NR_gettid));
	uint8_t write = 1;
	time_t t = time(NULL);
	struct sgf_data *tmp_struct = args;
	while(write != 0)
	{
		pthread_mutex_lock(&sgf_msg.mutex);
		if(sgf_msg.write_allowed == 1)
		{
			sgf_msg.time = t;
			sgf_msg.id = tmp_struct->id;
			sgf_msg.min = tmp_struct->min;
			sgf_msg.max = tmp_struct->max;
			sgf_msg.mean = tmp_struct->mean;
			sgf_msg.std_dev = tmp_struct->std_dev;
			sgf_msg.write_allowed = 0;
			write = 0;
		}
		pthread_mutex_unlock(&sgf_msg.mutex);
		sleep(2);
	}
	pthread_exit((void *) 1);
}


static time_t get_ref_time()
{
	time_t t = time(NULL);
	struct tm *tmp = localtime(&t);
	// @TODO : start de la journée ? quelle valeur de réf:
	tmp->tm_sec = 0;
	tmp->tm_min = 0;
	tmp->tm_hour = 0;
	return mktime(tmp);
}


void *sigfox(void* args)
{
	printf("Thread sigfox created, id : %li\n",
			syscall(__NR_gettid));

	int i, pos_in = 0, pos_out = 0;
	uint32_t t;
	uint8_t *messages[MAX_NB_MSG];

	time_t ref_time = get_ref_time();

	_Atomic uint8_t *alive = args;

	if((messages[0] = malloc(MAX_NB_MSG*SIZE_SIGFOX_MSG)) == NULL)
	{
		printf("malloc failed\n");
		return 0;
	}
	for(i = 0; i < MAX_NB_MSG; i++)
		messages[i] = messages[0] + 12*i;

	FILE *fp;
	if(!(fp = fopen("logs/sigfox", "w+")))
	{
		printf("Can't open file sigfox");
		pthread_exit((void *) 0);
	}

	pthread_mutex_init(&sgf_msg.mutex, NULL);
	sgf_msg.write_allowed = 1;

	while(!end_program)
	{
		for(i = 0; i < 12; i++)
			messages[pos_in][i] = 0;

		pthread_mutex_lock(&sgf_msg.mutex);
		//if acq thread has finished writing :
		if(sgf_msg.write_allowed == 0)
		{
			if(difftime(sgf_msg.time, ref_time) > 3600*24)
				ref_time += 3600*24;
			t = (uint32_t) difftime(sgf_msg.time, ref_time) << 12;

			//Build message min/max
			build_message(messages[pos_in], t, sgf_msg.min, 
					sgf_msg.max, alive, sgf_msg.id, 0);
			
			//Build message mean/standard dev
			pos_in++;
			build_message(messages[pos_in], t, sgf_msg.mean, 
					sgf_msg.std_dev, alive, sgf_msg.id, 1);
			pos_in++;
			sgf_msg.write_allowed = 1;
		}
		pthread_mutex_unlock(&sgf_msg.mutex);

		//@TODO : remplacer par envoi sigfox
		if(pos_out < pos_in)
		{
			for (i = 0; i < 12; i++)
				fprintf(fp, "%hhx ", messages[pos_out][i]);
			pos_out++;
			fprintf(fp, "\n");
		}
		sleep(2);
	}
	fclose(fp);
	free(messages[0]);
	pthread_exit((void *) 1);
}


static void build_message(uint8_t msg[12], uint32_t t, float val1, float val2, 
		_Atomic uint8_t *alive, identity id, uint8_t msg_type)
{
	uint8_t tmp, i;
	// Construct the 12 bytes message sent with sigfox
	memcpy(&msg[0], &t, 4);  //TIME on 3 bytes
	//@TODO : a cause de little endian :
	
	for(i=0;i<2;i++)
	{
		tmp = msg[i];
		msg[i] = msg[3-i];
		msg[3-i] = tmp;
	}
	

	//alive, id, type of msg :
	/*
	msg[3] |= (alive[0] << 6);
	if(LSM9DS0_ENABLE)
		msg[3] |= (alive[1] << 5);
	if(WLX2_ENABLE)
		msg[3]|= (alive[2] << 4);
	*/
	msg[3] |= (id << 1);
	msg[3] |= msg_type;

	memcpy(&msg[4], &val1, 4);
	memcpy(&msg[8], &val2, 4);
	printf("val1 : %f, val2 : %f\n", val1, val2);
}
