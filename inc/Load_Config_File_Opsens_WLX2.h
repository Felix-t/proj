#ifndef LOAD_CONFIG_FILE__H
#define LOAD_CONFIG_FILE__H

#include "headers.h"
#include "Configuration_Programme_Opsens_WLX2.h"

#define NB_MEASURE_MAX_BY_PAQUET 250

#define NOMFIC_CONFIG_FILE "conftest"

int Load_config_file(struct config_all *pconfig, char **module_idn);

uint8_t save_zeros_offset(float zeros_1, float zeros_2);

#endif
