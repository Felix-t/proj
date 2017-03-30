#include "Acquisition_Opsens_WLX2.h" 


/********************************************************/
/*Fonction: Open_file_Enregistrement_data               */
/*                                                      */ 
/********************************************************/
int Open_file_Enregistrement_data(struct param_pgm *param)
{
	char *nomfic_save,*nomfile_data,*rep_data; 
	char suffixe_nb[20]="",suffixe1[10]="__",suffixe2[10]="/";
	int nomfic_len,nb_file_save;
	int nb,ok=1;

	FILE *fp=NULL;

	pthread_mutex_lock(param->pshared->mutex);
	nomfile_data=param->pconfig_all->pconfig_save_file->nomfic;//printf("%s\n",nomfile_data);
	nb_file_save=param->pshared->nb_save_file;
	rep_data=param->pshared->chemin; //printf("----%s\n",rep_data);
	pthread_mutex_unlock(param->pshared->mutex);

	nb=nb_file_save+1;
	sprintf(suffixe_nb,"%d",nb);
	//printf("iiii%s\n",suffixe_nb);
	//getchar();
	nomfic_len=strlen(rep_data)+strlen(suffixe2)+strlen(nomfile_data)+strlen(suffixe1)+strlen(suffixe_nb)+1;

	nomfic_save=malloc(nomfic_len*sizeof(char));
	nomfic_save[0]='\0';

	strcpy(nomfic_save,rep_data);
	strcat(nomfic_save,suffixe2);
	strcat(nomfic_save,nomfile_data);
	strcat(nomfic_save,suffixe1);
	strcat(nomfic_save,suffixe_nb);


	if ((fp=fopen(nomfic_save,"w")) == NULL){
		printf("%s %s\n","Impossible d'ouvrir le fichier: ", nomfic_save);
		ok=0;
	}else{
		printf("%s %s\n","Fichier en cours d'écriture :", nomfic_save);

		pthread_mutex_lock(param->pshared->mutex);
		param->pshared->fp=fp;
		pthread_mutex_unlock(param->pshared->mutex);

		Print_Entete_file_Enregistrement_data(param);
	}



	return ok;
}




/********************************************************/
/*Fonction: Close_file_Enregistrement_data              */      
/*                                                      */
/********************************************************/
int Close_file_Enregistrement_data(struct param_pgm *param)
{ 
	int ok=1;
	FILE *fp;

	fp=param->pshared->fp;

	if (fclose(fp)==EOF){ok=0;} 
	//printf("%d\n",sf->size);

	return ok;
}




/********************************************************/
/*Fonction: Print_Entete_file_Enregistrement_data       */      
/*                                                      */
/********************************************************/
void Print_Entete_file_Enregistrement_data(struct param_pgm *param )
{

	int i;
	float zero_channel,offset_channel;
	char GF0[10],GF1[10],GF2[10],GF3[10];
	char temp_str[100]={'\0'};
	char GF[100], type_jauge[100]={'\0'},numero_jauge[100]={'\0'};
	char current_time_WLX2[100]={'\0'};
	FILE *fp;

	fp=param->pshared->fp;



	//nb=param->pconfig_all->pmeas->select_ch[0]param->pmeas->ch_2_actif;

	fprintf(fp, "#%s %s\n","Projet: ", param->pconfig_all->pconfig_save_file->nom_projet);
	fprintf(fp,"#%s\n"," ");
	/*
	   Zero_str(current_time_WLX2);
	   Get_date_time_from_WLX2(param->pparam_connection, current_time_WLX2);
	   fprintf(fp, "#%s\n\n", current_time_WLX2);
	   fprintf(fp,"#%s\n"," ");
	   */



	fprintf(fp,"# MEASureRATE: %d\n",param->pconfig_all->pconfig_meas->MEASureRATE);
	fprintf(fp,"# SAMPLingrate: %d\n",param->pconfig_all->pconfig_meas->SAMPLingrate);
	fprintf(fp,"# Freq_echantillonnage: %d\n",param->pconfig_all->pconfig_meas->Freq_echantillonnage);
	fprintf(fp,"#%s\n"," ");


	for(i=0;i<NB_CH;i++)
	{
		Zero_str(GF);Zero_str(GF0);Zero_str(GF1);Zero_str(GF2);Zero_str(GF3);
		Zero_str(type_jauge);Zero_str(numero_jauge);

		if(param->pconfig_all->pconfig_meas->select_ch[i])
		{
			memcpy(type_jauge,param->pconfig_all->pconfig_meas->type_jauge_ch[i],strlen(param->pconfig_all->pconfig_meas->type_jauge_ch[i])*sizeof(char));
			memcpy(numero_jauge,param->pconfig_all->pconfig_meas->numero_jauge_ch[i],strlen(param->pconfig_all->pconfig_meas->numero_jauge_ch[i])*sizeof(char));

			sprintf(GF0, "%d", param->pconfig_all->pconfig_meas->GFx_jauge_ch[i][0]);
			sprintf(GF1, "%d", param->pconfig_all->pconfig_meas->GFx_jauge_ch[i][1]);
			sprintf(GF2, "%d", param->pconfig_all->pconfig_meas->GFx_jauge_ch[i][2]);
			sprintf(GF3, "%d", param->pconfig_all->pconfig_meas->GFx_jauge_ch[i][3]);
			strcat(GF,"GF0=");strcat(GF,GF0);strcat(GF," ");
			strcat(GF,"GF1=");strcat(GF,GF1);strcat(GF," ");
			strcat(GF,"GF2=");strcat(GF,GF2);strcat(GF," ");
			strcat(GF,"GF3=");strcat(GF,GF3);

			fprintf(fp,"# %s\t%d\n","Capteur du canal ",i+1);
			fprintf(fp,"#\t->%s %s\n","Type jauge: ",type_jauge);
			fprintf(fp,"#\t->%s %s\n","Numero jauge: ",numero_jauge);
			fprintf(fp,"#\t->%s %s\n","Paramètres: ",GF);


			fprintf(fp,"#\t ZERO: %f\t",param->pshared->ch_zero[i]);
			fprintf(fp,"OFFSET: %f\n",param->pshared->ch_offset[i]);
			fprintf(fp,"#%s\n"," ");

		}
	}

	fprintf(fp,"#%s\t","Date et heure");
	for(i=0;i<NB_CH;i++)
	{
		fprintf(fp,"%s %d\t","Canal ",i+1);
	}
	fprintf(fp,"%s\n","");

	fflush(stdout);
}



