/*
 * startup.c:
 *	Program launched on startup 
 *	 - Check remaining battery power
 *	 - Schedule next startup and shutdown 
 *	 - Start acquisitions subroutines
 *  @TODO : Logging system, see syslog
 */
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <stdint.h>

#define MAG_ACQ 0
#define ACC_GYR 1
#define WLX2 0

int8_t end_program;//To be modified only by main program to shutdown other threads
