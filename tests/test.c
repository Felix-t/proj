#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <byteswap.h>

static uint8_t set_interface_attribs (int fd, int speed, int parity);
static uint8_t set_blocking (int fd, int should_block);
static void parse_downlink(uint8_t *msg_received);
static void receive_downlink(int fd, uint8_t *msg_received);

static void receive_downlink(int fd, uint8_t *msg_received)
{
	FILE *fp = fopen("log_tests", "a");
	FILE *fp2 = fopen("received", "a");
	uint8_t i,bytes_rcv,  save = 0, bytes_ack = 0;
	uint8_t rcv_msg[100];
	write(fd, "+++", 3);
	sleep(2);
	write(fd, "ATS000=d0\n", 10);
	sleep(2);
	write(fd, "ATQ\n", 4);
	sleep(2);
	write(fd, "ACK", 3);
	sleep(60);
	fprintf(fp, "Checking for Aknowledgment... \n");
	printf("Mark 1\n");
	do
	{
		bytes_rcv = read(fd, rcv_msg, 100);

		for(i=0 ; i<bytes_rcv ; i++)
		{
			if(save && bytes_ack < 8)
			{
				msg_received[bytes_ack++] = rcv_msg[i];
				fprintf(fp2, "%i\t", rcv_msg[i]);
			}

			else if(i > 1 
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
			fprintf(fp, "%c", rcv_msg[i]);

		}
	}while(bytes_rcv != 0);
	printf("Mark 2\n");
	sleep(2);
	write(fd, "+++", 3);
	sleep(2);
	write(fd, "ATS000=50\n", 10);
	sleep(2);
	write(fd, "ATQ\n", 4);
	sleep(2);
	fclose(fp);
	fprintf(fp2, "\n");
	printf("Mark 3\n");
	fclose(fp2);
}


static void parse_downlink(uint8_t *msg_received)
{
	FILE *fp = fopen("log_tests", "a");
	char cfg_str[100] = "";
	uint32_t tmp;
	float cfg_val;
	switch(msg_received[0]){
	case 0:
		strcat(cfg_str, "ACQ_TIME");
		break;
	case 1:
		strcat(cfg_str, "MIN_VOLT");
		break;
	case 2:
		strcat(cfg_str, "MIN_VOLT");
		break;
	case 3:
		strcat(cfg_str, "THRESHOLD");
		break;
	case 4:
		strcat(cfg_str, "freq_echantillonnage");
		break;
	case 5:
		strcat(cfg_str, "zeros");
		break;
	default:
		strcat(cfg_str, "ACQ_TIME");
		break;
	}
	printf("%p\n", &cfg_val);
	tmp = msg_received[1] << 24 
		| (msg_received[2] << 16) 
		| (msg_received[3] << 8) 
		| msg_received[4]; 
	memcpy(&cfg_val, &tmp, 4);
	fprintf(fp, "Set config %s to %f\n", cfg_str, cfg_val);
	//set_cfg((char **) &cfg_str, (float) &cfg_val, 1);
	sleep(2);
	fclose(fp);

}


int main()
{
	int i = 0, u = 1, j = 0;

	uint8_t msg[8] = {0x02,0x40,0x80,2,3,0,0,0};
	uint32_t tmp = 0;
	tmp = msg[1] << 24 | (msg[2] << 16) | (msg[3] << 8) | msg[4]; 

	float a ;
	memcpy(&a, &tmp, 4);
	printf("%f\n", a);
	char *portname = "/dev/SGF";
	uint8_t in_messages[500];
	uint8_t msg_rcv[8];


	int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	set_interface_attribs (fd, B19200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking
	receive_downlink(fd, msg_rcv);
	parse_downlink(msg_rcv);
	char send[8];
	sprintf(send, "%f", (float) msg_rcv[1]);

	write(fd, send, 3);
	sleep(8);


	printf("All out_messages sent\n");
	close (fd);
}



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



