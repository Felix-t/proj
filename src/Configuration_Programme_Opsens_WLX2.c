#include "Configuration_Programme_Opsens_WLX2.h" 


/********************************************************/
/*Fonction: Init_struct_config                          */
/*                                                      */
/********************************************************/
void Init_struct_config_all(struct config_all *pconfig, struct config_save_file *pconfig_save_file, struct config_meas *pconfig_meas)
{
	pconfig->pconfig_save_file=pconfig_save_file;
	pconfig->pconfig_meas=pconfig_meas;
}


/********************************************************/
/*Fonction: Print_struct_config                          */
/*                                                      */
/********************************************************/
void Print_struct_config_all(struct config_all *pconfig)
{
	struct config_save_file *pconfig_save_file;
	struct config_meas *pconfig_meas;

	pconfig_save_file=pconfig->pconfig_save_file;
	Print_Init_struct_config_save_file(pconfig_save_file);

	pconfig_meas=pconfig->pconfig_meas;
	Print_struct_config_meas(pconfig_meas);
}




/********************************************************/
/*Fonction: Init_struct_config_save_file                */
/*                                                      */
/********************************************************/
void Init_struct_config_save_file(struct config_save_file *pconfig_save_file, char *nom_projet,char *nomfic,char *rep_usb)
{
	pconfig_save_file->size_max_save_file=0.0;
	pconfig_save_file->usb=0;

	pconfig_save_file->nom_projet=nom_projet;
	pconfig_save_file->nomfic=nomfic;
	pconfig_save_file->rep_usb=rep_usb;
}


/********************************************************/
/*Fonction: Print_struct_config_save_file                */
/*                                                      */
/********************************************************/
void Print_Init_struct_config_save_file(struct config_save_file *pconfig_save_file)
{
	printf("%s %s\n","nom_projet: ",pconfig_save_file->nom_projet);
	printf("%s %s\n","nomfic: ",pconfig_save_file->nomfic);
	printf("%s %s\n","rep_usb: ",pconfig_save_file->rep_usb);
	printf("%s %f\n","size_max_save_file: ",pconfig_save_file->size_max_save_file);
	printf("%s %d\n","usb: ", pconfig_save_file->usb);
}



/********************************************************/
/*Fonction: Init_struct_config_meas                     */
/*                                                      */
/********************************************************/
void Init_struct_config_meas(struct config_meas *pconfig_meas, int *select_ch, int (*GFx_jauge_ch)[4], char *numero_jauge_ch[NB_CH], char *type_jauge_ch[NB_CH])
{
	int i,j;

	select_ch=malloc(NB_CH*sizeof(int));
	GFx_jauge_ch=malloc(NB_CH*sizeof(*GFx_jauge_ch));

	for(i=0;i<NB_CH;i++)
	{
		numero_jauge_ch[i]=malloc(_STR_SHORT*sizeof(char));
		type_jauge_ch[i]=malloc(_STR_SHORT*sizeof(char));
	}

	pconfig_meas->mode_meas=0;
	pconfig_meas->mode_debug=0;
	pconfig_meas->sms_alert=0;
	pconfig_meas->select_ch=select_ch;
	pconfig_meas->GFx_jauge_ch=GFx_jauge_ch;
	pconfig_meas->SAMPLingrate=0;
	pconfig_meas->MEASureRATE=0;

	for(i=0;i<NB_CH;i++)
	{
		pconfig_meas->select_ch[i]=1;
		pconfig_meas->numero_jauge_ch[i]=numero_jauge_ch[i];
		pconfig_meas->type_jauge_ch[i]=type_jauge_ch[i];
		Zero_str(numero_jauge_ch[i]);
		Zero_str(type_jauge_ch[i]);
		for (j=0;j<4;j++)
		{
			pconfig_meas->GFx_jauge_ch[i][j]=0;

		}
	}
}


/********************************************************/
/*Fonction: Print_struct_config_meas                     */
/*                                                      */
/********************************************************/
void Print_struct_config_meas(struct config_meas *pconfig_meas)
{
	int i,j;

	printf("%s %d\n","SAMPLingrate: ", pconfig_meas->SAMPLingrate);
	printf("%s %d\n","MEASureRATE: ", pconfig_meas->MEASureRATE);
	printf("%s %d\n","Freq_echantillonnage: ", pconfig_meas->Freq_echantillonnage);

	for (i=0;i<NB_CH;i++)
	{
		printf("%s%d %s %d\n","select_ch_",i+1,": ",pconfig_meas->select_ch[i]);
		if (pconfig_meas->select_ch[i])
		{
			printf("%s %s\n","type_jauge: ",pconfig_meas->type_jauge_ch[i]);
			printf("%s %s\n","numero_jauge: ",pconfig_meas->numero_jauge_ch[i]);
			printf("%s \n","GFx_jauge: ");
			for (j=0;j<4;j++)
			{
				printf("%s%d%s%d ","GF",j,"=",pconfig_meas->GFx_jauge_ch[i][j]);
			}
			printf("%s\n","");
		}
	}
	printf("%s %d\n","mode_meas: ", pconfig_meas->mode_meas);
	printf("%s %d\n","mode_debug: ", pconfig_meas->mode_debug);
	printf("%s %d\n","sms_alert: ", pconfig_meas->sms_alert);
}







