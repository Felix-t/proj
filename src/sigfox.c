#include "headers.h"
#include "sigfox.h"
#include "cfg.h"
#include "serial.h"
#include <termios.h>
#include <errno.h>
#include <fcntl.h>

static uint8_t build_message(uint8_t msg[12], uint32_t t, float val1, float val2, 
		identity id, uint8_t msg_type);
static time_t get_ref_time();
static void parse_downlink(uint8_t *msg_received);
static void receive_downlink(int fd, uint8_t *msg_received);


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
	printf("Thread send data exited\n");
	pthread_exit(0);
}


static time_t get_ref_time(time_t t)
{
	struct tm *tmp = localtime(&t);
	// @TODO : start de la journée ? quelle valeur de réf:
	tmp->tm_sec = 0;
	tmp->tm_min = 0;
	tmp->tm_hour = tmp->tm_hour >= 12 ? 12 : 0; 
	return mktime(tmp);
}

static void receive_downlink(int fd, uint8_t *msg_received)
{
	uint8_t i,bytes_rcv,  save = 0, bytes_ack = 0;
	uint8_t rcv_msg[100] = {0};
	write(fd, "+++", 3);
	sleep(2);
	write(fd, "ATS000=d0\n", 10);
	sleep(2);
	write(fd, "ATQ\n", 4);
	sleep(2);
	write(fd, "ACK", 3);
	sleep(DOWNLINK_TIMEOUT);
	printf("Checking for Aknowledgment... \n");
	do
	{
		bytes_rcv = read(fd, rcv_msg, 100);

		for(i=0 ; i<bytes_rcv ; i++)
		{
			if(save && bytes_ack < 8)
				msg_received[bytes_ack++] = rcv_msg[i];

			else if(i > 7 
					&& rcv_msg[i-8] == 'T' 
					&& rcv_msg[i-7] == ' ' 
					&& rcv_msg[i-6] == 'S' 
					&& rcv_msg[i-5] == 'E' 
					&& rcv_msg[i-4] == 'T' 
					&& rcv_msg[i-3] == 'U' 
					&& rcv_msg[i-2] == 'P' )
					//&& rcv_msg[i-1] == '\r' 
					//&& rcv_msg[i] == '\n')
				save = 1;
		}
	}while(bytes_rcv != 0);
	if(!save)
		memset(msg_received, 0, 8);
	sleep(2);
	write(fd, "+++", 3);
	sleep(2);
	write(fd, "ATS000=50\n", 10);
	sleep(2);
	write(fd, "ATQ\n", 4);
	sleep(2);
	char tmp[12];
	sprintf(tmp, "recu:%hhxZ", msg_received[0]);
	write(fd, tmp, 7);
}

static void parse_downlink(uint8_t *msg_received)
{
	char *cfg_str[1];
	cfg_str[0] = malloc(100);
	uint32_t tmp;
	float tmp_float;
	double cfg_val;
	switch(msg_received[0]){
	case 1:
		printf("Case 1\n");
		strcpy(cfg_str[0], "MIN_VOLT");
		break;
	case 2:
		printf("Case 2\n");
		strcpy(cfg_str[0], "MAX_VOLT");
		break;
	case 3:
		printf("Case 3\n");
		strcpy(cfg_str[0], "THRESHOLD");
		break;
	case 4:
		printf("Case 4\n");
		strcpy(cfg_str[0], "freq_echantillonnage");
		break;
	case 5:
		printf("Case 5\n");
		strcpy(cfg_str[0], "zeros");
		break;
	case 6:
		printf("Case 6\n");
		strcpy(cfg_str[0], "ACQ_TIME");
		break;
	default:
		strcpy(cfg_str[0], "");
		return;
		break;
	}
	tmp = (msg_received[1] << 24)
		| (msg_received[2] << 16)
		| (msg_received[3] << 8)
		| msg_received[4];
	memcpy(&tmp_float, &tmp, 4);
	cfg_val = tmp_float;
	printf("Changing config %s to %f\n", cfg_str[0], cfg_val);
	set_cfg(cfg_str, &cfg_val, 1);
	free(cfg_str[0]);

}

