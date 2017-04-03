#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>

#define TIMEOUT 2
int main()
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
	return 0;
}
