#ifndef CONFIGURATION_PGM__H
#define CONFIGURATION_PGM__H

#include "headers.h"
#include "Fonctions_Utiles.h"

#ifndef _INET_H
#define _INET_H
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define NB_CH 2

#define _STR_LONG 1000
#define _STR_SHORT 500

struct config_save_file
{
	float size_max_save_file;
	int usb;
	char *nom_projet;
	char *nomfic;
	char *rep_usb;
};

struct config_meas
{
	int mode_meas;
	int mode_debug;
	int sms_alert;
	int *select_ch; 
	char *type_jauge_ch[NB_CH];
	char *numero_jauge_ch[NB_CH];
	int (*GFx_jauge_ch)[4];
	int SAMPLingrate;
	int MEASureRATE;
	int Freq_echantillonnage;
	float zeros[NB_CH];
};


struct config_all
{
	struct config_save_file *pconfig_save_file;
	struct config_meas *pconfig_meas;
};


void Init_struct_config_save_file(struct config_save_file *pconfig_save_file, char *nom_projet,char *nomfic,char *rep_usb);

void Init_struct_config_meas(struct config_meas *pconfig_meas, int *select_ch, int (*GFx_jauge_ch)[4], char *numero_jauge_ch[NB_CH], char *type_jauge_ch[NB_CH]);

void Init_struct_config_all(struct config_all *pconfig, struct config_save_file *pconfig_save_file, struct config_meas *pconfig_meas);

//void Init_struct_param_pgm(struct param_pgm *pparam_pgm, struct config_all *pconfig_all, struct shared *pshared);

void Print_Init_struct_config_save_file(struct config_save_file *pconfig_save_file);

void Print_struct_config_meas(struct config_meas *pconfig_meas);

void Print_struct_config_all(struct config_all *pconfig);

#endif
