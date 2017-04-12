#include "util.h"
#include <dirent.h>
#include <errno.h>

static uint8_t exec_compression(char * path);

uint8_t copy(char *source, char *dest)
{
	pid_t pid;
	if (!source || !dest) {
		printf("Source or destination is not valid -copy aborted");
		return 0;
	}

	pid = fork();

	if (pid == 0) { /* child */
		execl("/bin/cp", "/bin/cp", source, dest, (char *)0);
		exit(0);
	}
	else if (pid < 0) {
		printf(" error - couldn't start process");
	}
	else {
		pid_t ws = waitpid( pid, NULL, WNOHANG);
		if (ws == -1)
		{
			printf("Error during copy");
			return 0;
		}
	}
	return 1;
}

uint8_t exec_script(char *cmd)
{
	pid_t pid;

	pid = fork();

	if (pid == 0) { /* child */
		execl(cmd, cmd, (char *)0);
		exit(0);
	}
	else if (pid < 0) {
		printf(" error - couldn't start process");
	}
	else {
		pid_t ws = waitpid( pid, NULL, WNOHANG);
		if (ws == -1)
		{ 
			printf("Error during cmd %s\n", cmd);
			return 0;
		}
	}
	return 1;
}

/* Function : Start wittyPi script to schedule next startup
 * Params : startup_time is time in sec until next startup
 * Return : 0 if error occured, 1 otherwise
 */
uint8_t set_next_startup(int32_t startup_time)
{
	FILE *fp;
	char str[100] = "";
	sprintf(str, "H%i M%i S%i", startup_time/3600, 
			(startup_time%3600)/60, 
			startup_time%60);

	pid_t pid = fork();
	if(pid == 0)
	{
		execl("/bin/cp", "/bin/cp", "schedule_base.wpi", "schedule.wpi", (char *) 0);
		exit(0);
	}
	if((fp=fopen("schedule.wpi", "a+")) == NULL)
	{
		printf("Error opening schedule.wpi");
		return 0;
	}
	fprintf(fp, "ON %s", str);
	pid = fork();
	if (pid == 0)
	{
		execl("/bin/mv", "/bin/mv", "schedule.wpi", "/home/pi/wittyPi/", (char *) 0);
		exit(0);
	}
	exec_script("/home/pi/wittyPi/runScript.sh");

	fclose(fp);
	return 1;
}

uint8_t move_logs()
{
	pid_t pid = fork();
	if (pid == 0) { /* child */
		execl("/bin/cp", "/bin/cp", "-u", "-r", "/home/pi/projet/logs/", "Data/", (char *) 0);
		exit(0);
	}
	else if (pid < 0) {
		printf(" error - couldn't start process");
		return 0;
	}
	else {
		sleep(TIMEOUT);
		pid_t ws = waitpid( pid, NULL,0);
		if (ws == -1)
		{
			printf("Error during mv logs/* Data/logs/");
			return 0;
		}
	}
	return 1;
}

uint8_t start_dhcp_server()
{
	pid_t pid = fork();
	if (pid == 0) { /* child */
		execl("/etc/init.d/dhcpcd", "/etc/init.d/dhcpcd", "stop",(char *) 0);
		exit(0);
	}
	else if (pid < 0) {
		printf(" error - couldn't start process");
		/* error - couldn't start process - you decide how to handle */
	}
	else {
		sleep(TIMEOUT);
		pid_t ws = waitpid( pid, NULL,0);
		if (ws == -1)
		{
			printf("Error during dhcpcd stop");
			return 0;
		}
	}
	pid = fork();
	if (pid == 0) { /* child */
		execl("/sbin/ifdown", "/sbin/ifdown", "eth0", "eth1",(char *) 0);
		exit(0);
	}
	else if (pid < 0) {
		printf(" error - couldn't start process");
		/* error - couldn't start process - you decide how to handle */
	}
	else {
		sleep(TIMEOUT);
		pid_t ws = waitpid( pid, NULL,0);
		if (ws == -1)
		{
			printf("Error during ifdown");
			return 0;
		}
	}
	pid = fork();
	if (pid == 0) { /* child */
		execl("/sbin/ifup", "/sbin/ifup", "eth0", "eth1",(char *) 0);
		exit(0);
	}
	else if (pid < 0) {
		printf(" error - couldn't start process");
		/* error - couldn't start process - you decide how to handle */
	}
	else {
		sleep(TIMEOUT);
		pid_t ws = waitpid( pid, NULL,0);
		if (ws == -1)
		{
			printf("Error during ifup");
			return 0;
		}
	}
	pid = fork();
	if (pid == 0) { /* child */
		execl("/etc/init.d/dnsmasq", "/etc/init.d/dnsmasq", "start",(char *) 0);
		exit(0);
	}
	else if (pid < 0) {
		printf(" error - couldn't start process");
		/* error - couldn't start process - you decide how to handle */
	}
	else {
		sleep(TIMEOUT);
		pid_t ws = waitpid( pid, NULL,0);
		if (ws == -1)
		{
			printf("Error during ifdown");
			return 0;
		}
	}
	return 1;
}


uint8_t archive_data()
{
	int i=1;
	DIR* FD;
	struct dirent* in_file;

	char file_name[100]="\0";
	char dir_name[100] = TEMPUSB;

	//Add time to the name of the file
	time_t t = time(NULL);
	int month = localtime(&t)->tm_mon;
	int year = localtime(&t)->tm_year - 100;
	int day = localtime(&t)->tm_mday;
	sprintf(file_name, "%i_%i_%i_data.tar.lrz", day, month, year);

	//Look if there is no file named file_name, change file_name accordingly
	if (NULL == (FD = opendir(TEMPUSB))) 
	{
		fprintf(stderr, "Error : Failed to open input directory - %s\n",
			       	strerror(errno));
		return 0;
	}
	while ((in_file = readdir(FD))) 
	{
		if (!strcmp(in_file->d_name, file_name))
		{
			strcpy(file_name, "\0");
			sprintf(file_name, "%i_%i_%i_data_%i.tar.lrz", 
					day, month, year, i++);
			rewinddir(FD);
		}
	}

	strcat(dir_name, file_name);
	printf("Compressed to %s\n", dir_name);
	return exec_compression(dir_name);
}


static uint8_t exec_compression(char * path)
{
	pid_t pid = fork();
	if (pid == 0) { /* child */
		execl("/usr/bin/lrztar", "/usr/bin/lrztar", "-l", "-L", "5",
				"-o", path, "Data/", (char *)0); //@TODO : remplacer Data/ par const ou config
		exit(0);
	}
	else if (pid < 0) {
		printf(" error - couldn't start process");
		return 0;
	}
	else {
		sleep(TIMEOUT);
		pid_t ws = waitpid( pid, NULL,0);
		if (ws == -1)
		{
			printf("Error during compression");
			return 0;
		}
		//execl("/bin/rm", "/bin/rm", "-r", "-f", "Data/Data/", (char *) 0);
		return 1;
	}
	return 1;
}
