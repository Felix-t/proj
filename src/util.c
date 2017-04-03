#include "util.h"
#include <dirent.h>
#include <errno.h>

void copy(char *source, char *dest)
{
	int childExitStatus;
	pid_t pid;
	if (!source || !dest) {
		/* handle as you wish */
	}

	pid = fork();

	if (pid == 0) { /* child */
		execl("/bin/cp", "/bin/cp", source, dest, (char *)0);
	}
	else if (pid < 0) {
		/* error - couldn't start process - you decide how to handle */
	}
	else {
		/* parent - wait for child - this has all error handling, you
		 * could just call wait() as long as you are only expecting to
		 * have one child process at a time.
		 */
		pid_t ws = waitpid( pid, &childExitStatus, WNOHANG);
		if (ws == -1)
		{ /* error - handle as you wish */
		}

		if( WIFEXITED(childExitStatus)) /* exit code in childExitStatus */
		{
			printf("Copy process ended normally\n");
		}
		else if (WIFSIGNALED(childExitStatus)) /* killed */
		{
			printf("Copy process killed\n");
		}
		else if (WIFSTOPPED(childExitStatus)) /* stopped */
		{
			printf("Copy process got stopped");
		}
	}
}

void exec_script(char *cmd)
{
	int childExitStatus;
	pid_t pid;

	pid = fork();

	if (pid == 0) { /* child */
		execl(cmd, cmd, (char *)0);
	}
	else if (pid < 0) {
		printf(" error - couldn't start process");
		/* error - couldn't start process - you decide how to handle */
	}
	else {
		/* parent - wait for child - this has all error handling, you
		 * could just call wait() as long as you are only expecting to
		 * have one child process at a time.
		 */
		pid_t ws = waitpid( pid, &childExitStatus, WNOHANG);
		if (ws == -1)
		{ /* error - handle as you wish */
			printf("Error during cmd %s\n", cmd);
		}

		if( WIFEXITED(childExitStatus)) /* exit code in childExitStatus */
		{
			printf("Child cmd %s ended normally\n", cmd);
		}
		else if (WIFSIGNALED(childExitStatus)) /* killed */
		{
			printf("Child cmd %s killed\n", cmd);
		}
		else if (WIFSTOPPED(childExitStatus)) /* stopped */
		{
			printf("Child cmd %s got stopped", cmd);
		}
	}
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
	
	execl("bin/cp", "bin/cp", "schedule_base.wpi", "schedule.wpi", (char *) 0);

	if((fp=fopen("schedule.wpi", "a+")) == NULL)
	{
		printf("Error opening schedule.wpi");
		return 0;
	}
	fprintf(fp, "ON %s", str);
	execl("bin/mv", "bin/mv", "schedule.wpi", "/home/pi/wittyPi/", (char *) 0);

	exec_script("/home/pi/wittyPi/runScript.sh");
	
	fclose(fp);
	return 1;
}

void move_logs()
{
	execl("bin/mv", "bin/mv", "logs/*", "Data/logs/", (char *) 0);
}

uint8_t start_dhcp_server()
{
	int childExitStatus;
	pid_t pid = fork();
	if (pid == 0) { /* child */
		execl("/etc/init.d/dhcpcd", "/etc/init.d/dhcpcd", "stop",(char *) 0);
	}
	else if (pid < 0) {
		printf(" error - couldn't start process");
		/* error - couldn't start process - you decide how to handle */
	}
	else {
		sleep(TIMEOUT);
		pid_t ws = waitpid( pid, &childExitStatus, WNOHANG);
		if (ws == -1)
		{
			printf("Error during dhcpcd stop");
			return 0;
		}
	}
	pid = fork();
	if (pid == 0) { /* child */
		execl("/sbin/ifdown", "/sbin/ifdown", "eth0", "eth1",(char *) 0);
	}
	else if (pid < 0) {
		printf(" error - couldn't start process");
		/* error - couldn't start process - you decide how to handle */
	}
	else {
		sleep(TIMEOUT);
		pid_t ws = waitpid( pid, &childExitStatus, WNOHANG);
		if (ws == -1)
		{
			printf("Error during ifdown");
			return 0;
		}
	}
	pid = fork();
	if (pid == 0) { /* child */
		execl("/sbin/ifup", "/sbin/ifup", "eth0", "eth1",(char *) 0);
	}
	else if (pid < 0) {
		printf(" error - couldn't start process");
		/* error - couldn't start process - you decide how to handle */
	}
	else {
		sleep(TIMEOUT);
		pid_t ws = waitpid( pid, &childExitStatus, WNOHANG);
		if (ws == -1)
		{
			printf("Error during ifup");
			return 0;
		}
	}
	pid = fork();
	if (pid == 0) { /* child */
		execl("/etc/init.d/dnsmasq", "/etc/init.d/dnsmasq", "start",(char *) 0);
	}
	else if (pid < 0) {
		printf(" error - couldn't start process");
		/* error - couldn't start process - you decide how to handle */
	}
	else {
		sleep(TIMEOUT);
		pid_t ws = waitpid( pid, &childExitStatus, WNOHANG);
		if (ws == -1)
		{
			printf("Error during ifdown");
			return 0;
		}
	}
	return 1;
}


void archive_data()
{
	DIR* FD;
	struct dirent* in_file;

	//@TODO: Scanning the directory to check if data files have been created ?
	//	if (NULL == (FD = opendir("logs/"))) 
	//	{
	//		fprintf(stderr, "Error : Failed to open input directory - %s\n", strerror(errno));
	//	}
	//	while ((in_file = readdir(FD))) 
	//	{
	//		if (!strcmp (in_file->d_name, "."))
	//			continue;
	//		if (!strcmp (in_file->d_name, ".."))    
	//			continue;
	//		printf("%s\n", in_file->d_name);
	//	}
	//else
	
		time_t t = time(NULL);
		char d[100]="00";
		int month = localtime(&t)->tm_mon;
		int year = localtime(&t)->tm_year - 100;
		int day = localtime(&t)->tm_mday;
		sprintf(d, "tempUSB/%i_%i_%i_data.tar.lrz", day, month, year);

		execl("/usr/bin/lrztar", "/usr/bin/lrztar", "-l", "-L", "5", "-o", d, "trylrz/", (char *)0);
		execl("bin/rm", "bin/rm", "-r", "Data/*", (char *) 0);
	
}
