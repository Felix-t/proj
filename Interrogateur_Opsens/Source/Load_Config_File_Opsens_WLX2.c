#include "Load_Config_File_Opsens_WLX2.h" 

const int SAMPLINGRATE_VALUES_AUTHORIZED[]={4, 100, 250, 500};
const int FREQ_ECHANTILLONNAGE_VALUES_AUTHORIZED[]={13, 1, 2, 4, 5, 10, 20, 25, 50, 100, 125 , 250, 500};

#include <libconfig.h>
#define NOMFIC_CONFIG_FILE_TEST "Interrogateur_Opsens/conftest"

int Load_config_file(struct config_all *pconfig, char **module_idn)
{
	int i, SAMPLingrate,MEASureRATE;
	const char *tmp_str;

	config_t cfg;
	config_setting_t *root, *setting;
	config_init(&cfg);
	config_set_auto_convert (&cfg, 1);
	config_read_file(&cfg, NOMFIC_CONFIG_FILE_TEST);

	root = config_root_setting(&cfg);

	setting = config_setting_get_member(root, "nom_projet");
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_save_file->nom_projet, tmp_str);	

	setting = config_setting_get_member(root, "nom_fichier_sauvegarde");
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_save_file->nomfic, tmp_str);	

	setting = config_setting_get_member(root, "chemin_cle_usb");
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_save_file->rep_usb, tmp_str);	

	setting = config_setting_get_member(root, "numero_jauge_channel_1");
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_meas->numero_jauge_ch[0], tmp_str);	

	setting = config_setting_get_member(root, "numero_jauge_channel_2");
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_meas->numero_jauge_ch[1], tmp_str);	

	setting = config_setting_get_member(root, "type_jauge_channel_1");
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_meas->type_jauge_ch[0], tmp_str);	

	setting = config_setting_get_member(root, "type_jauge_channel_2");
	tmp_str = config_setting_get_string(setting);
	strcpy(pconfig->pconfig_meas->type_jauge_ch[1], tmp_str);	

	setting = config_setting_get_member(root,"sauvergarde_cle_usb");
	pconfig->pconfig_save_file->usb = config_setting_get_int(setting);

	setting = config_setting_get_member(root, "SMS_alert");
	pconfig->pconfig_meas->sms_alert = config_setting_get_int(setting);

	setting = config_setting_get_member(root, "mode_meas");
	pconfig->pconfig_meas->mode_meas = config_setting_get_int(setting);

	setting = config_setting_get_member(root, "mode_debug");
	pconfig->pconfig_meas->mode_debug= config_setting_get_int(setting);

	setting = config_setting_get_member(root, "channel_1__actif");
	pconfig->pconfig_meas->select_ch[0] = config_setting_get_int(setting);

	setting = config_setting_get_member(root, "channel_2__actif");
	pconfig->pconfig_meas->select_ch[1] = config_setting_get_int(setting);

	setting = config_setting_get_member(root, 
			"taille_max_du_fichier_sauvegarde");
	pconfig->pconfig_save_file->size_max_save_file = 
		config_setting_get_float(setting);

	setting = config_setting_get_member(root, "freq_echantillonnage");
	pconfig->pconfig_meas->Freq_echantillonnage = 
		config_setting_get_int(setting);

	setting = config_setting_get_member(root, "GFx_jauge");
	setting = config_setting_get_member(setting, "channel_1");
	for(i=0;i<4;i++)
	{
		pconfig->pconfig_meas->GFx_jauge_ch[0][i] =
			config_setting_get_int_elem(setting, i);
	}
	setting = config_setting_get_member(root, "GFx_jauge");
	setting = config_setting_get_member(setting, "channel_2");
	for(i=0;i<4;i++)
	{
		pconfig->pconfig_meas->GFx_jauge_ch[1][i] =
			config_setting_get_int_elem(setting, i);
	}
	
	*module_idn = malloc(sizeof(char)*100);
	setting = config_setting_get_member(root, "module_IDN");
	tmp_str = config_setting_get_string(setting);
	strcpy(*module_idn, tmp_str);
	
	setting = config_setting_get_member(root, "set_zero");
	pconfig->pconfig_meas->set_zero = 
		config_setting_get_int(setting);

	if(pconfig->pconfig_meas->set_zero == 1)
		config_setting_set_int(setting, 0);

	printf("Load config file\n"); 
	config_write_file(&cfg, NOMFIC_CONFIG_FILE_TEST);
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

	return 1;
}
/*
/********************************************************/
/*Fonction: Load_config_file                                    */
/*lecture du fichier de configuration (Acquisition_Opsens__paramaters) */
/********************************************************/
/*
int Load_config_file(struct config_all *pconfig, char **module_idn)
{
	int ok=1,ok_fscanf=0, set_zero = 1;
	int i;
	int channel_1__actif,channel_2__actif,mode_meas,mode_debug;
	int channel_1a__data,channel_2a__data,channel_1b__data,channel_2b__data;
	int sms_alert,usb;
	int GF0_jauge_ch_1,GF0_jauge_ch_2,GF1_jauge_ch_1,GF1_jauge_ch_2;
	int GF2_jauge_ch_1,GF2_jauge_ch_2,GF3_jauge_ch_1,GF3_jauge_ch_2;
	int SAMPLingrate,MEASureRATE,Freq_echantillonnage;
	float taille_max_du_fichier_sauvegarde;
	char nomfic[100]=NOMFIC_CONFIG_FILE;
	char *nom_projet, *nom_fichier_sauvegarde, *rep_usb;
	char numero_jauge_ch_1[_STR_SHORT]={'\0'},numero_jauge_ch_2[_STR_SHORT]={'\0'};
	char type_jauge_ch_1[_STR_SHORT]={'\0'},type_jauge_ch_2[_STR_SHORT]={'\0'};
	char *chartmp,rep;
	char c,temp_chemein_usb[500]={'\0'};

	FILE *fp;

	chartmp=malloc(1000*sizeof(char));
	nom_projet=malloc(1000*sizeof(char));
	nom_fichier_sauvegarde=malloc(1000*sizeof(char));
	rep_usb=malloc(1000*sizeof(char));
	*module_idn=malloc(100*sizeof(char));
	nom_projet[0]='\0';
	chartmp[0]='\0';
	nom_fichier_sauvegarde[0]='\0';
	rep_usb[0]='\0';


	if ((fp=fopen(nomfic,"r"))==NULL)
	{
		printf("%s %s\n","Impossible d'ouvrir le fichier de configuration: ",nomfic);
		ok=0;
	}else{

		if(((fscanf(fp,"%s %s",chartmp,nom_projet))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %s",chartmp,nom_fichier_sauvegarde))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %f",chartmp, &taille_max_du_fichier_sauvegarde))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %d",chartmp, &usb))==0)||(ok_fscanf)){ok_fscanf=1;}
		//if(((fscanf(fp,"%s %c",chartmp, rep_usb))==0)||(ok_fscanf)){ok_fscanf=1;}

		fscanf(fp,"%s %c",chartmp, &c);
		i=0;
		while(c!='\n')
		{
			if(c=='\x20'){c='\x20';}
			rep_usb[i]=c;
			i++;
			fscanf(fp,"%c", &c);
		}
		rep_usb[i]='\0';

		if(((fscanf(fp,"%s %d",chartmp, &sms_alert))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %d",chartmp, &mode_meas))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %d",chartmp, &mode_debug))==0)||(ok_fscanf)){ok_fscanf=1;}
		//if(((fscanf(fp,"%s %d",chartmp, &SAMPLingrate))==0)||(ok_fscanf)){ok_fscanf=1;}
		//if(((fscanf(fp,"%s %d",chartmp, &MEASureRATE))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %d",chartmp, &Freq_echantillonnage))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %d",chartmp, &channel_1__actif))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %s",chartmp, numero_jauge_ch_1))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %s",chartmp, type_jauge_ch_1))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %d %d %d %d",chartmp, &GF0_jauge_ch_1, &GF1_jauge_ch_1, &GF2_jauge_ch_1, &GF3_jauge_ch_1))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %d",chartmp, &channel_2__actif))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %s",chartmp, numero_jauge_ch_2))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %s",chartmp, type_jauge_ch_2))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %d %d %d %d",chartmp, &GF0_jauge_ch_2, &GF1_jauge_ch_2, &GF2_jauge_ch_2, &GF3_jauge_ch_2))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %[^\n]",chartmp, *module_idn))==0)||(ok_fscanf)){ok_fscanf=1;}
		if(((fscanf(fp,"%s %i",chartmp, &set_zero))==0)||(ok_fscanf)){ok_fscanf=1;}


	}
	fclose(fp);

	if ((channel_1__actif+channel_2__actif)==0)
	{
		channel_1__actif=1;
		printf("\t%s\n","***** Warning: canal 1 activé *****");
	}

	if(!Find_values_Samplingrate_measurerate(Freq_echantillonnage, 
				&SAMPLingrate, &MEASureRATE)
			|| ok_fscanf
			|| !usb
			|| !Verif_if_dir_exist(rep_usb)
	  )
	{
		ok=0;
		goto fin_Load_config_file;
	}

	if(taille_max_du_fichier_sauvegarde == 0)
		taille_max_du_fichier_sauvegarde=1000;

	memcpy(pconfig->pconfig_save_file->nom_projet,
			nom_projet,
			(strlen(nom_projet)+1)*sizeof(char));

	memcpy(pconfig->pconfig_save_file->nomfic,
			nom_fichier_sauvegarde,
			(strlen(nom_fichier_sauvegarde)+1)*sizeof(char));

	pconfig->pconfig_save_file->size_max_save_file=taille_max_du_fichier_sauvegarde;
	pconfig->pconfig_save_file->usb=usb;

	memcpy(pconfig->pconfig_save_file->rep_usb,
			rep_usb,
			(strlen(rep_usb)+1)*sizeof(char));

	pconfig->pconfig_meas->select_ch[0]=channel_1__actif;
	pconfig->pconfig_meas->select_ch[1]=channel_2__actif;

	pconfig->pconfig_meas->SAMPLingrate=SAMPLingrate;
	pconfig->pconfig_meas->MEASureRATE=MEASureRATE;
	pconfig->pconfig_meas->Freq_echantillonnage=Freq_echantillonnage;

	int size_int = sizeof(int);
	memcpy(&pconfig->pconfig_meas->GFx_jauge_ch[0][0],
			&GF0_jauge_ch_1,size_int);
	memcpy(&pconfig->pconfig_meas->GFx_jauge_ch[0][1],
			&GF1_jauge_ch_1,size_int);
	memcpy(&pconfig->pconfig_meas->GFx_jauge_ch[0][2],
			&GF2_jauge_ch_1,size_int);
	memcpy(&pconfig->pconfig_meas->GFx_jauge_ch[0][3],
			&GF3_jauge_ch_1,size_int);
	memcpy(&pconfig->pconfig_meas->GFx_jauge_ch[1][0],
			&GF0_jauge_ch_2,size_int);
	memcpy(&pconfig->pconfig_meas->GFx_jauge_ch[1][1],
			&GF1_jauge_ch_2,size_int);
	memcpy(&pconfig->pconfig_meas->GFx_jauge_ch[1][2],
			&GF2_jauge_ch_2,size_int);
	memcpy(&pconfig->pconfig_meas->GFx_jauge_ch[1][3],
			&GF3_jauge_ch_2,size_int);



	memcpy(pconfig->pconfig_meas->numero_jauge_ch[0],
			numero_jauge_ch_1,
			(strlen(numero_jauge_ch_1)+1)*sizeof(char));

	memcpy(pconfig->pconfig_meas->numero_jauge_ch[1],
			numero_jauge_ch_2,
			(strlen(numero_jauge_ch_2)+1)*sizeof(char));

	memcpy(pconfig->pconfig_meas->type_jauge_ch[0],
			type_jauge_ch_1,
			(strlen(type_jauge_ch_1)+1)*sizeof(char));

	memcpy(pconfig->pconfig_meas->type_jauge_ch[1],
			type_jauge_ch_2,
			(strlen(type_jauge_ch_2)+1)*sizeof(char));

	pconfig->pconfig_meas->mode_meas=mode_meas;
	pconfig->pconfig_meas->mode_debug=mode_debug;
	pconfig->pconfig_meas->sms_alert=sms_alert;
	pconfig->pconfig_meas->set_zero=set_zero;

fin_Load_config_file:

	free (chartmp);chartmp=NULL;
	free (nom_projet);nom_projet=NULL;
	free (nom_fichier_sauvegarde);nom_fichier_sauvegarde=NULL;
	free (rep_usb);rep_usb=NULL;

	//Print_struct_config_meas(pconfig->pconfig_meas);

	return ok;
}

*/


