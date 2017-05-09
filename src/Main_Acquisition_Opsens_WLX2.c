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


static void WLX2_cleanup(void * cleanup_args)
{
	uint8_t i;
	cleanup_struct *args = cleanup_args;
	for (i=0;i<args->nb_of_malloc;i++)
		free(*args->mem_to_free[i]);
	*args->alive = 0;
	pthread_mutex_destroy(args->mutex);
}



#ifndef _MAIN_
int main()
{
	int *select_ch = NULL;
	int (*GFx_jauge_ch)[4]={0};
	float *ch_zero,*ch_offset, *ch_value;

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

	ch_zero=malloc(NB_CH*sizeof(float));
	ch_offset=malloc(NB_CH*sizeof(float));
	ch_value=malloc(NB_CH*sizeof(float));

	// Setup the handler called when acq terminates : mutex, malloc, alive
	cleanup_struct cleanup_args = {
		.mem_to_free[0] = (void *) &ch_zero,
		.mem_to_free[1] = (void *) &ch_offset,
		.mem_to_free[2] = (void *) &ch_value,
		.mem_to_free[3] = (void *) &param_connection.module_idn,
		.mem_to_free[4] = (void *) &pconfig_meas.select_ch,
		.mem_to_free[5] = (void *) &pconfig_meas.GFx_jauge_ch,
		.mem_to_free[6] = (void *) &pconfig_meas.numero_jauge_ch[0],
		.mem_to_free[7] = (void *) &pconfig_meas.type_jauge_ch[0],
		.nb_of_malloc = 8, 
		.mutex = &mutex
	};
	if(NB_CH == 2)
	{
		cleanup_args.mem_to_free[8] = 
			(void *) &pconfig_meas.type_jauge_ch[NB_CH - 1];
		cleanup_args.mem_to_free[9] = 
			(void *) &pconfig_meas.numero_jauge_ch[NB_CH - 1];
		cleanup_args.nb_of_malloc = 10;
	}
	pthread_cleanup_push(WLX2_cleanup, (void*) &cleanup_args);



	printf("\n%s\n","*********************************************");
	printf("%s\n","*** Pilotage de WLX2 via le protocole UDP ***");
	printf("%s\n\n","*********************************************");


	/*----------------------------------------------------------------  */
	printf("%s\n","o Configuration du programme d'acquisition: ...");

	Kill_old_process_Pgm_Opsens("Programme_Acquisition_Opsens_WLX2");
	Init_struct_config_save_file(&pconfig_save_file, nom_projet,
			nomfic, rep_usb);
	Init_struct_config_meas(&pconfig_meas, select_ch, GFx_jauge_ch, 
			numero_jauge_ch, type_jauge_ch);
	Init_struct_config_all(&pconfig_all,&pconfig_save_file,&pconfig_meas);
	printf("%s \n\n","... ok: configuration du programme d'acquisition terminé");

	/*----------------------------------------------------------------  */
	printf("%s\n","o Chargement du fichier de configuration : ...");
	if (Load_config_file(&pconfig_all, &param_connection.module_idn)==0)
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
	printf("%s\n","o Préparation de l'acquisition des données: ...");
	if (Init_struct_shared(&pshared,chemin, ch_zero, ch_offset, 
				ch_value, &mutex)==0)
	{
		printf("%s\n","... Echec de la préparation");
		goto fin_main;
	}
	sleep(1);


	/*----------------------------------------------------------------  */
	printf("%s\n","o Configuration du module WLX2 : ...");
	if (Configuration_WLX2(&param_connection, &pconfig_all)==0)
	{
		printf("%s\n","... Echec de la configuration ");
		goto fin_main;
	}
	printf("%s \n\n","... ok: configuration réussie");


	/*----------------------------------------------------------------  */
	if (Preparation_acquisition_WLX2(&pparam_pgm, &param_connection, 
				&pconfig_all, &pshared)==0)
	{
		printf("%s\n","... Echec de la préparation");
		goto fin_main;
	}
	printf("%s \n\n","... ok: l'acquisition des données peut être lancée");
	sleep(1);


	/*----------------------------------------------------------------  */
	printf("%s\n","o Lancement de l'acquisition des données: ...");
	if (Lancement_thread_acquistion(&pparam_pgm)==0)
	{
		printf("%s\n","... Echec de l'acquisition des données");
		goto fin_main;
	}
	printf("%s \n\n","... ok: fin de l'acquisition ");



	/*----------------------------------------------------------------  */


fin_main:;


	 pthread_cleanup_pop(1);
	 printf("%s\n","o Fermeture de la connexion avec WLX2: ...");
	 //Reset_connexion(&param_connection);

	pthread_exit(NULL);
	 //printf("%s\n","Bye Bye");
}
#endif

