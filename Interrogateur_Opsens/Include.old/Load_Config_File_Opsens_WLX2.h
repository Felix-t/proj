#ifndef LOAD_CONFIG_FILE__H
#define LOAD_CONFIG_FILE__H

#ifndef HEADERS__H
#include "Headers.h"
#endif

#ifndef CONFIGURATION_PGM__H
#include "Configuration_Programme_Opsens_WLX2.h"
#endif


#define NB_MEASURE_MAX_BY_PAQUET 250

const int FREQ_ECHANTILLONNAGE_VALUES_AUTHORIZED[]={13, 1, 2, 4, 5, 10, 20, 25, 50, 100, 125 , 250, 500};
const int SAMPLINGRATE_VALUES_AUTHORIZED[]={4, 100, 250, 500};

#define NOMFIC_CONFIG_FILE "Config/Acquisition_Opsens_WLX2_parameters"

int Load_config_file(struct config_all *pconfig);

int Find_values_Samplingrate_measurerate(int Freq_echantillonnage, int *S, int *M);

#endif