void *sigfox(void* args)
{
	printf("Thread sigfox created, id : %li\n",
			syscall(__NR_gettid));

	time_t last_msg = time(NULL);
	int i, pos_in = 0, pos_out = 0;
	uint8_t local_alive[NB_IDENTITY] = {0};
	uint32_t t;
	uint8_t *messages[MAX_NB_MSG + 1] = {NULL};

	time_t ref_time  = 0;

	int fd = open (SGF_PORT, O_RDWR | O_NOCTTY | O_SYNC);
	
	if (fd < 0)
	{
		printf("error %d opening %s: %s", errno, SGF_PORT, strerror (errno));
		alive[SGF] = 2;
		pthread_exit((void *) 0);
	}
	set_interface_attribs (fd, B19200, 0);  // set speed to 19,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking


	if((messages[0] = malloc(MAX_NB_MSG*SIZE_SIGFOX_MSG)) == NULL)
	{
		printf("malloc failed\n");
		alive[SGF] = 2;
		pthread_exit((void *) 0);
	}
	for(i = 0; i < MAX_NB_MSG; i++)
		messages[i] = messages[0] + 12*i;

	FILE *fp;
	if(!(fp = fopen("logs/sigfox", "w+")))
	{
		printf("Can't open file sigfox");
		alive[SGF] = 2;
		pthread_exit((void *) 0);
	}
	
	// @TODO : receive downlink message
	uint8_t msg_downlink[8] = {0};
	receive_downlink(fd, msg_downlink);
	// @TODO : remove, only for testing
	parse_downlink(msg_downlink);

	alive[SGF] = 1;

	sgf_msg.write_allowed = 1;

	sleep(60);

	//Stop the loop if end_program is 1, or if too many messages have been
	// sent through sigfox
	while(!end_program && pos_in < MAX_NB_MSG)
	{
		for(i = 0; i < 12; i++)
			messages[pos_in][i] = 0;

		pthread_mutex_lock(&sgf_msg.mutex);
		//if acq thread has finished writing :
		if(sgf_msg.write_allowed == 0)
		{
			ref_time = get_ref_time(sgf_msg.time);
			t = (uint32_t) difftime(sgf_msg.time, ref_time);

			//Build message min/max
			pos_in += build_message(messages[pos_in], t, 
					sgf_msg.min, sgf_msg.max, 
					sgf_msg.id, 0);
			
			//Build message mean/standard dev
			pos_in += build_message(messages[pos_in], t,
					sgf_msg.mean, sgf_msg.std_dev,
					sgf_msg.id, 1);
			sgf_msg.write_allowed = 1;
		}
		// Check if any thread stopped working. local_alive is set to 1
		// if the failure of a thread has already been reported
		else if(local_alive[WLX2_CH1] == 0 && alive[WLX2_CH1] == 0)
		{
			pos_in += build_message(messages[pos_in], t,
					0.0, 0.0, WLX2_CH1, 3);
			local_alive[WLX2_CH1] = 1;
		}
		else if(local_alive[LSM9DS0_ACC_X] == 0 && alive[LSM9DS0_ACC_X] == 0)
		{
			pos_in += build_message(messages[pos_in], t,
					0.0, 0.0, LSM9DS0_ACC_X, 3);
			local_alive[LSM9DS0_ACC_X] = 1;
		}
		else if(local_alive[GPS] == 0 && alive[GPS] == 0)
		{
			pos_in += build_message(messages[pos_in], t,
					0.0, 0.0, GPS, 3);
			local_alive[GPS] = 1;
		}
		pthread_mutex_unlock(&sgf_msg.mutex);

		//@TODO : remplacer par envoi sigfox
		if(pos_out < pos_in && 
				difftime(time(NULL), last_msg) > SGF_INTERVAL)
		{
			for (i = 0; i < 12; i++)
				fprintf(fp, "%hhx ", messages[pos_out][i]);
			fprintf(fp, "\n");
			write(fd, messages[pos_out], 12);
			pos_out++;
			fflush(fp);
			last_msg = time(NULL);
			printf("--> Message sigfox sent\n");
		}
		sleep(1);
	}
	alive[SGF] = 0;
	//If messages > 140, allow the send_sigfox thread to exit by setting 
	// the shared variable to 1 continuously
	while(!end_program)
	{
		sgf_msg.write_allowed = 1;
		sleep(1);
	}

	fclose(fp);
	free(messages[0]);
	pthread_exit((void *) 1);
}


