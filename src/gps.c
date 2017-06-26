#include "gps.h"
#include "serial.h"
#include "cfg.h"
#include <termios.h>
#include "errno.h"
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>


static uint8_t open_new_file(FILE **fp)
{
	if(*fp != NULL)
		fclose(*fp);

	static DIR* FD;
	struct dirent* in_file;

	static int file_count = 0;
	static char *path_base = NULL;
	char *path = malloc(100);

	// Path specified in the config, add a suffixe to it
	if(path_base == NULL)
	{
		path_base = malloc(100);
		char *cfg = "PATH_GPS_DATA";
		get_cfg(&path_base, &cfg, 1);
		if(NULL == (FD = opendir(path_base)))
		{
			printf("Creating directory %s ...", path_base);
			mkdir(path_base, S_IRWXU | S_IRWXG | S_IRWXO);
			if(NULL == (FD = opendir(path_base)))
			{
				printf("Error opening output directory\n");
				return 0;
			}
			printf("ok\n");
		}
		while ((in_file = readdir(FD))) 
		{
			sprintf(path, "data_%i", file_count);
			if (!strcmp(in_file->d_name, path))
			{
				printf("in file : %s\t d_name : %s\n", in_file->d_name, path);
				file_count++;
				rewinddir(FD);
			}
		}
		closedir(FD);
	}

	sprintf(path, "%sdata_%i", path_base, file_count++);

	if(!(*fp = fopen(path, "w+")))
	{
		printf("Can't open file %s\n", path);
		return 0;
	}

	printf("\tSaving GPS data to file : %s\n", path);

	free(path);
	return 1;
}

void *gps(void * args)
{
	printf("Thread GPS created, id : %li\n",syscall(__NR_gettid));
	alive[GPS] = 1;
	int i = 0, u = 1;

	FILE *fp = NULL;
	open_new_file(&fp);	

	uint8_t in_messages[500];

	int fd = open (GPS_PORT, O_RDWR | O_NOCTTY | O_SYNC);
	
	if (fd < 0)
	{
		printf("error %d opening %s: %s", errno, GPS_PORT, strerror (errno));
		alive[GPS] = 0;
		pthread_exit((void *) 0);
	}

	set_interface_attribs (fd, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
	set_blocking (fd, 0);                // set no blocking
	
	char * str = "$PMTK300,1000,0,0,0,0*1C\r\n";
	write(fd, str, strlen(str));
	str = "$PMTK220,1000*1F\r\n";
	write(fd, str, strlen(str));

	// Uncomment to only get minimal coordinates information (GPGGA, GPRMC)
	/*
	str = "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28";
	write(fd, str, strlen(str));
	*/
	str = "$PMTK314,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0*28";
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

	while(!end_program)
	{
		nanosleep(&tt, NULL);
		

		// Open new file is old one is full
		if(ftell(fp) > SIZE_MAX_FILE*1024)
		{
			open_new_file(&fp);
		}

		u = read(fd, in_messages, 500);

		for(i=0;i<u;i++)
			fprintf(fp, "%c", in_messages[i]);
	}
	fclose(fp);
	close (fd);
	alive[GPS] = 0;
	pthread_exit((void *) 0);
}