void * acq_WLX2(void * args)
{
	uint8_t *alive = args;
	*alive = 1;

	int *select_ch = NULL;
	int (*GFx_jauge_ch)[4]={0};

	float *ch_zero = malloc(NB_CH*sizeof(float));
	float *ch_offset = malloc(NB_CH*sizeof(float));
	float *ch_value = malloc(NB_CH*sizeof(float));

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
	pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;
	struct param_pgm pparam_pgm;

	//Kill_old_process_Pgm_Opsens("Programme_Acquisition_Opsens_WLX2");
	Init_struct_config_save_file(&pconfig_save_file, nom_projet, nomfic, 
			rep_usb);

	Init_struct_config_meas(&pconfig_meas, select_ch, GFx_jauge_ch, 
			numero_jauge_ch, type_jauge_ch);

	Init_struct_config_all(&pconfig_all,&pconfig_save_file,&pconfig_meas);

	// Setup the handler called when acq terminates : mutex, malloc, alive
	cleanup_struct cleanup_args = {
		.mem_to_free[0] = (void *) &ch_zero,
		.mem_to_free[1] = (void *) &ch_offset,
		.mem_to_free[2] = (void *) &ch_value,
		.mem_to_free[3] = (void *) &param_connection.module_idn,
		.mem_to_free[4] = (void *) &pconfig_meas.select_ch,
		.mem_to_free[5] = (void *) &pconfig_meas.GFx_jauge_ch,
		.mem_to_free[6] = (void *) &pconfig_meas.numero_jauge_ch[0],
		.mem_to_free[7] = (void *) &pconfig_meas.type_jauge_ch[0],
		.nb_of_malloc = 8, 
		.alive = alive,
		.mutex = &mut
	};
	if(NB_CH == 2)
	{
		cleanup_args.mem_to_free[8] = 
			(void *) &pconfig_meas.type_jauge_ch[NB_CH - 1];
		cleanup_args.mem_to_free[9] = 
			(void *) &pconfig_meas.numero_jauge_ch[NB_CH - 1];
		cleanup_args.nb_of_malloc = 10;
	}
	pthread_cleanup_push(WLX2_cleanup, (void*) &cleanup_args);


	// Run each function one after another
	// Exit if one returns 0
	if (!Load_config_file(&pconfig_all, &param_connection.module_idn))
		printf("%s\n","... Echec du chargement du fichier de "
				"configuration\n");
	else if(!Make_and_test_connexion(&param_connection,0))
		printf("%s\n","... Echec de la connexion avec WLX2\n");

	else if(!Init_struct_shared(&pshared,chemin, ch_zero, 
				ch_offset, ch_value, &mut))
		printf("%s\n","... Echec de la préparation\n");

	else if(!Configuration_WLX2(&param_connection, &pconfig_all))
		printf("%s\n","... Echec de la configuration\n");

	else if(!Preparation_acquisition_WLX2(&pparam_pgm, 
				&param_connection, &pconfig_all, &pshared))
		printf("%s\n","... Echec de la préparation\n");

	else if(!Lancement_thread_acquistion(&pparam_pgm))
		printf("%s\n","... Echec de l'acquisition des données\n");

	else
	{
		sleep(1);
		pthread_exit((void *) 1);
	}

	printf("Error during WLX2 execution");
	pthread_mutex_destroy(&mut);
	pthread_cleanup_pop(1);
	pthread_exit(0);
}





