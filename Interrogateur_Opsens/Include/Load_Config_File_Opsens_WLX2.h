#ifndef LOAD_CONFIG_FILE__H
#define LOAD_CONFIG_FILE__H

#ifndef HEADERS__H
#include "Headers.h"
#endif

#ifndef CONFIGURATION_PGM__H
#include "Configuration_Programme_Opsens_WLX2.h"
#endif


#define NB_MEASURE_MAX_BY_PAQUET 250


#define NOMFIC_CONFIG_FILE "Interrogateur_Opsens/Config/Acquisition_Opsens_WLX2_parameters"

int Load_config_file(struct config_all *pconfig, char **module_idn);

int Find_values_Samplingrate_measurerate(int Freq_echantillonnage, int *S, int *M);

#endif
