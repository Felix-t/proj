/*====================================================================================================*/
/* Pilotage of WLX2 via UDP protocol (Programming in C)                                               */
/*----------------------------------------------------------------------------------------------------*/
/* Program sends requests/commands (SCPI) to WLX2 and performs data acquisition                       */
/*----------------------------------------------------------------------------------------------------*/
/* Compiler/IDE  : gcc 4.9.2                                                                          */
/* Library       :                                                                                    */
/* Commands      : gcc -lpthread Acquisition_Opsens_dev.c -o Acquisition_Opsens_dev                   */
/* OS            : Raspbian GNU/Linux 8 (jessie)                                                      */                              
/* Programmer    : X. Chapeleau                                                                       */
/* Date	         : 24-mai-2016                                                                        */
/*====================================================================================================*/

/*====================================================================================================*/
/* Copyright (C) 2016 X. Chapeleau IFSTTAR COSYS SII                                                  */
/*====================================================================================================*/
/* TODO before running this program:                                                                  */
/* ---------------------------------------------------------------------------------------------------*/ 
/* 1- start dhcp server                                                                               */
/* 2- connect WLX2 to raspberry Pi with Ethernet cable                                                */
/*IP of WLX2: 10.0.0.15 (automatically by dhcp server: see file /etc/dhcp/dhcpd.conf)                 */              
/*====================================================================================================*/

