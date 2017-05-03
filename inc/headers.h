#ifndef HEADER_H
#define HEADER_H

#define __USE_XOPEN
#define _GNU_SOURCE
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

#define SGF_ENABLE 1  //Sigfox enabled
#define LSM9DS0_MAG_ENABLE 0
#define LSM9DS0_ENABLE 1
#define WLX2_ENABLE 1
#define SHUTDOWN 0


#define TEMPUSB "tempUSB/" //@ TODO: for debug purposes

#define SGF_SEND_PERIOD 60


typedef enum {WLX2_CH1,
	WLX2_CH2,
	LSM9DS0_ACC_X,
	LSM9DS0_ACC_Y,
	LSM9DS0_ACC_Z,
	LSM9DS0_GYR_X,
	LSM9DS0_GYR_Y,
	LSM9DS0_GYR_Z,
	LSM9DS0_MAG_X,
	LSM9DS0_MAG_Y,
	LSM9DS0_MAG_Z,
	AD_CONVERTER} identity;



struct sgf_data{
	float min;
	float max;
	float mean;
	float std_dev;
	time_t time;
	pthread_mutex_t mutex;
	identity id;
	uint8_t write_allowed;
};

 //To be modified only by main program to shutdown other threads
extern _Atomic uint8_t end_program;

// @ TODO : message queue global var or pass through pointer
struct sgf_data sgf_msg;

//Forward declaration of acq_wlx
extern void * acq_WLX2(void *);

#endif
