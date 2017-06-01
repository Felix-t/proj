#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>


static uint8_t set_interface_attribs (int fd, int speed, int parity);
static uint8_t set_blocking (int fd, int should_block);

int main()
{
	sleep(10);
	int i = 0, u = 1, j = 0;

	FILE *fp = fopen("log_tests", "a");

	char *portname = "/dev/ttyUSB0";
	uint8_t in_messages[500];
	uint8_t out_messages[9][12] = {
		{0,1,2,3,4,5,6,7,8,9,10,11},
		{'+','+','+'},
		{'A','T','S','0','0','0','=','d','0','\n'},
		{'A','T','Q','\n'},
		{'a','b','c','d','e','f','g','h','i','j','k','l'},
		{'+','+','+'},
		{'A','T','S','0','0','0','=','5','0','\n'},
		{'A','T','Q','\n'},
		{0,1,2,3,4,5,6,7,8,9,10,11}};

	int fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
	set_interface_attribs (fd, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking
	
	char * str = "$PMTK300,1000,0,0,0,0*1C\r\n";
	write(fd, str, strlen(str));
	str = "$PMTK220,1000*1F\r\n";
	write(fd, str, strlen(str));
	str = "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28";
	write(fd, str, strlen(str));
	str = "$PMTK313,1*2E\r\n";
	write(fd, str, strlen(str));
	str = "$PMTK301,2*2E\r\n";
	write(fd, str, strlen(str));
	sleep(8);
	//set_interface_attribs (fd, B38400, 0);  // set speed to 115,200 bps, 8n1 (no parity)
   struct timespec tt = {
               .tv_sec = 0,        /* seconds */
               .tv_nsec = 500000000       /* nanoseconds */
           };

	while(j < 200)
	{
		u = read(fd, in_messages, 500);

		for(i=0;i<u;i++)
			fprintf(fp, "%c", in_messages[i]);
		nanosleep(&tt, NULL);
		j++;
	}
	printf("All out_messages sent\n");
	fclose(fp);
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