#include "Main_Acquisition_Opsens_WLX2.h"
#ifndef _MAIN_
int main_()
{
	int i,j,k;
	int *select_ch;
	int (*GFx_jauge_ch)[4]={0};
	float *ch_zero,*ch_offset;
	float *ch_value;

	char nom_projet[_STR_LONG]={'\0'},chemin[_STR_LONG]={'\0'},nomfic[_STR_LONG]={'\0'},rep_usb[_STR_LONG]={'\0'};

	char *numero_jauge_ch[NB_CH];
	char *type_jauge_ch[NB_CH];

	struct parametres_connexion param_connection;
	struct config_save_file pconfig_save_file;
	struct config_meas pconfig_meas;
	struct config_all pconfig_all;
	struct shared pshared;
	pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;//=PTHREAD_MUTEX_INITIALIZER;
	struct param_pgm pparam_pgm;


	printf("\n%s\n","*********************************************");
	printf("%s\n","*** Pilotage de WLX2 via le protocole UDP ***");
	printf("%s\n\n","*********************************************");


	/*----------------------------------------------------------------  */
	printf("%s\n","o Configuration du programme d'acquisition: ...");

	Kill_old_process_Pgm_Opsens("Programme_Acquisition_Opsens_WLX2");
	Init_struct_config_save_file(&pconfig_save_file, nom_projet, nomfic, rep_usb);
	Init_struct_config_meas(&pconfig_meas, select_ch, GFx_jauge_ch, numero_jauge_ch, type_jauge_ch);
	Init_struct_config_all(&pconfig_all,&pconfig_save_file,&pconfig_meas);
	printf("%s \n\n","... ok: configuration du programme d'acquisition terminé");

	/*----------------------------------------------------------------  */
	printf("%s\n","o Chargement du fichier de configuration : ...");

	if (Load_config_file(&pconfig_all)==0)
	{
		printf("%s\n","... Echec du chargement du fichier de configuration");
		goto fin_main;
	}
	printf("%s \n\n","... ok: chargement terminé");
	//Print_struct_config(&pconfig);


	/*----------------------------------------------------------------  */
	printf("%s\n","o Etablissement et test de la connexion avec WLX2: ...");
	if (Make_and_test_connexion(&param_connection,0)==0)
	{
		printf("%s\n","... Echec de la connexion avec WLX2");
		goto fin_main;
	}
	printf("%s \n\n","... ok: connexion établie");


	/*----------------------------------------------------------------  */
	printf("%s\n","o Configuration du module WLX2 : ...");
	/*if (Configuration_WLX2(&param_connection, &pconfig_all)==0)
	  {
	  printf("%s\n","... Echec de la configuration ");
	  goto fin_main;
	  }
	  printf("%s \n\n","... ok: configuration réussie");
	  */


	/*----------------------------------------------------------------  */
	printf("%s\n","o Préparation de l'acquisition des données: ...");

	ch_zero=malloc(NB_CH*sizeof(float));
	ch_offset=malloc(NB_CH*sizeof(float));
	ch_value=malloc(NB_CH*sizeof(float));

	if (Init_struct_shared(&pshared,chemin, ch_zero, ch_offset, ch_value, &mutex)==0)
	{
		printf("%s\n","... Echec de la préparation");
		goto fin_main;
	}
	sleep(1);

	if (Preparation_acquisition_WLX2(&pparam_pgm, &param_connection, &pconfig_all, &pshared)==0)
	{
		printf("%s\n","... Echec de la préparation");
		goto fin_main;
	}
	printf("%s \n\n","... ok: l'acquisition des données peut être lancée");
	sleep(1);


	/*----------------------------------------------------------------  */
	printf("%s\n","o Lancement de l'acquisition des données: ...");
//@TODO ; cette condition n'est pas possible, lancement renvoie -1,1 ou une ERROR
	if (Lancement_thread_acquistion(&pparam_pgm)==0)
	{
		printf("%s\n","... Echec de l'acquisition des données");
		goto fin_main;
	}
	printf("%s \n\n","... ok: fin de l'acquisition ");

	pthread_mutex_destroy(&mutex);
	pthread_exit(NULL);


	/*----------------------------------------------------------------  */


fin_main:;


	 printf("%s\n","o Fermeture de la connexion avec WLX2: ...");
	 //Reset_connexion(&param_connection);

	 //printf("%s\n","Bye Bye");
}
#endif
void * acq_WLX2(void * args)
{
	uint8_t *alive = args;
	*alive = 1;

	int *select_ch;
	int (*GFx_jauge_ch)[4]={0};
	float *ch_zero,*ch_offset;
	float *ch_value;

	char nom_projet[_STR_LONG]={'\0'};
	char chemin[_STR_LONG]={'\0'};
	char nomfic[_STR_LONG]={'\0'};
	char rep_usb[_STR_LONG]={'\0'};

	char *numero_jauge_ch[NB_CH];
	char *type_jauge_ch[NB_CH];

	struct parametres_connexion param_connection;
	struct config_save_file pconfig_save_file;
	struct config_meas pconfig_meas;
	struct config_all pconfig_all;
	struct shared pshared;
	pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
	struct param_pgm pparam_pgm;


	//Kill_old_process_Pgm_Opsens("Programme_Acquisition_Opsens_WLX2");
	Init_struct_config_save_file(&pconfig_save_file, nom_projet, nomfic, 
					rep_usb);

	Init_struct_config_meas(&pconfig_meas, select_ch, GFx_jauge_ch, 
					numero_jauge_ch, type_jauge_ch);

	Init_struct_config_all(&pconfig_all,&pconfig_save_file,&pconfig_meas);


	ch_zero=malloc(NB_CH*sizeof(float));
	ch_offset=malloc(NB_CH*sizeof(float));
	ch_value=malloc(NB_CH*sizeof(float));

	// Run each function one after another
	// Exit if one returns 0
	if (!Load_config_file(&pconfig_all, &param_connection.module_idn)
			|| !Make_and_test_connexion(&param_connection,0)
			|| !Init_struct_shared(&pshared,chemin, ch_zero, 
				ch_offset, ch_value, &mutex)
			|| !Preparation_acquisition_WLX2(&pparam_pgm, 
			      &param_connection, &pconfig_all, &pshared)
			|| !Configuration_WLX2(&param_connection, &pconfig_all)
			//|| !Lancement_thread_acquistion(&pparam_pgm)
			)
//	{
//		pthread_exit(0);
//	}
	sleep(10);
	pthread_mutex_destroy(&mutex);
	pthread_exit(NULL);

	pthread_exit((void *) 1);


}