static uint8_t build_message(uint8_t msg[12], uint32_t t, float val1, float val2, 
		identity id, uint8_t msg_type)
{
	printf("Build message ; ID = %i\n", id);
	static uint8_t power = 0;

	// We need to save the battery info to embed it into the other messages
	// This information is in val1 when id is AD_CONVERTER and type is 1
	// Battery information (min max std_dev) is not send through sigfox
	if(id == AD_CONVERTER && msg_type == 1)
	{
		power = (uint8_t) val1;
		printf("Battery info updated : %u\n", power);
		return 0;
	}
	else if (id == AD_CONVERTER)
		return 0;
	// Only longitude and latitude are sent through sigfox, no mean or dev
	else if (id == GPS && msg_type == 1)
		return 0;


	//Time is always inferior to 12h = 43200 : coded on only two bytes
 	//Because of endianess, the least significant bytes are written first.
 	// The last 2 bytes of the uint32_t time should then be 0
	memcpy(&msg[0], &t, 4); 

	//Write power in the third byte
	msg[2] = power;

	// Write message id and type in byte 4.
	msg[3] = (id << 2);
	msg[3] |= msg_type;

	memcpy(&msg[4], &val1, 4);
	memcpy(&msg[8], &val2, 4);
	
	return 1;
}
/*
uint8_t setup_sgf_card(arm_t *sgf_card)
{
	armError_t err = ARM_ERR_NONE;

	err = armInit(sgf_card, TTY);
	if(err != ARM_ERR_NONE)
	{
		printf("Init card sigfox failed!\n");
		printArmErr(err);
		armDeInit(sgf_card);
		return 1;
	}
	err = armSetMode(sgf_card, ARM_MODE_SFX);
	if(err != ARM_ERR_NONE)
	{
		printf("Init card sigfox failed!\n");
		printArmErr(err);
		armDeInit(sgf_card);
		return 1;
	}
	err = armSfxEnableDownlink(sgf_card, 1);
	if(err != ARM_ERR_NONE)
	{
		printf("Couldn't enable sigfox downlink!\n");
		printArmErr(err);
		armDeInit(sgf_card);
		return 1;
	}
	
	err = armUpdateConfig(sgf_card);
	if(err != ARM_ERR_NONE)
	{
		printf("Couldn't update sigfox configuration!\n");
		printArmErr(err);
		armDeInit(sgf_card);
		return 1;
	}
}




void printArmErr(armError_t err)
{
	switch(err)
	{
		case ARM_ERR_NONE:
			printf("ARM_ERR_NONE: 'No error.'\n");
		break;

		case ARM_ERR_NO_SUPPORTED:
			printf("ARM_ERR_NO_SUPPORTED: 'Functionality no supported by theARM.'\n");
		break;

		case ARM_ERR_PORT_OPEN:
			printf("ARM_ERR_PORT_OPEN: 'Port Error, at the port opening.'\n");
		break;

		case ARM_ERR_PORT_CONFIG:
			printf("ARM_ERR_PORT_CONFIG: 'Port Error, at the port configuring.'\n");
		break;

		case ARM_ERR_PORT_READ:
			printf("ARM_ERR_PORT_READ: 'Port Error, at the port reading.'\n");
		break;

		case ARM_ERR_PORT_WRITE:
			printf("ARM_ERR_PORT_WRITE: 'Port Error, at the port writing.'\n");
		break;

		case ARM_ERR_PORT_WRITE_READ:
			printf("ARM_ERR_PORT_WRITE_READ: 'Port Error, at the port reading/writing.'\n");
		break;

		case ARM_ERR_PORT_CLOSE:
			printf("ARM_ERR_PORT_CLOSE: 'Port Error, at the port closing.'\n");
		break;

		case ARM_ERR_PARAM_OUT_OF_RANGE:
			printf("ARM_ERR_PARAM_OUT_OF_RANGE: 'Error, one or more of parameters is out of range.'\n");
		break;

		case ARM_ERR_PARAM_INCOMPATIBLE:
			printf("ARM_ERR_PARAM_INCOMPATIBLE: 'Error, the parameters is incompatible between them.'\n");
		break;

		case ARM_ERR_ADDRESSING_NOT_ENABLE:
			printf("ARM_ERR_ADDRESSING_NOT_ENABLE: 'Error, the addressing is not enable.'\n");
		break;

		case ARM_ERR_WOR_ENABLE:
			printf("ARM_ERR_WOR_ENABLE: 'Error, the WOR mode is enable.'\n");
		break;

		case ARM_ERR_ARM_GO_AT:
			printf("ARM_ERR_ARM_GO_AT: 'ARM commend Error, can't switch to AT commend.'\n");
		break;

		case ARM_ERR_ARM_BACK_AT:
			printf("ARM_ERR_ARM_BACK_AT: 'ARM commend Error, can't quit AT commend.'\n");
		break;

		case ARM_ERR_ARM_CMD:
			printf("ARM_ERR_ARM_CMD: 'ARM commend Error, from AT commend.'\n");
		break;

		case ARM_ERR_ARM_GET_REG:
			printf("ARM_ERR_ARM_GET_REG: 'ARM commend Error, from get register.'\n");
		break;

		case ARM_ERR_ARM_SET_REG:
			printf("ARM_ERR_ARM_SET_REG: 'ARM commend Error, from set register.'\n");
		break;

		default:
			printf("ARM_ERR_UNKNON: 'Error unknown'\n");
		break;
	}
}*/
