#include "Load_Config_File_Opsens_WLX2.h" 
#include <libconfig.h>

const int SAMPLINGRATE_VALUES_AUTHORIZED[]= {4, 100, 250, 500};
const int FREQ_ECHANTILLONNAGE_VALUES_AUTHORIZED[]= 
		{13, 1, 2, 4, 5, 10, 20, 25, 50, 100, 125 , 250, 500};

static uint8_t print_config(struct config_all *cfg);
static int Find_values_Samplingrate_measurerate(int Freq_echantillonnage, 
		int *S, int *M);

uint8_t save_zeros_offset(float zero_1, float zero_2)
{
	config_t cfg;
	config_setting_t *root, *setting;

	config_init(&cfg);
	config_set_auto_convert (&cfg, 1);
	if(config_read_file(&cfg, NOMFIC_CONFIG_FILE) == CONFIG_FALSE)
	{
		printf("Impossible d'ouvrir le fichier de configuration : %s",
				NOMFIC_CONFIG_FILE);
		return 0;
	}

	root = config_root_setting(&cfg);	
	setting = config_setting_get_member(root, "zeros");
	config_setting_set_float_elem(setting, 0, zero_1);
	config_setting_set_float_elem(setting, 1, zero_2);

	config_write_file(&cfg, NOMFIC_CONFIG_FILE);
	config_destroy(&cfg);
	return 1;
}

static uint8_t print_config(struct config_all *cfg)
{
	char answer;

	printf("\t%s\n","----------------------------");
	printf("\t%s %s\n","Nom_projet: ",
			cfg->pconfig_save_file->nom_projet);
	printf("\t%s %s\n","Nom_fichier_sauvegarde: ",
			cfg->pconfig_save_file->nomfic);
	printf("\t%s %f %s\n","Taille_max_du_fichier_sauvegarde: ",
			cfg->pconfig_save_file->size_max_save_file," ko");
	printf("\t%s %d\n","Sauvergarde_cle_usb: ",cfg->pconfig_save_file->usb);
	printf("\t%s %s\n","Chemin_cle_usb: ",cfg->pconfig_save_file->rep_usb);
	printf("\t%s %d\n","SMS_alert: ",cfg->pconfig_meas->sms_alert);
	printf("\t%s %d\n","Mode_meas: ",cfg->pconfig_meas->mode_meas);
	printf("\t%s %d\n","Mode_debug: ",cfg->pconfig_meas->mode_debug); 
	printf("\t%s %d\n","Freq_echantillonnage: ",
			cfg->pconfig_meas->Freq_echantillonnage); 
	printf("\t%s %d\n","Channel_1__actif: ",
			cfg->pconfig_meas->select_ch[0]);
	printf("\t%s %s\n","Numero_jauge_channel_1: ",
			cfg->pconfig_meas->numero_jauge_ch[0]);
	printf("\t%s %s\n","Type_jauge_channel_1: ",
			cfg->pconfig_meas->type_jauge_ch[0]);
	printf("\t%s%d %s%d %s%d %s%d\n","GFx_jauge_channel_1:\
			GF0=", cfg->pconfig_meas->GFx_jauge_ch[0][0],
			" GF1=", cfg->pconfig_meas->GFx_jauge_ch[0][1],
			" GF2=", cfg->pconfig_meas->GFx_jauge_ch[0][2],
			" GF3=", cfg->pconfig_meas->GFx_jauge_ch[0][3]);
	printf("\t%s %d\n","Channel_2__actif: ",
			cfg->pconfig_meas->select_ch[1]);
	printf("\t%s %s\n","Numero_jauge_channel_2: ",
			cfg->pconfig_meas->numero_jauge_ch[1]);
	printf("\t%s %s\n","Type_jauge_channel_2: ",
			cfg->pconfig_meas->type_jauge_ch[1]);
	printf("\t%s%d %s%d %s%d %s%d\n","GFx_jauge_channel_2:\
			GF0=", cfg->pconfig_meas->GFx_jauge_ch[1][0],
			" GF1=", cfg->pconfig_meas->GFx_jauge_ch[1][1],
			" GF2=", cfg->pconfig_meas->GFx_jauge_ch[1][2],
			" GF3=", cfg->pconfig_meas->GFx_jauge_ch[1][3]);

	printf("\n\t%s %d\n","-> SAMPLingrate=",
			cfg->pconfig_meas->SAMPLingrate);
	printf("\t%s %d\n","-> MEASureRATE=",
			cfg->pconfig_meas->MEASureRATE);

	printf("\t%s\n","----------------------------");
	printf("\t%s\n\t","Paramètres corrects? ['o' ou 'n']");

	answer=getchar();
	while ((answer!='n')&&(answer!='o'))
	{
		answer=getchar();
		printf("\t%s\n\t","Paramètres corrects? ['o' ou 'n']");
	}

	if (answer=='n')
	{
		return(0);
	}

	return 1;
}


