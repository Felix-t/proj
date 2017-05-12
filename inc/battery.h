#ifndef BATTERY_H
#define BATTERY_H

#include "headers.h"
#include "cfg.h"
#include "util.h"
#include "ADS1256.h"


#define CH_NUM 0
#define NB_MEASURES 100
#define MEASURE_FREQUENCY 100 	//X every sec
#define INTERVAL 30 		//Between each series of data

#define PATH_VOLT_LOGS "logs/tension.txt"
#define NB_CFG_BATTERY 5
void *battery(void *arg);

enum {MAX_VOLT,	//Battery volt capacity
	MIN_VOLT,
	THRESHOLD,    		//Battery 
	LAST_DISCHARGE, 		//Discharge volt last acquisition
	ACQ_TIME};     		//Discharge time last acquisition


#endif
