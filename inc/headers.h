#ifndef HEADER_H
#define HEADER_H

/*
 * startup.c:
 *	Program launched on startup 
 *	 - Check remaining battery power
 *	 - Schedule next startup and shutdown 
 *	 - Start acquisitions subroutines
 */
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include <bcm2835.h>
#include <libconfig.h>
#include <pthread.h>

#define MAG_ACQ 0
#define ACC_GYR 1
#define WLX2 0

extern uint8_t end_program;//To be modified only by main program to shutdown other threads

#endif