int Load_config_file(struct config_all *pconfig, char **module_idn)
{
	int i, SAMPLingrate, MEASureRATE;
	const char *tmp_str;

	// Syntax of the configuration file is :
	// 	name_of_config_option = option_value;
	// The libconfig library loads in memory every setting into a
	// config_t object, which we then query for the desired setting.
	// Then we get the value of this setting depending on its type
	config_t cfg;
	config_setting_t *root, *setting;

	config_init(&cfg);
	config_set_auto_convert (&cfg, 1);
	if(config_read_file(&cfg, NOMFIC_CONFIG_FILE) == CONFIG_FALSE)
	{
		printf("Impossible d'ouvrir le fichier de configuration : %s",
				NOMFIC_CONFIG_FILE);
		return 0;
	}

	root = config_root_setting(&cfg);

	if(!(setting = config_setting_get_member(root, "nom_projet")))
		return 0;
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_save_file->nom_projet, tmp_str);	

	if(!(setting=config_setting_get_member(root, "nom_fichier_sauvegarde")))
		return 0;
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_save_file->nomfic, tmp_str);	

	if(!(setting = config_setting_get_member(root, "chemin_cle_usb")))
		return 0;
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_save_file->rep_usb, tmp_str);	

	if(!(setting=config_setting_get_member(root, "numero_jauge_channel_1")))
		return 0;
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_meas->numero_jauge_ch[0], tmp_str);	

	if(!(setting=config_setting_get_member(root, "numero_jauge_channel_2")))
		return 0;
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_meas->numero_jauge_ch[1], tmp_str);	

	if(!(setting = config_setting_get_member(root, "type_jauge_channel_1")))
		return 0;
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_meas->type_jauge_ch[0], tmp_str);	

	if(!(setting = config_setting_get_member(root, "type_jauge_channel_2")))
		return 0;
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_meas->type_jauge_ch[1], tmp_str);	

	if(!(setting = config_setting_get_member(root,"sauvergarde_cle_usb")))
		return 0;
	pconfig->pconfig_save_file->usb = config_setting_get_int(setting);

	if(!(setting = config_setting_get_member(root, "SMS_alert")))
		return 0;
	pconfig->pconfig_meas->sms_alert = config_setting_get_int(setting);

	if(!(setting = config_setting_get_member(root, "mode_meas")))
		return 0;
	pconfig->pconfig_meas->mode_meas = config_setting_get_int(setting);

	if(!(setting = config_setting_get_member(root, "mode_debug")))
		return 0;
	pconfig->pconfig_meas->mode_debug= config_setting_get_int(setting);

	if(!(setting = config_setting_get_member(root, "channel_1__actif")))
		return 0;
	pconfig->pconfig_meas->select_ch[0] = config_setting_get_int(setting);

	if(!(setting = config_setting_get_member(root, "channel_2__actif")))
		return 0;
	pconfig->pconfig_meas->select_ch[1] = config_setting_get_int(setting);

	if(!(setting = config_setting_get_member(root,
			"taille_max_du_fichier_sauvegarde")))
		return 0;
	pconfig->pconfig_save_file->size_max_save_file = 
		config_setting_get_float(setting);

	if(!(setting = config_setting_get_member(root, "freq_echantillonnage")))
		return 0;
	pconfig->pconfig_meas->Freq_echantillonnage = 
		config_setting_get_int(setting);

	if(!(setting = config_setting_get_member(root, "GFx_jauge"))
		|| !(setting = config_setting_get_member(setting, "channel_1")))
		return 0;
	for(i=0;i<4;i++)
	{
		pconfig->pconfig_meas->GFx_jauge_ch[0][i] =
			config_setting_get_int_elem(setting, i);
	}
	if(!(setting = config_setting_get_member(root, "GFx_jauge"))
		|| !(setting = config_setting_get_member(setting, "channel_2")))
		return 0;
	for(i=0;i<4;i++)
	{
		pconfig->pconfig_meas->GFx_jauge_ch[1][i] =
			config_setting_get_int_elem(setting, i);
	}
	
	// Since module_idn is a part of the struct param_connection, its 
	// corresponding init_function has not yet been called when the config 
	// file is loaded : allocate mem here
	if(!(setting = config_setting_get_member(root, "module_IDN")))
		return 0;
	tmp_str = config_setting_get_string(setting);
	*module_idn = malloc(100);
	strcpy(*module_idn, tmp_str);
	
	if(!(setting = config_setting_get_member(root, "zeros")))
		return 0;
	pconfig->pconfig_meas->zeros[0] = 
		config_setting_get_int_elem(setting, 0);
	pconfig->pconfig_meas->zeros[1] = 
		config_setting_get_int_elem(setting, 1);

	//config_write_file(&cfg, NOMFIC_CONFIG_FILE);
	config_destroy(&cfg);

	if (!pconfig->pconfig_meas->select_ch[0] 
			&& !pconfig->pconfig_meas->select_ch[0] )
	{
		pconfig->pconfig_meas->select_ch[0] = 1;
		printf("\t%s\n","***** Warning: canal 1 activé *****");
	}

	if(!Find_values_Samplingrate_measurerate(
				pconfig->pconfig_meas->Freq_echantillonnage,
				&SAMPLingrate, &MEASureRATE)
			|| !pconfig->pconfig_meas->Freq_echantillonnage
			|| !Verif_if_dir_exist(
				pconfig->pconfig_save_file->rep_usb) )
	{
		return 0;
	}
	else
	{
		pconfig->pconfig_meas->SAMPLingrate=SAMPLingrate;
		pconfig->pconfig_meas->MEASureRATE=MEASureRATE;
	}

	if(pconfig->pconfig_save_file->size_max_save_file == 0)
		pconfig->pconfig_save_file->size_max_save_file = 1000;

