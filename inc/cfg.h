#ifndef CFG_H
#define CFG_H

/*
 * startup.c:
 *	Program launched on startup 
 *	 - Check remaining battery power
 *	 - Schedule next startup and shutdown 
 *	 - Start acquisitions subroutines
 *  @TODO : Logging system, see syslog
 */
#define CFG_FILE "conftest"

#include "headers.h"
#include "util.h"

enum {MAX_VOLT,    		//Battery volt capacity
     TRESHOLD,    		//Battery treshold
     LAST_DISCHARGE, 		//Discharge volt last acquisition
     ACQ_TIME};     		//Discharge time last acquisition


void get_cfg_str(const char **values, char **str,int32_t str_nb);
void get_cfg_double(double *values, char **str, int32_t str_nb);
void set_cfg(char ** str, double *values, int32_t str_nb);

#define get_cfg(_1, ...) _Generic((_1),                                  \
                              double *: get_cfg_double,                          \
                              const char **:get_cfg_str                       \
				) (_1, __VA_ARGS__)
#define FIRST(A, ...) A



#endif
