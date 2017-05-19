#ifndef CONFIGURATION_MOD__H
#define CONFIGURATION_MOD__H

#include "headers.h"
#include "Configuration_Programme_Opsens_WLX2.h"
#include "Connexion_Opsens_WLX2.h"



struct sensor_def
{
	char sensor_type[2];
	int sensor_GF0;
	int sensor_GF1;
	int sensor_GF2;
	int sensor_GF3;
};


int Configuration_WLX2(struct parametres_connexion *param_connection, struct config_all *pconfig_all);

int Configuration_WLX2_date_time(struct parametres_connexion *param_connection);

int Date_time_saisie(struct parametres_connexion *param_connection, int choix_modif, char *str_ans_time,char *str_ans_date);

int Get_date_time_from_RPI(char *current_date, char *current_time);

int Get_date_time_from_WLX2(struct parametres_connexion *param_connection, char *current_time);

int Change_date_time_RPI(char *current_time);

int Change_date_WLX2(struct parametres_connexion *param_connection, char *current_date);

int Change_time_WLX2(struct parametres_connexion *param_connection, char *current_time);

int Compare_date_time_RPI_WLX2(char *current_time_RPI, char *current_time_WLX2);

int fgets_stdin(char *str_line,int size_str_line);

int Configuration_WLX2_channel(struct parametres_connexion *param_connection, struct config_all *pconfig_all);

int Delete_all_sensor(struct parametres_connexion *param_connection);

int Add_sensors(struct parametres_connexion *param_connection, struct config_all *pconfig_all);

int Get_sensors_list(struct parametres_connexion *param_connection, char *answer);

int Analyse_get_sensors_list(char *str_sensor_list, struct sensor_def sensor_list[2]);

int Associate_sensors_to_channel(struct parametres_connexion *param_connection, struct config_all *pconfig_all);

int Get_associate_sensors_to_channel(struct parametres_connexion *param_connection, struct config_all *pconfig_all, int number_channel);

int Desactivation_channel(struct parametres_connexion *param_connection, int i_channel);

int Activation_channel(struct parametres_connexion *param_connection, int i_channel);

int Activation_channels(struct parametres_connexion *param_connection, struct config_all *pconfig_all);

int Get_status_channel(struct parametres_connexion *param_connection, struct config_all *pconfig_all, int number_channel);

int Configuration_WLX2_SAMPLingrate_MEASureRATE(struct parametres_connexion *param_connection, struct config_all *pconfig_all);

int Set_SAMPLingrate(struct parametres_connexion *param_connection, struct config_all *pconfig_all);

int Get_SAMPLingrate(struct parametres_connexion *param_connection, char *answer);

int Set_MEASureRATE(struct parametres_connexion *param_connection, struct config_all *pconfig_all);

int Get_MEASureRATE(struct parametres_connexion *param_connection, char *answer);

int Show_system_description(struct parametres_connexion *param_connection);

int Set_system_description(struct parametres_connexion *param_connection);



#endif