#ifndef _MAIN_
	return print_config(pconfig);
#endif

	return 1;
}


/********************************************************/
/*Fonction: Find_values_Samplingrate_measurerate               */
/*                                                      */
/********************************************************/
static int Find_values_Samplingrate_measurerate(int Freq_echantillonnage, int *S, int *M)
{
	int i = 0;

	//Find_value_in_int_array takes an array with array[0] = len(array)
	if (find_value_in_int_array(Freq_echantillonnage,
				FREQ_ECHANTILLONNAGE_VALUES_AUTHORIZED)==-1)
	{
		printf("***** Erreur: la valeur de "
				"Freq_echantillonnage est invalide *****");
		return 0;	
	}

	// Loop stops when smallest multiple of Freq_echantillonnage is reached
	while (++i<SAMPLINGRATE_VALUES_AUTHORIZED[0]
		&& SAMPLINGRATE_VALUES_AUTHORIZED[i]%Freq_echantillonnage);

	*S = SAMPLINGRATE_VALUES_AUTHORIZED[i];
	
	if(!(*S%NB_MEASURE_MAX_BY_PAQUET))
		*M = *S/NB_MEASURE_MAX_BY_PAQUET;
	else
		*M = *S/NB_MEASURE_MAX_BY_PAQUET + 1;

	return 1;
}