/********************************************************/
/*Fonction: Find_values_Samplingrate_measurerate               */
/*                                                      */
/********************************************************/
int Find_values_Samplingrate_measurerate(int Freq_echantillonnage, int *S, int *M)
{
	int i,j,ok=1;
	int S_temp,k_temp,k;
	float M_temp;
	int modulo;

	i=SAMPLINGRATE_VALUES_AUTHORIZED[0]-1;
	k_temp=SAMPLINGRATE_VALUES_AUTHORIZED[i];

	if (find_value_in_int_array(Freq_echantillonnage,FREQ_ECHANTILLONNAGE_VALUES_AUTHORIZED)==-1)
	{
		printf("\t%s\n","***** Erreur: la valeur de Freq_echantillonnage est invalide *****");
		ok=0;
		goto fin_Find_values_Samplingrate_measurerate;
	}

	//Take the smallest SAMPLINGRATE_VALUES_AUTHORIZED multiple of Freq_echantillonnage
	for (i=SAMPLINGRATE_VALUES_AUTHORIZED[0];i>1;i--)
	{
		j=i-1;
		modulo=SAMPLINGRATE_VALUES_AUTHORIZED[j]%Freq_echantillonnage;
		//printf("%d %d\n",i,modulo);
		if(modulo==0)
		{
			k=SAMPLINGRATE_VALUES_AUTHORIZED[j]/Freq_echantillonnage;
			if(k<k_temp){k_temp=k; S_temp=SAMPLINGRATE_VALUES_AUTHORIZED[j];}
		}
		//printf("k=%d S=%d\n", k_temp, S_temp);
	}

	//@TODO:easier to understand ?
	/*
	   i = 1;
	   while(!(SAMPLINGRATE_VALUES_AUTHORIZED[i++]%Freq_echantillonnage));
	 *S = SAMPLINGRATE_VALUES_AUTHORIZED[i];
	 */

	*S=S_temp;


	modulo=S_temp%NB_MEASURE_MAX_BY_PAQUET;
	M_temp=S_temp/NB_MEASURE_MAX_BY_PAQUET; // =0
	if(modulo==0)
	{
		*M=(int)(ceil(M_temp));
	}else{
		*M=(int)(ceil(M_temp))+1;
	}
	//printf("F=%d S=%d M=%f\n", Freq_echantillonnage,S_temp, M_temp);

	//printf("F=%d S=%d M=%d\n", Freq_echantillonnage,*S, *M);

	printf("S : %i M : %i\n", *S, *M);


fin_Find_values_Samplingrate_measurerate: ;

					  return ok;
}


