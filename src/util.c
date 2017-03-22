#include "include.h"

#include "util.h"

void copy(char *source, char *dest)
{
    int childExitStatus;
    pid_t pid;
    int status;
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
		status = WEXITSTATUS(childExitStatus); /* zero is normal exit */
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
    int status;

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
		status = WEXITSTATUS(childExitStatus); /* zero is normal exit */
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



void set_next_startup(int32_t startup_time)
{
	FILE *fp;
	char * str;
	fp=fopen("schedule.wpi", "r+");

	// Write proper shutdown time in the file
	fscanf(fp, str);
	printf("%s", str);
	int fclose(FILE *a_file);

	copy("schedule.wpi", "/home/pi/wittyPi/");
	exec_script("/home/pi/wittyPi/runScript.sh");
}