/********************************************************/
/*Fonction: Measure_start_infinite                      */
/*                                                      */
/********************************************************/
int Measure_start_infinite (struct parametres_connexion *param_connection)
{
	int ok,i,nb_answer=NB_CH;
	char command1[100]="MEASure:STARt INFInite";
	char command2[100]="MEASure:RUN?";
	char reponse_ok[3]="ok";
	char answer1[100]={'\0'},answer2[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={'\0','\0'};


	Zero_str(answer1);Zero_str(answer2);
	//printf("%s %s\n","Command to the instrument: ", command);
	//ok=Send_command_and_receive_answer(param_connection,command1, answer1, 0);
	ok=Send_command_and_receive_answer_2(param_connection,command1, nb_answer,answer_2, 0);

	//printf("%s %s %s\n","Response of the instrument: ", answer_2[0],answer_2[1]);

	if(ok)
	{
		ok=Compare_2str(reponse_ok, answer_2[0]);
	}


	Zero_str(answer_2[0]);Zero_str(answer_2[1]);

	if(ok)
	{
		//ok=Send_command_and_receive_answer(param_connection,command2, answer2, 0);
		ok=Send_command_and_receive_answer_2(param_connection,command2, nb_answer,answer_2, 0);
	}

	//printf("%s %s %s\n","Response of the instrument: ", answer_2[0],answer_2[1]);

	if(ok)
	{
		if (answer_2[0]==0){ok=0;}
		//ok=Compare_2str("-1", answer_2[0]);
	}

	//printf("OK MEASure:STARt INFInite %d\n", ok);

	return ok;
}




/********************************************************/
/*Fonction: N_Measure_start                      */
/*                                                      */
/********************************************************/
int N_Measure_start (struct parametres_connexion *param_connection, int nb_meas_to_do)
{
	int ok,i,nb_answer=NB_CH;
	char prefix_command1[100]="MEASure:STARt ";
	char command1[100]={'\0'};
	char temp_str[100]={'\0'};
	char reponse_ok[3]="ok";
	char answer1[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={'\0','\0'};

	Zero_str(answer1);Zero_str(command1);
	strcat(command1,prefix_command1);
	sprintf(temp_str, "%d", nb_meas_to_do);
	strcat(command1,temp_str); 


	//printf("%s %s\n","Command to the instrument: ", command);
	//ok=Send_command_and_receive_answer(param_connection,command1, answer1, 0);
	ok=Send_command_and_receive_answer_2(param_connection,command1, nb_answer,answer_2, 0);

	//printf("%s %s\n","Response of the instrument: ", answer1);
	if(ok)
	{
		ok=Compare_2str(reponse_ok, answer_2[0]);
	}


	return ok;
}


pthread_cond_t condition_single_measurement=PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_single_measurement=PTHREAD_MUTEX_INITIALIZER;
void* thread_start_single_measurement (void* arg);
void* thread_receive_single_measurement (void* arg);


/********************************************************/
/*Fonction: Get_single_measurement                      */
/*                                                      */
/********************************************************/
int Get_single_measurement (struct param_pgm *p_data)
{
	int ok;

	pthread_t xthread_start_single_measurement;
	pthread_t xthread_receive_single_measurement;


	ok=pthread_create (&xthread_start_single_measurement, NULL, thread_start_single_measurement, p_data); 
	if (ok==0)
	{
		ok=pthread_create (&xthread_receive_single_measurement, NULL, thread_receive_single_measurement, p_data); 
	}else{ok=1;}

	if (ok==0)
	{
		ok=pthread_join(xthread_start_single_measurement,NULL);
	}else{ok=1;}

	if (ok==0)
	{
		ok=pthread_join(xthread_receive_single_measurement,NULL);
	}else{ok=1;}

	if (ok==1){ok=-1;}
	if (ok==0){ok=1;}
	return ok;
}


/********************************************************/
/*Fonction: thread_start_single_measurement             */
/*                                                      */
/********************************************************/
void* thread_start_single_measurement (void* arg)
{
	struct param_pgm  *p_data=arg;
	int ok;

	//sleep(1);
	//printf("%s\n","thread_start_single_measurement");

	pthread_mutex_lock(&mutex_single_measurement);
	//printf("%s\n","wait_start_single_measurement");
	pthread_cond_wait(&condition_single_measurement,&mutex_single_measurement);
	//printf("%s\n","waitgo_start_single_measurement");

	ok=N_Measure_start(p_data->pparam_connection,1);
	//printf("%s %d\n","ok thread_start_single_measurement", ok);

	pthread_mutex_unlock(&mutex_single_measurement);

	//printf("%s\n","fin thread_start_single_measurement");
	pthread_exit(NULL);

}



/********************************************************/
/*Fonction: thread_receive_single_measurement           */
/*                                                      */
/********************************************************/
void* thread_receive_single_measurement (void* arg)
{
	struct param_pgm  *p_data=arg;
	int i,j,k,nb_channel_actif,nb_answer=NB_CH;
	int ok,ok_reception;
	uint8_t val_ch[2]={1,2};

	struct stUDPSendMeasureType_t SendMeasureType_t_ch[NB_CH];

	//printf("%s\n","thread_receive_single_measurement");

	nb_channel_actif=0;
	for(i=0;i<NB_CH;i++)
	{ 
		nb_channel_actif=nb_channel_actif+p_data->pconfig_all->pconfig_meas->select_ch[i];
	}
	//sleep(1);
	//printf("nb_channel_actif %d\n",nb_channel_actif);


	for(i=0;i<nb_channel_actif;i++)
	{
		if(i==0)
		{
			pthread_mutex_lock(&mutex_single_measurement);
			//printf("%s\n","envoi_condition");
			pthread_cond_signal(&condition_single_measurement);
			//printf("%s\n","envoi_condition_ok"); 
			pthread_mutex_unlock(&mutex_single_measurement);
		}

		Zero_stUDPSendMeasureType_t (&SendMeasureType_t_ch[i]); 
		if(Reception_data(p_data->pparam_connection,&SendMeasureType_t_ch[i],0)==0)
		{   
			printf("\n%s\n","Erreur in reception data");goto thread_receive_single_measurement;
		} 

		//Print_stUDPSendMeasureType_t(&SendMeasureType_t_ch[i]);   

		//printf("ok_reception %d\n",ok_reception);
	}


	for(i=0;i<nb_channel_actif;i++)
	{
		p_data->pshared->ch_value[i]=-99999.99;
	}
	for(i=0;i<nb_channel_actif;i++)
	{
		j=SendMeasureType_t_ch[i].stHeader.stMeasureHeader.ui8ChannelID-1;
		p_data->pshared->ch_value[j]=SendMeasureType_t_ch[i].fMeasure[0];
		// printf("%d %d %f\n",i,j,SendMeasureType_t_ch[i].fMeasure[0]);                   
	}
	//printf("xxxx %f %f\n",p_data->pshared->ch_value[0],p_data->pshared->ch_value[1]);


	//printf("%s\n","fin thread_receive_single_measurement");

thread_receive_single_measurement: ;

				   pthread_exit(NULL);

}




/********************************************************/
/*Fonction: Measure_stop                                */
/*                                                      */
/********************************************************/
int Measure_stop (struct parametres_connexion *param_connection)
{
	int ok,i,nb_answer=NB_CH;
	char command1[100]="MEASure:STOP";
	char command2[100]="MEASure:RUN?";
	char reponse_ok[3]="ok";
	char answer1[100]={'\0'},answer2[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={'\0','\0'};


	Zero_str(answer1);Zero_str(answer2);
	//printf("%s %s\n","Command to the instrument: ", command);
	//ok=Send_command_and_receive_answer(param_connection,command1, answer1, 0);
	ok=Send_command_and_receive_answer_2(param_connection,command1, nb_answer,answer_2, 0);

	//printf("%s %s\n","Response of the instrument: ", answer1);
	if(ok)
	{
		ok=Compare_2str(reponse_ok, answer_2[0]);
	}

	Zero_str(answer_2[0]);Zero_str(answer_2[1]);

	if(ok)
	{
		//ok=Send_command_and_receive_answer(param_connection,command2, answer2, 0);
		ok=Send_command_and_receive_answer_2(param_connection,command2, nb_answer,answer_2, 0);
	}

	if(ok)
	{
		ok=Compare_2str("0", answer_2[0]);
	}

	return ok;
}




/********************************************************/
/*Fonction: Zero_sensor                                */
/*                                                      */
/********************************************************/
int Zero_sensor(struct parametres_connexion *param_connection, int ch1_zero, int ch2_zero)
{
	int ok,i,nb_answer=NB_CH;
	char command1a[10]="SENSor",command1b[10]=":ZERO";
	char command1[100]={'\0'};
	char reponse_ok[3]="ok";
	char answer1[100]={'\0'},answer2[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={'\0','\0'};


	if (ch1_zero)
	{
		strcat(command1,command1a);
		strcat(command1,"1");
		strcat(command1,command1b);

		Zero_str(answer1);Zero_str(answer2);
		//printf("%s %s\n","Command to the instrument: ", command1);
		//ok=Send_command_and_receive_answer(param_connection,command1, answer1, 0);
		ok=Send_command_and_receive_answer_2(param_connection,command1, nb_answer,answer_2, 0);
		//printf("*** %s %s %s\n",command1,answer_2[0],answer_2[1]);
		if(ok)
		{
			ok=Compare_2str(reponse_ok, answer_2[0]);
		}else{goto fin_zero_sensor; }
	}




	if (ch2_zero)
	{
		Zero_str(command1);
		strcat(command1,command1a);
		strcat(command1,"2");
		strcat(command1,command1b);

		Zero_str(answer1);Zero_str(answer2);
		//printf("%s %s\n","Command to the instrument: ", command);
		//ok=Send_command_and_receive_answer(param_connection,command1, answer1, 0);
		ok=Send_command_and_receive_answer_2(param_connection,command1, nb_answer,answer_2, 0);
		//printf("***%s %s %s\n",command1,answer_2[0],answer_2[1]);
		if(ok)
		{
			ok=Compare_2str(reponse_ok, answer_2[0]);
		}else{goto fin_zero_sensor; }
	}


fin_zero_sensor: ;


		 return ok;
}





/********************************************************/
/*Fonction: Get_zero_sensor                             */
/*                                                      */
/********************************************************/
int Get_zero_sensor(struct parametres_connexion *param_connection, int ch_zero, float *zero_channel)
{
	int ok,i,nb_answer=NB_CH;
	char command1a[10]="SENSor",command1b[10]=":ZERO?";
	char command1[100]={'\0'};
	char temp_str[100]={'\0'};
	char answer1[100]={'\0'},answer2[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={'\0','\0'};


	sprintf(temp_str, "%d",ch_zero);

	strcat(command1,command1a);
	strcat(command1,temp_str);

	strcat(command1,command1b);

	Zero_str(answer1);Zero_str(answer2);
	//printf("%s %s\n","Command to the instrument: ", command1);
	//ok=Send_command_and_receive_answer(param_connection,command1, answer1, 0);
	ok=Send_command_and_receive_answer_2(param_connection,command1, nb_answer,answer_2, 0);
	//printf("*** %s %s %s\n",command1,answer_2[0],answer_2[1]);

	if (ch_zero==1){sscanf(answer_2[0],"%f",zero_channel);}
	if (ch_zero==2){sscanf(answer_2[1],"%f",zero_channel);}


fin_get_zero_sensor: ;


		     return ok;
}




/********************************************************/
/*Fonction: Get_offset_sensor                             */
/*                                                      */
/********************************************************/
int Get_offset_sensor(struct parametres_connexion *param_connection, int ch_offset, float *offset_channel)
{
	int ok,i,nb_answer=NB_CH;
	char command1a[10]="SENSor",command1b[10]=":OFFSet?";
	char command1[100]={'\0'};
	char temp_str[100]={'\0'};
	char answer1[100]={'\0'},answer2[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={'\0','\0'};

	sprintf(temp_str, "%d",ch_offset);

	strcat(command1,command1a);
	strcat(command1,temp_str);

	strcat(command1,command1b);

	Zero_str(answer1);Zero_str(answer2);
	//printf("%s %s\n","Command to the instrument: ", command1);
	//ok=Send_command_and_receive_answer(param_connection,command1, answer1, 0);
	ok=Send_command_and_receive_answer_2(param_connection,command1, nb_answer,answer_2, 0);
	//printf("*** %s %s %s\n",command1,answer_2[0],answer_2[1]);

	if (ch_offset==1){sscanf(answer_2[0],"%f",offset_channel);}
	if (ch_offset==2){sscanf(answer_2[1],"%f",offset_channel);}

	//printf("%d %f\n",ch_offset,*offset_channel);
fin_get_offset_sensor: ;


		       return ok;
}



/********************************************************/
/*Fonction: Set_offset_sensor                             */
/*                                                      */
/********************************************************/
int Set_offset_sensor(struct parametres_connexion *param_connection, int ch_offset, float offset_channel)
{
	int ok,i,nb_answer=NB_CH;
	char command1a[10]="SENSor",command1b[10]=":OFFSet";
	char command1[100]={'\0'};
	char temp_str[100]={'\0'};
	char answer1[100]={'\0'},answer2[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={'\0','\0'};
	char reponse_ok[3]="ok";



	sprintf(temp_str, "%d",ch_offset);
	strcat(command1,command1a);
	strcat(command1,temp_str);
	strcat(command1,command1b);
	strcat(command1," ");

	sprintf(temp_str, "%f",offset_channel);
	strcat(command1,temp_str);

	Zero_str(answer1);Zero_str(answer2);
	//printf("%s %s\n","Command to the instrument: ", command1);
	//ok=Send_command_and_receive_answer(param_connection,command1, answer1, 0);
	ok=Send_command_and_receive_answer_2(param_connection,command1, nb_answer,answer_2, 0);
	//printf("*** %s %s %s\n",command1,answer_2[0],answer_2[1]);

	if(ok)
	{
		if (ch_offset==1){strcat(answer1,answer_2[0]);}
		if (ch_offset==2){strcat(answer1,answer_2[1]);}

		if(Compare_2str(answer_2[0], answer1)==0){ok=0;goto fin_set_offset_sensor;}

	}
	//printf("%d %f\n",ch_offset,*offset_channel);


fin_set_offset_sensor: ;

		       //printf("OK %d\n",ok);
		       //printf("%s %s\n",reponse_ok, answer1);

		       return ok;
}



/********************************************************/
/*Fonction: Run_Thread_Enregistrement_data              */
/*                                                      */
/********************************************************/
int Run_Thread_Enregistrement_data(struct param_pgm *param)
{
	int ok;

	pthread_t xthread_Enregistrement_data;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	ok=pthread_create (&xthread_Enregistrement_data, &attr, thread_Enregistrement_data, param); 
	/*
	   if (ok==0)
	   {
	   ok=pthread_join(xthread_Enregistrement_data,NULL);
	   }else{ok=1;}*/

	if (ok==0){ok=1;param->pshared->thread_enregistrement=xthread_Enregistrement_data;}else{ok=0;}

	return ok;

}


/********************************************************/
/*Fonction: Stop_Thread_Enregistrement_data              */
/*                                                      */
/********************************************************/
int Stop_Thread_Enregistrement_data(struct param_pgm *param)
{
	int ok;
	char answer_2[2][_RECEIVE_BUFF_SIZE]={'\0','\0'};

	ok=pthread_cancel (param->pshared->thread_enregistrement); 

	Send_command_and_receive_answer_2(param->pparam_connection, "MEASure:RUN?", 2 ,answer_2, 0);
	//printf("%s\n",answer_2[0],answer_2[1]);
	if (answer_2[0][0]!='0')
	{
		Zero_str(answer_2[0]);Zero_str(answer_2[1]);
		Send_command_and_receive_answer_2(param->pparam_connection, "MEASure:STOP", 2 ,answer_2, 0);
		//printf("%s\n",answer_2[0],answer_2[1]);
	}

	if (ok==0){ok=1;}else{ok=0;}

	return ok;

}





/********************************************************/
/*Fonction: thread_Enregistrement_data                  */
/*                                                      */
/********************************************************/
void* thread_Enregistrement_data (void* arg)
{
	struct param_pgm  *p_data=arg;
	int ok=1,ok_open,ok_close,ok_record,nb_channel_actif;
	int i,j,k,m,ok_find,nb_i,nb_meas;
	int nb,ok_reception,ok_reception_1,ok_reception_2;
	int nb_data_by_paquet,nb_data_to_write_from_paquet, inc_nb_data_to_write;
	float size_max_save_file,size_file,size_written;
	float data1,data2;
	double delta_t_echantillonnage,temp_tv_usec;
	char buffer[26],str_date_time[100]={'\0'},str_value[50]={'\0'},String_SendMeasureType_t[20000]={'\0'};

	struct timeval tp, tp_add, tp_inc;
	gettimeofday(&tp,0);

	time_t curtime=tp.tv_sec;
	struct tm *t=localtime(&curtime);

	tp_inc=tp;
	timerclear(&tp_inc);


	time_t curtime_add;
	struct tm *t_add; 

	uint8_t val_ch[2]={1,2};

	struct stUDPSendMeasureType_t SendMeasureType_t_ch[NB_CH];


	FILE *fp;

	fp=p_data->pshared->fp;

	size_max_save_file=p_data->pconfig_all->pconfig_save_file->size_max_save_file*1000;


	//printf("MEASureRATE %d\n",p_data->pconfig_all->pconfig_meas->MEASureRATE);
	//printf("SAMPLingrate %d\n",p_data->pconfig_all->pconfig_meas->SAMPLingrate);
	//printf("Freq_echantillonnage %d\n",p_data->pconfig_all->pconfig_meas->Freq_echantillonnage);



	nb_data_by_paquet=p_data->pconfig_all->pconfig_meas->SAMPLingrate/p_data->pconfig_all->pconfig_meas->MEASureRATE;
	//printf("nb_data_by_paquet %d\n",nb_data_by_paquet);
	nb_data_to_write_from_paquet=nb_data_by_paquet/(p_data->pconfig_all->pconfig_meas->SAMPLingrate/p_data->pconfig_all->pconfig_meas->Freq_echantillonnage);
	//printf("nb_data_to_write_from_paquet %d\n",nb_data_to_write_from_paquet);
	delta_t_echantillonnage=1000.0/((double)(p_data->pconfig_all->pconfig_meas->SAMPLingrate));
	//printf("delta_t_echantillonnage %lf\n",delta_t_echantillonnage);

	inc_nb_data_to_write=p_data->pconfig_all->pconfig_meas->SAMPLingrate/p_data->pconfig_all->pconfig_meas->Freq_echantillonnage;
	//printf("inc_nb_data_to_write %d\n",inc_nb_data_to_write);


	nb_i=0;

	pthread_mutex_lock(p_data->pshared->mutex);
	ok_record=p_data->pshared->ok_record;
	pthread_mutex_unlock(p_data->pshared->mutex);

	//printf("ok_record %d\n",ok_record);
	nb_meas=p_data->pshared->nb_meas_done;
	while(ok_record)
	{

		size_file=p_data->pshared->size_save_file;

		nb_channel_actif=0;
		for(i=0;i<NB_CH;i++)
		{ 
			nb_channel_actif=nb_channel_actif+p_data->pconfig_all->pconfig_meas->select_ch[i];
		}

		ok_reception=1;
		for(i=0;i<nb_channel_actif;i++)
		{

			Zero_stUDPSendMeasureType_t (&SendMeasureType_t_ch[i]);

			if(Reception_data(p_data->pparam_connection,&SendMeasureType_t_ch[i],0)==0)
			{   
				ok_reception=0;printf("\n%s\n","Erreur in reception data");break;
			} 
		}
		/*    
		      if(nb_i<4)
		      {
		      Print_stUDPSendMeasureType_t(&SendMeasureType_t_ch[0]);
		      Print_stUDPSendMeasureType_t(&SendMeasureType_t_ch[1]);
		      }
		      nb_i++;
		      */
		//    printf("\n%s %d\n\n","Mesure",i); 
		// printf("\n%d\n",SendMeasureType_t_ch[i].stHeader.stMeasureHeader.ui8ChannelID);


		if(ok_reception==0)
		{
			Envoi_SMS_alert();
			goto fin_enregistrement;
		}else{
			t->tm_year=2000+SendMeasureType_t_ch[0].stHeader.stMeasureHeader.ui16Year-1900;
			t->tm_mon=SendMeasureType_t_ch[0].stHeader.stMeasureHeader.ui8Month;
			t->tm_mday=SendMeasureType_t_ch[0].stHeader.stMeasureHeader.ui8Day;
			t->tm_hour=SendMeasureType_t_ch[0].stHeader.stMeasureHeader.ui8Hour;
			t->tm_min=SendMeasureType_t_ch[0].stHeader.stMeasureHeader.ui8Min;
			t->tm_sec=SendMeasureType_t_ch[0].stHeader.stMeasureHeader.ui8Seconds;

			curtime=mktime(t);
			tp.tv_sec=curtime;
			// tp.tv_usec=(double)(SendMeasureType_t_ch[0].stHeader.stMeasureHeader.ui810thOfSeconds)*1000.0;
			temp_tv_usec=(double)(SendMeasureType_t_ch[0].stHeader.stMeasureHeader.ui810thOfSeconds)*1000.0;
			tp.tv_usec=1000.0-(temp_tv_usec/256.0);
			//   printf("%02d/%02d/%04d  %02d:%02d:%02d.%03d\n",t->tm_mday,t->tm_mon,t->tm_year+1900,t->tm_hour, t->tm_min,t->tm_sec,tp.tv_usec/1000);


			Zero_str(String_SendMeasureType_t);

			for (i=0;i<nb_data_by_paquet;i=i+inc_nb_data_to_write)
			{
				Zero_str(str_date_time);
				timerclear(&tp_add);
				tp_inc.tv_usec=(double)(i)*delta_t_echantillonnage*1000.0;
				tp_inc.tv_sec=0.0;
				timeradd(&tp,&tp_inc,&tp_add);
				curtime_add=tp_add.tv_sec;
				t_add=localtime(&curtime_add);
				//    printf("%d***%02d/%02d/%04d  %02d:%02d:%02d.%03d\n",i, t_add->tm_mday,t_add->tm_mon,t_add->tm_year+1900,t_add->tm_hour, t_add->tm_min,t_add->tm_sec,tp_add.tv_usec/1000);

				sprintf(str_date_time,"%02d/%02d/%04d  %02d:%02d:%02d.%03ld\t", t_add->tm_mday,t_add->tm_mon,t_add->tm_year+1900,t_add->tm_hour, t_add->tm_min,t_add->tm_sec,tp_add.tv_usec/1000); 
				strcat(String_SendMeasureType_t,str_date_time); 

				for(j=0;j<NB_CH;j++)
				{

					Zero_str(str_value);
					if (p_data->pconfig_all->pconfig_meas->select_ch[j])
					{
						k=0;
						ok_find=0;
						while((ok_find==0)&&(k<NB_CH))
						{

							if(SendMeasureType_t_ch[k].stHeader.stMeasureHeader.ui8ChannelID!=val_ch[j]){ok_find=0;}else{ok_find=1;}
							k++;
						}
						if((k>=NB_CH)&&(ok_find==0))
						{
							sprintf(str_value,"%s\t","NULL");
						}else{
							pthread_mutex_lock(p_data->pshared->mutex);
							p_data->pshared->ch_value[k-1]=SendMeasureType_t_ch[k-1].fMeasure[i];
							pthread_mutex_unlock(p_data->pshared->mutex);
							sprintf(str_value,"%lf\t",SendMeasureType_t_ch[k-1].fMeasure[i]);                      
						}
						strcat(String_SendMeasureType_t,str_value); 
					}
				}

				strcat(String_SendMeasureType_t,"\n");  //  printf("%s\n",String_SendMeasureType_t);

				nb_meas++;
				pthread_mutex_lock(p_data->pshared->mutex);
				p_data->pshared->nb_meas_done=nb_meas;
				pthread_mutex_unlock(p_data->pshared->mutex);

				//printf("%d %d %d\n",p_data->pconfig_all->pconfig_meas->mode_meas, p_data->pshared->nb_meas_done,nb_meas);
				if ((p_data->pconfig_all->pconfig_meas->mode_meas > 0)&&(p_data->pshared->nb_meas_done >= p_data->pconfig_all->pconfig_meas->mode_meas))
				{
					size_written=fprintf(fp,"%s",String_SendMeasureType_t);
					fflush(stdout);
					goto fin_enregistrement;
				}

			}

			pthread_mutex_lock(p_data->pshared->mutex);
			size_written=fprintf(fp,"%s",String_SendMeasureType_t);
			fflush(stdout);
			p_data->pshared->size_save_file=size_file+size_written;
			pthread_mutex_unlock(p_data->pshared->mutex);
			//printf("%s\n",String_SendMeasureType_t);

		}


		//sleep(1.0);
		//size_written=fprintf(fp,"%f %f %f\n",size_file,p_data->pshared->size_save_file,size_max_save_file);



		if (p_data->pshared->size_save_file >= size_max_save_file)
		{

			if(Fermeture_ouverture_new_file(p_data)==0)
			{
				printf("%s\n","Bizarre!!!!!!!!!!!!!");
				pthread_mutex_lock(p_data->pshared->mutex);
				p_data->pshared->cmd_acq='s';
				pthread_mutex_unlock(p_data->pshared->mutex);
				goto fin_enregistrement;
			}

			fp=p_data->pshared->fp;

		}

		//time_meas_prec_ch1=time_meas_ch1;

		//printf("%s %d\n","thread_Enregistrement_data 2",ok_record);
		pthread_mutex_lock(p_data->pshared->mutex);
		ok_record=p_data->pshared->ok_record;
		pthread_mutex_unlock(p_data->pshared->mutex);
		//printf("%s %d\n","thread_Enregistrement_data 3",ok_record);
	}


fin_enregistrement:; 

		   ok_record=0;
		   pthread_mutex_lock(p_data->pshared->mutex);
		   p_data->pshared->ok_record=ok_record;
		   //p_data->pshared->cmd_acq='s';
		   pthread_mutex_unlock(p_data->pshared->mutex);



		   //return ok;
}



/********************************************************/
/*Fonction: Fermeture_ouverture_new_file                */
/*                                                      */
/********************************************************/
int Fermeture_ouverture_new_file(struct param_pgm *param)
{
	int ok,ok_close,ok_open;

	FILE *fp;

	ok=1;

	ok_close=Close_file_Enregistrement_data(param);
	pthread_mutex_lock(param->pshared->mutex);
	param->pshared->size_save_file=0.0;
	pthread_mutex_unlock(param->pshared->mutex);
	if(ok_close)
	{
		pthread_mutex_lock(param->pshared->mutex);
		param->pshared->nb_save_file=param->pshared->nb_save_file+1;
		pthread_mutex_unlock(param->pshared->mutex);
		//printf("%s\n","Open new file");
	}else{
		ok=0;
		printf("%s\n","Erreur lors de la fermeture du fichier de sauvegarde");
		goto fin_fermeture_ouverture_new_file;
	}

	if(Verif_free_space(param,0))
	{
		ok_open=Open_file_Enregistrement_data(param);
		/*pthread_mutex_lock(param->pshared->mutex);
		  fp=param->pshared->fp;
		  pthread_mutex_unlock(param->pshared->mutex);*/
	}else{
		ok_open=0;
		printf("%s\n","Erreur il n'y a plus assez d'espace pour enregistrer les données");
		goto fin_fermeture_ouverture_new_file;
	}

fin_fermeture_ouverture_new_file: ;
				  if (ok_open==0){ok=0;};

				  return ok;
}



/********************************************************/
/*Fonction: Reception_data                              */
/*                                                      */
/********************************************************/
int Reception_data(struct parametres_connexion *param_connection, struct stUDPSendMeasureType_t *SendMeasureType_t, int ok_print)
{
	int ok=1,nb_ans;

	fd_set select_fds;
	struct timeval timeout;


	FD_ZERO(&select_fds);
	FD_SET(param_connection->ID_socket_acquisition_data, &select_fds);

	timeout.tv_sec=TIMEOUT_S;
	timeout.tv_usec=TIMEOUT_MS*1000;

	//printf("%s\n","Reception_data ");

	//&timeout

	if(select(sizeof(struct stUDPSendMeasureType_t), &select_fds, NULL, NULL, NULL) == 0)
	{
		printf("%s\n","Select has timed out ..."); 
		printf("%s\n","Pas de réponse de WLX2");
		ok=0;
	}else{

		recvfrom(param_connection->ID_socket_acquisition_data,SendMeasureType_t,sizeof(struct stUDPSendMeasureType_t),0,NULL,NULL);

		//Print_stUDPSendMeasureType_t(SendMeasureType_t);

		if (SendMeasureType_t->stHeader.ui16DataID !=_OPTICAL_MEASURE)
		{
			ok=0;
			printf("%s %d %s %d %s\n","Erreur: DataID de la réponse reçue : ",SendMeasureType_t->stHeader.ui16DataID," (",_OPTICAL_MEASURE," attendu)");
		}


		if (ok_print){Print_stUDPSendMeasureType_t(SendMeasureType_t);}
	}

fin_Reception__data:;

		    return ok;

}






/********************************************************/
/*Fonction: Zero_stUDPSendMeasureType_t                 */
/*                                                      */
/********************************************************/
int Zero_stUDPSendMeasureType_t(struct stUDPSendMeasureType_t *SendMeasureType_t)
{
	int i;



	SendMeasureType_t->stHeader.stMeasureHeader.ui16DataSize=0;
	SendMeasureType_t->stHeader.stMeasureHeader.ui8DataType=0;
	SendMeasureType_t->stHeader.stMeasureHeader.ui8ModuleID=0;
	SendMeasureType_t->stHeader.stMeasureHeader.ui8ChannelID=0;
	SendMeasureType_t->stHeader.stMeasureHeader.ui8MeasureUnit=0;
	SendMeasureType_t->stHeader.stMeasureHeader.ui16Year=0;
	SendMeasureType_t->stHeader.stMeasureHeader.ui8Month=0;
	SendMeasureType_t->stHeader.stMeasureHeader.ui8Day=0;
	SendMeasureType_t->stHeader.stMeasureHeader.ui8Hour=0;
	SendMeasureType_t->stHeader.stMeasureHeader.ui8Min=0;
	SendMeasureType_t->stHeader.stMeasureHeader.ui8Seconds=0;
	SendMeasureType_t->stHeader.stMeasureHeader.ui810thOfSeconds=0;

	SendMeasureType_t->stHeader.ui16DataID=0;
	SendMeasureType_t->stHeader.ui8SegmentID=0;
	SendMeasureType_t->stHeader.ui8SegmentQty=0;


	for (i=0;i<_MAX_FLOAT_MEASURES;i++)
	{
		SendMeasureType_t->fMeasure[i]=0.0;
	}
}





/********************************************************/
/*Fonction: Print_stUDPSendMeasureType_t               */
/*                                                      */
/********************************************************/
int Print_stUDPSendMeasureType_t(struct stUDPSendMeasureType_t *SendMeasureType_t)
{
	int i;
	uint16_t ui16DataID;
	uint8_t ui8SegmentID;
	uint8_t ui8SegmentQty;
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
	float fMeasure[_MAX_FLOAT_MEASURES];


	ui16DataSize=SendMeasureType_t->stHeader.stMeasureHeader.ui16DataSize;
	ui8DataType=SendMeasureType_t->stHeader.stMeasureHeader.ui8DataType;
	ui8ModuleID=SendMeasureType_t->stHeader.stMeasureHeader.ui8ModuleID;
	ui8ChannelID=SendMeasureType_t->stHeader.stMeasureHeader.ui8ChannelID;
	ui8MeasureUnit=SendMeasureType_t->stHeader.stMeasureHeader.ui8MeasureUnit;
	ui16Year=SendMeasureType_t->stHeader.stMeasureHeader.ui16Year;
	ui8Month=SendMeasureType_t->stHeader.stMeasureHeader.ui8Month;
	ui8Day=SendMeasureType_t->stHeader.stMeasureHeader.ui8Day;
	ui8Hour=SendMeasureType_t->stHeader.stMeasureHeader.ui8Hour;
	ui8Min=SendMeasureType_t->stHeader.stMeasureHeader.ui8Min;
	ui8Seconds=SendMeasureType_t->stHeader.stMeasureHeader.ui8Seconds;
	ui810thOfSeconds=SendMeasureType_t->stHeader.stMeasureHeader.ui810thOfSeconds;

	ui16DataID=SendMeasureType_t->stHeader.ui16DataID;
	ui8SegmentID=SendMeasureType_t->stHeader.ui8SegmentID;
	ui8SegmentQty=SendMeasureType_t->stHeader.ui8SegmentQty;


	for (i=0;i<_MAX_FLOAT_MEASURES;i++)
	{
		fMeasure[i]=SendMeasureType_t->fMeasure[i];
	}

	printf("%s\n","Paquet de données reçues");
	printf("%s %u\n","DataID: ",ui16DataID);
	printf("%s %u\n","SegmentID: ",ui8SegmentID);
	printf("%s %u\n","SegmentQty: ",ui8SegmentQty);
	printf("%s %u\n","DataSize: ",ui16DataSize);
	printf("%s %u\n","DataType: ",ui8DataType);
	printf("%s %u\n","ModuleID: ",ui8ModuleID);
	printf("%s %u\n","ChannelID: ",ui8ChannelID);
	printf("%s %u\n","MeasureUnit: ",ui8MeasureUnit);
	printf("%s %u\n","Year: ",ui16Year);
	printf("%s %u\n","Month: ",ui8Month);
	printf("%s %u\n","Day: ",ui8Day);
	printf("%s %u\n","Hour: ",ui8Hour);
	printf("%s %u\n","Min: ",ui8Min);
	printf("%s %u\n","Seconds: ",ui8Seconds);
	printf("%s %u\n","Milliseconds: ",ui810thOfSeconds);
	printf("%s","Value: ");
	for (i=0;i<_MAX_FLOAT_MEASURES;i++)
	{
		printf("%f\t",fMeasure[i]);
	}
	printf("%s\n","");
}






/********************************************************/
/*Fonction: Convert_stUDPSendMeasureType_t__to__string  */
/*                                                      */
/********************************************************/
void Convert_stUDPSendMeasureType_t__to__string(struct stUDPSendMeasureType_t *SendMeasureType_t, char *string_SendMeasureType_t)
{
	int i;
	uint16_t ui16DataID;
	uint8_t ui8SegmentID;
	uint8_t ui8SegmentQty;
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
	float fMeasure[_MAX_FLOAT_MEASURES];
	char temp_str1[100]={'\0'},temp_str2[100]={'\0'},temp_str0[100]={'\0'};

	Zero_str(string_SendMeasureType_t);
	Zero_str(temp_str1);Zero_str(temp_str2);

	ui16Year=SendMeasureType_t->stHeader.stMeasureHeader.ui16Year;
	ui8Month=SendMeasureType_t->stHeader.stMeasureHeader.ui8Month;
	ui8Day=SendMeasureType_t->stHeader.stMeasureHeader.ui8Day;
	ui8Hour=SendMeasureType_t->stHeader.stMeasureHeader.ui8Hour;
	ui8Min=SendMeasureType_t->stHeader.stMeasureHeader.ui8Min;
	ui8Seconds=SendMeasureType_t->stHeader.stMeasureHeader.ui8Seconds;
	ui810thOfSeconds=SendMeasureType_t->stHeader.stMeasureHeader.ui810thOfSeconds;

	sprintf(temp_str1,"%02u/%02u/%04u  %02u:%02u:%02u.%03u \n",ui8Day,ui8Month,ui16Year,ui8Hour,ui8Min,ui8Seconds,ui810thOfSeconds);


	for (i=0;i<_MAX_FLOAT_MEASURES;i++)
	{
		Zero_str(temp_str0);
		fMeasure[i]=SendMeasureType_t->fMeasure[i];
		sprintf(temp_str0, "%f ",fMeasure[i]);
		strcat(temp_str2,temp_str0);
	}

	strcat(string_SendMeasureType_t,temp_str1);
	strcat(string_SendMeasureType_t,temp_str2);

}

