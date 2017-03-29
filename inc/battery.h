#ifndef BATTERY_H
#define BATTERY_H

#include "headers.h"
#include "cfg.h"
#include "util.h"
#include "ADS1256.h"


#define CH_NUM 0
#define NB_MEASURES 500
#define MEASURE_FREQUENCY 50 	//X every sec
#define INTERVAL 30 		//Between each series of data

#define PATH_VOLT_LOGS "logs/tension.txt"

void *battery(void *arg);

#endif
