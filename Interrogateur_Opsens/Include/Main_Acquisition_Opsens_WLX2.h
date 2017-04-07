#ifndef MAIN__H
#define MAIN__H


#ifndef HEADERS__H
#include "Headers.h"
#endif

#ifndef CONFIGURATION_PGM__H
#include "Configuration_Programme_Opsens_WLX2.h"
#endif

#ifndef CONFIGURATION_MOD__H
#include "Configuration_Module_Opsens_WLX2.h"
#endif

#ifndef CONNEXION__H
#include "Connexion_Opsens_WLX2.h"
#endif

#ifndef PREPARATION_ACQ__H
#include "Preparation_acquisition_Opsens_WLX2.h"
#endif

#ifndef LOAD_CONFIG_FILE__H
#include "Load_Config_File_Opsens_WLX2.h"
#endif

#ifndef ACQUISITION__H
#include "Acquisition_Opsens_WLX2.h"
#endif

void * acq_WLX2(void * args);

typedef struct cleanup_args{
	void ** mem_to_free[20];
	uint8_t nb_of_malloc;
	uint8_t *alive;
	pthread_mutex_t *mutex;
} cleanup_struct;

#endif

