#ifndef MAIN__H
#define MAIN__H

#include "headers.h"
#include "Configuration_Programme_Opsens_WLX2.h"
#include "Configuration_Module_Opsens_WLX2.h"
#include "Connexion_Opsens_WLX2.h"
#include "Load_Config_File_Opsens_WLX2.h"
#include "Acquisition_Opsens_WLX2.h"
#include "Preparation_acquisition_Opsens_WLX2.h"

void * acq_WLX2(void * args);

typedef struct cleanup_args{
	void ** mem_to_free[20];
	uint8_t nb_of_malloc;
	pthread_mutex_t *mutex;
} cleanup_struct;

#endif

