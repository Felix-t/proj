#include "headers.h"
#include "sigfox.h"
#include <termios.h>
#include <errno.h>
#include <fcntl.h>

static uint8_t build_message(uint8_t msg[12], uint32_t t, float val1, float val2, 
		identity id, uint8_t msg_type);
static time_t get_ref_time();
static uint8_t set_interface_attribs (int fd, int speed, int parity);
static uint8_t set_blocking (int fd, int should_block);


static uint8_t set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf("error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                printf("error %d from tcsetattr", errno);
                return 0;
        }
        return 1;
}

static uint8_t set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                printf("error %d from tggetattr", errno);
                return 0;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
		printf("error %d setting term attributes", errno);
	return 1;
}


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
		printf("loop :  %li\n",
			syscall(__NR_gettid));
		sleep(2);
	}
	printf("Thread send data exited\n");
	pthread_exit(0);
}


static time_t get_ref_time()
{
	time_t t = time(NULL);
	struct tm *tmp = localtime(&t);
	// @TODO : start de la journée ? quelle valeur de réf:
	tmp->tm_sec = 0;
	tmp->tm_min = 0;
	tmp->tm_hour = tmp->tm_hour >= 12 ? 12 : 0; 
	return mktime(tmp);
}


void *sigfox(void* args)
{
	printf("Thread sigfox created, id : %li\n",
			syscall(__NR_gettid));

	char *portname = "/dev/ttyUSB0";

	time_t last_msg = time(NULL);
	int i, pos_in = 0, pos_out = 0;
	uint32_t t;
	uint8_t *messages[MAX_NB_MSG + 1];

	time_t ref_time = get_ref_time();

	int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	
	if (fd < 0)
	{
		printf("error %d opening %s: %s", errno, portname, strerror (errno));
		alive[SGF] = 2;
		pthread_exit((void *) 0);
	}
	set_interface_attribs (fd, B19200, 0);  // set speed to 19,200 bps, 8n1 (no parity)
	set_blocking (fd, 1);                // set no blocking


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
	
	alive[SGF] = 1;

	sgf_msg.write_allowed = 1;

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
			if(difftime(sgf_msg.time, ref_time) > 3600*12)
				ref_time = get_ref_time();
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
	// val1 of msg_type 2 is the relative mean, which is what we're after 
	// Battery information (min max std_dev) is not send through sigfox
	if(id == AD_CONVERTER && msg_type == 1)
	{
		power = (uint8_t) val1;
		printf("Battery info updated : %u\n", power);
		return 0;
	}
	else if (id == AD_CONVERTER)
		return 0;

	//Time is always inferior to 12h = 43200 : coded on only two bytes
 	//Because of endianess, the least significant bytes are written first.
 	// The last 2 bytes of the uint32_t time should then be 0
	memcpy(&msg[0], &t, 4); 

	//Write power in the third byte
	msg[2] = power;

	// Write message id and type in byte 4.
	msg[3] = (id << 1);
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
