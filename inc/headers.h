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

#define SGF 1  //Sigfox enabled
#define MAG_ACQ 0
#define ACC_GYR 0
#define WLX2 1

extern _Atomic uint8_t end_program;//To be modified only by main program to shutdown other threads

//Forward declaration of acq_wlx
extern void * acq_WLX2(void *);

//TODO :  fichier sigfox
#define SGF_INTERVAL 10  // 140 messages/jour : 3600*24/140

#endif
