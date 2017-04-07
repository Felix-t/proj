#ifndef ACQUISITION__H
#define ACQUISITION__H

#ifndef HEADERS__H
#include "Headers.h"
#endif

#ifndef CONFIGURATION_PGM__H
#include "Configuration_Programme_Opsens_WLX2.h"
#endif

#ifndef CONNEXION__H
#include "Connexion_Opsens_WLX2.h"
#endif

#ifndef FUNCTIONS_UTILES__H
#include "Fonctions_Utiles.h"
#endif

#ifndef PREPARATION_ACQ__H
#include "Preparation_acquisition_Opsens_WLX2.h"
#endif


struct stUDPMeasureHeader_t
{
uint16_t ui16DataSize;
uint8_t ui8DataType;
uint8_t ui8ModuleID;
uint8_t ui8ChannelID;
uint8_t ui8MeasureUnit;
uint16_t ui16Year;
uint8_t ui8Month;
uint8_t ui8Day;
uint8_t ui8Hour;
uint8_t ui8Min;
uint8_t ui8Seconds;
uint8_t ui810thOfSeconds;
};


struct stUDPSendMeasureHeader_t
{
uint16_t ui16DataID;
uint8_t ui8SegmentID;
uint8_t ui8SegmentQty;
struct stUDPMeasureHeader_t stMeasureHeader;
};


#define _MAX_FLOAT_MEASURES ((_UDP_MAX_PACKET_SIZE-sizeof(struct stUDPSendMeasureHeader_t))/sizeof(float))

struct stUDPSendMeasureType_t
{
struct stUDPSendMeasureHeader_t stHeader;
float fMeasure[_MAX_FLOAT_MEASURES];
};

struct sendToSgf
{
	float mean;
	float std_dev;
	float min;
	float max;
}

void stats(struct stUDPSendMeasureType_t *ch1, struct stUDPSendMeasureType_t *ch2, int nb_measures);

int Open_file_Enregistrement_data(struct param_pgm *param);

int Close_file_Enregistrement_data(struct param_pgm *param);

void Print_Entete_file_Enregistrement_data(struct param_pgm *param );

int N_Measure_start (struct parametres_connexion *param_connection, int nb_meas_to_do);

int Measure_stop (struct parametres_connexion *param_connection);

int Measure_start_infinite (struct parametres_connexion *param_connection);

int Get_single_measurement (struct param_pgm *p_data);

int Run_Thread_Enregistrement_data(struct param_pgm *param);

int Stop_Thread_Enregistrement_data(struct param_pgm *param);

int Fermeture_ouverture_new_file(struct param_pgm *param);

int Zero_sensor(struct parametres_connexion *param_connection, int ch1_zero, int ch2_zero);

void* thread_Enregistrement_data (void* arg);

int Reception_data(struct parametres_connexion *param_connection, struct stUDPSendMeasureType_t *SendMeasureType_t, int ok_print);

int Zero_stUDPSendMeasureType_t(struct stUDPSendMeasureType_t *SendMeasureType_t);

int Print_stUDPSendMeasureType_t(struct stUDPSendMeasureType_t *SendMeasureType_t);

void Convert_stUDPSendMeasureType_t__to__string(struct stUDPSendMeasureType_t *SendMeasureType_t, char *string_SendMeasureType_t);

int Get_zero_sensor(struct parametres_connexion *param_connection, int ch_zero, float *zero_channel);

int Get_offset_sensor(struct parametres_connexion *param_connection, int ch_offset, float *offset_channel);

int Set_offset_sensor(struct parametres_connexion *param_connection, int ch_offset, float offset_channel);

#endif
