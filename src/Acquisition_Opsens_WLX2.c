#include "Acquisition_Opsens_WLX2.h" 
#include <math.h>
#include <float.h>
#include <sys/time.h>

static void stats(struct stUDPSendMeasureType_t *ch1, 
		struct stUDPSendMeasureType_t *ch2, int nb_measures);
static void fill_time_struct(struct tm *t, struct stUDPMeasureHeader_t *st_M_H);

static void fill_time_struct(struct tm *t, struct stUDPMeasureHeader_t *st_M_H)
{
	t->tm_year=2000+st_M_H->ui16Year-1900;
	t->tm_mon= st_M_H->ui8Month;
	t->tm_mday=st_M_H->ui8Day;
	t->tm_hour=st_M_H->ui8Hour;
	t->tm_min= st_M_H->ui8Min;
	t->tm_sec= st_M_H->ui8Seconds;
}

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

	if(pthread_mutex_lock(param->pshared->mutex) == 0)
	{
		nomfile_data=param->pconfig_all->pconfig_save_file->nomfic;
		nb_file_save=param->pshared->nb_save_file;
		rep_data=param->pshared->chemin; 
		pthread_mutex_unlock(param->pshared->mutex);
	}

	nb=nb_file_save+1;
	sprintf(suffixe_nb,"%d",nb);
	nomfic_len = strlen(rep_data) + strlen(suffixe2) + strlen(nomfile_data)
		+ strlen(suffixe1) + strlen(suffixe_nb) + 1;

	nomfic_save = malloc(nomfic_len*sizeof(char));
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

	free(nomfic_save);

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
	char GF0[10]={'\0'},GF1[10]={'\0'},GF2[10]={'\0'},GF3[10]={'\0'};
	char GF[100]={'\0'}, type_jauge[100]={'\0'},numero_jauge[100]={'\0'};
	FILE *fp;

	fp=param->pshared->fp;



	fprintf(fp, "#%s %s\n","Projet: ", param->pconfig_all->pconfig_save_file->nom_projet);
	fprintf(fp,"#%s\n"," ");

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
	char command1[100]="MEASure:STARt INFInite";
	char command2[100]="MEASure:RUN?";
	char reponse_ok[3]="ok";
	char answer1[100]={'\0'},answer2[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};


	Zero_str(answer1);Zero_str(answer2);

	if(!Send_command_and_receive_answer_2(param_connection,command1,
				NB_CH,answer_2, 0)
			|| !Compare_2str(reponse_ok, answer_2[0]))
		return 0;

	Zero_str(answer_2[0]);
	Zero_str(answer_2[1]);

	if(!Send_command_and_receive_answer_2(param_connection, command2,
				NB_CH, answer_2, 0) || !answer_2[0])
		return 0;

	return 1;
}




/********************************************************/
/*Fonction: N_Measure_start                      */
/*                                                      */
/********************************************************/
int N_Measure_start (struct parametres_connexion *param_connection, int nb_meas_to_do)
{
	char prefix_command1[100]="MEASure:STARt ";
	char command1[100]={'\0'};
	char temp_str[100]={'\0'};
	char reponse_ok[3]="ok";
	char answer1[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};

	Zero_str(answer1);Zero_str(command1);
	strcat(command1,prefix_command1);
	sprintf(temp_str, "%d", nb_meas_to_do);
	strcat(command1,temp_str); 

	if(!Send_command_and_receive_answer_2(param_connection,command1,
				NB_CH,answer_2, 0)
			|| !Compare_2str(reponse_ok, answer_2[0]))
		return 0;


	return 1;
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
	pthread_t xthread_start_single_measurement;
	pthread_t xthread_receive_single_measurement;

	if(!pthread_create (&xthread_start_single_measurement, 
				NULL, thread_start_single_measurement, p_data)
			&& !pthread_create (&xthread_receive_single_measurement,
				NULL, thread_receive_single_measurement, p_data)
			&& !pthread_join(xthread_start_single_measurement,
				NULL) 
			&& !pthread_join(xthread_receive_single_measurement,
				NULL))
		return 1;
	return 0;
}


/********************************************************/
/*Fonction: thread_start_single_measurement             */
/*                                                      */
/********************************************************/
void* thread_start_single_measurement (void* arg)
{
	struct param_pgm  *p_data=arg;

	pthread_mutex_lock(&mutex_single_measurement);
	
	pthread_cond_wait(&condition_single_measurement,&mutex_single_measurement);
	N_Measure_start(p_data->pparam_connection,1);
	
	pthread_mutex_unlock(&mutex_single_measurement);

	pthread_exit(NULL);
}



/********************************************************/
/*Fonction: thread_receive_single_measurement           */
/*                                                      */
/********************************************************/
void* thread_receive_single_measurement (void* arg)
{
	struct param_pgm  *p_data=arg;
	int i,j,nb_channel_actif;

	struct stUDPSendMeasureType_t SendMeasureType_t_ch[NB_CH];


	nb_channel_actif=0;
	for(i=0;i<NB_CH;i++)
	{ 
		nb_channel_actif += 
			p_data->pconfig_all->pconfig_meas->select_ch[i];
	}

	for(i=0;i<nb_channel_actif;i++)
	{
		if(i==0)
		{
			pthread_mutex_lock(&mutex_single_measurement);
			pthread_cond_signal(&condition_single_measurement);
			pthread_mutex_unlock(&mutex_single_measurement);
		}

		Zero_stUDPSendMeasureType_t (&SendMeasureType_t_ch[i]); 
		if(Reception_data(p_data->pparam_connection,
					&SendMeasureType_t_ch[i],0)==0)
		{   
			printf("\n%s\n","Erreur in reception data");;
			pthread_exit(NULL);
		}
	}

	for(i=0;i<nb_channel_actif;i++)
	{
		p_data->pshared->ch_value[i]=-99999.99;
	}
	for(i=0;i<nb_channel_actif;i++)
	{
		j = SendMeasureType_t_ch[i].stHeader.stMeasureHeader.ui8ChannelID-1;
		p_data->pshared->ch_value[j] = 
			SendMeasureType_t_ch[i].fMeasure[0];
	}

	pthread_exit(NULL);
}




/********************************************************/
/*Fonction: Measure_stop                                */
/*                                                      */
/********************************************************/
int Measure_stop (struct parametres_connexion *param_connection)
{
	char command1[100]="MEASure:STOP";
	char command2[100]="MEASure:RUN?";
	char reponse_ok[3]="ok";
	char answer1[100]={'\0'},answer2[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};


	Zero_str(answer1);Zero_str(answer2);

	if(!Send_command_and_receive_answer_2(param_connection,command1,
				NB_CH,answer_2, 0) 
			|| !Compare_2str(reponse_ok, answer_2[0]))
		return 0;

	Zero_str(answer_2[0]);
	Zero_str(answer_2[1]);

	if(!Send_command_and_receive_answer_2(param_connection,command2,
				NB_CH,answer_2, 0)
			|| !Compare_2str("0", answer_2[0]))
		return 0;
	return 1;
}




/********************************************************/
/*Fonction: Zero_sensor                                */
/*                                                      */
/********************************************************/
int Zero_sensor(struct parametres_connexion *param_connection, int ch1_zero, int ch2_zero)
{
	char command1a[10]="SENSor",command1b[10]=":ZERO";
	char command[100]={'\0'};
	char reponse_ok[3]="ok"; //?WLX2 doesn't send ok back, but value of zero
	char answer1[100]="\0",answer2[100]="\0";
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};


	if (ch1_zero)
	{
		strcat(command,command1a);
		strcat(command,"1");
		strcat(command,command1b);

		Zero_str(answer1);Zero_str(answer2);

		if(!Send_command_and_receive_answer_2(param_connection,command,
					NB_CH,answer_2, 0)
				//|| !Compare_2str(reponse_ok, answer_2[0]))
				)
			return 0;
	}


	if (ch2_zero)
	{
		Zero_str(command);
		strcat(command,command1a);
		strcat(command,"2");
		strcat(command,command1b);

		Zero_str(answer1);Zero_str(answer2);
		printf("%s,\n", command);
		if(!Send_command_and_receive_answer_2(param_connection,command,
					NB_CH,answer_2, 0)
				//|| !Compare_2str(reponse_ok, answer_2[0]))
			)
			return 0;
	}

	printf("Sensor zeroed\n");
	return 1;
}





/********************************************************/
/*Fonction: Get_zero_sensor                             */
/*                                                      */
/********************************************************/
int Get_zero_sensor(struct parametres_connexion *param_connection, int ch_zero, float *zero_channel)
{
	char command1a[10]="SENSor",command1b[10]=":ZERO?";
	char command[100]={'\0'};
	char temp_str[100]={'\0'};
	char answer1[100]={'\0'},answer2[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};


	sprintf(temp_str, "%d",ch_zero);

	strcat(command,command1a);
	strcat(command,temp_str);

	strcat(command,command1b);

	Zero_str(answer1);Zero_str(answer2);
	if(!Send_command_and_receive_answer_2(param_connection,command,
				NB_CH,answer_2, 0))
		return 0;

	if (ch_zero==1)
		sscanf(answer_2[0],"%f",zero_channel);
	if (ch_zero==2)
		sscanf(answer_2[1],"%f",zero_channel);

	return 1;
}




/********************************************************/
/*Fonction: Get_offset_sensor                             */
/*                                                      */
/********************************************************/
int Get_offset_sensor(struct parametres_connexion *param_connection, int ch_offset, float *offset_channel)
{
	char command1a[10]="SENSor",command1b[10]=":OFFSet?";
	char command[100]={'\0'};
	char temp_str[100]={'\0'};
	char answer1[100]={'\0'},answer2[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};

	sprintf(temp_str, "%d",ch_offset);

	strcat(command,command1a);
	strcat(command,temp_str);

	strcat(command,command1b);

	Zero_str(answer1);
	Zero_str(answer2);
	
	if(!Send_command_and_receive_answer_2(param_connection,command,
				NB_CH,answer_2, 0))
		return 0;

	if (ch_offset==1)
		sscanf(answer_2[0],"%f",offset_channel);
	if (ch_offset==2)
		sscanf(answer_2[1],"%f",offset_channel);

	return 1;
}



/********************************************************/
/*Fonction: Set_offset_sensor                             */
/*                                                      */
/********************************************************/
int Set_offset_sensor(struct parametres_connexion *param_connection, int ch_offset, float offset_channel)
{
	char command1a[10]="SENSor",command1b[10]=":OFFSet";
	char command[100]={'\0'};
	char temp_str[100]={'\0'};
	char answer1[100]={'\0'},answer2[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};

	sprintf(temp_str, "%d",ch_offset);
	strcat(command,command1a);
	strcat(command,temp_str);
	strcat(command,command1b);
	strcat(command," ");

	sprintf(temp_str, "%f",offset_channel);
	strcat(command,temp_str);

	Zero_str(answer1);Zero_str(answer2);
	if(!Send_command_and_receive_answer_2(param_connection,command, 
				NB_CH,answer_2, 0))
		return 0;
	if (ch_offset==1)
		strcat(answer1,answer_2[0]);
	if (ch_offset==2)
		strcat(answer1,answer_2[1]);
	if(!Compare_2str(answer_2[0], answer1))
		return 0;

	return 1;
}



/********************************************************/
/*Fonction: Run_Thread_Enregistrement_data              */
/*                                                      */
/********************************************************/
int Run_Thread_Enregistrement_data(struct param_pgm *param)
{
	pthread_t xthread_Enregistrement_data;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	if(!pthread_create (&xthread_Enregistrement_data, &attr,
			       thread_Enregistrement_data, param))
	{
		param->pshared->thread_enregistrement =
			xthread_Enregistrement_data;
		return 1;
	}
	return 0;
}


/********************************************************/
/*Fonction: Stop_Thread_Enregistrement_data              */
/*                                                      */
/********************************************************/
int Stop_Thread_Enregistrement_data(struct param_pgm *param)
{
	int ok = 1;
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};

	if(pthread_cancel (param->pshared->thread_enregistrement))
		ok = 0;

	Send_command_and_receive_answer_2(param->pparam_connection,
			"MEASure:RUN?", 2 ,answer_2, 0);

	if (answer_2[0][0]!='0')
	{
		Zero_str(answer_2[0]);Zero_str(answer_2[1]);
		Send_command_and_receive_answer_2(param->pparam_connection,
				"MEASure:STOP", 2 ,answer_2, 0);
		printf("\tMeasure:STOP sent\n");
	}

	return ok;
}





/********************************************************/
/*Fonction: thread_Enregistrement_data                  */
/*                                                      */
/********************************************************/
void* thread_Enregistrement_data (void* arg)
{
	struct param_pgm *p_data = arg;

	int nb_channel_actif, nb_meas, nb_data_by_paquet, inc_nb_data_to_write;
	int i,j,k;
	int ok_record;

	float size_max_save_file, size_file,size_written;
	double delta_t_echantillonnage, temp_tv_usec;

	char str_date_time[100]={'\0'};
	char str_value[50]={'\0'};
	char String_SendMeasureType_t[20000]={'\0'};

	struct timeval tp, tp_add, tp_inc;
	gettimeofday(&tp,0);

	time_t curtime=tp.tv_sec;
	struct tm *t=localtime(&curtime);

	tp_inc=tp;
	timerclear(&tp_inc);

	time_t curtime_add;
	struct tm *t_add; 


	uint8_t id_ch[NB_CH]={1,2};

	struct stUDPSendMeasureType_t SendMeasureType_t_ch[NB_CH];

	FILE *fp = p_data->pshared->fp;


	// Init from shared memory
	size_max_save_file = 
		p_data->pconfig_all->pconfig_save_file->size_max_save_file*1000;
	nb_data_by_paquet = p_data->pconfig_all->pconfig_meas->SAMPLingrate / 
		p_data->pconfig_all->pconfig_meas->MEASureRATE;
	delta_t_echantillonnage = 1000.0 / 
		((double)(p_data->pconfig_all->pconfig_meas->SAMPLingrate));
	inc_nb_data_to_write = p_data->pconfig_all->pconfig_meas->SAMPLingrate /
		p_data->pconfig_all->pconfig_meas->Freq_echantillonnage;


	pthread_mutex_lock(p_data->pshared->mutex);
	ok_record=p_data->pshared->ok_record;
	pthread_mutex_unlock(p_data->pshared->mutex);

	nb_meas=p_data->pshared->nb_meas_done;

	while(ok_record)
	{
		// Don't quit while acquisition is running
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		size_file=p_data->pshared->size_save_file;

		nb_channel_actif=0;
		for(i=0;i<NB_CH;i++)
		{ 
			nb_channel_actif += 
				p_data->pconfig_all->pconfig_meas->select_ch[i];
		}

		for(i=0;i<nb_channel_actif;i++)
		{

			Zero_stUDPSendMeasureType_t (&SendMeasureType_t_ch[i]);

			if(!Reception_data(p_data->pparam_connection,
						&SendMeasureType_t_ch[i],0))
			{   
				printf("\n%s\n","Erreur in reception data");

				pthread_mutex_lock(p_data->pshared->mutex);
				p_data->pshared->ok_record=0;
				pthread_mutex_unlock(p_data->pshared->mutex);

				pthread_exit(NULL);
			} 
		}

		// Use UDP header to get a reference time tp of 
		// the first data point of the packet
		fill_time_struct(t, &SendMeasureType_t_ch[0].stHeader.
				stMeasureHeader);	
		curtime = mktime(t);
		tp.tv_sec = curtime;
		temp_tv_usec = (double)(SendMeasureType_t_ch[0].stHeader
				.stMeasureHeader.ui810thOfSeconds)*1000.0;
		tp.tv_usec=1000.0-(temp_tv_usec/256.0);

		Zero_str(String_SendMeasureType_t);

		for (i=0 ; i<nb_data_by_paquet ; i += inc_nb_data_to_write)
		{
			// Gets the time of the data point i, with 
			// time(i) = time(0) + i*dt
			timerclear(&tp_add);
			tp_inc.tv_usec=(double)(i)*delta_t_echantillonnage*1000.0;
			tp_inc.tv_sec=0.0;
			timeradd(&tp,&tp_inc,&tp_add);
			curtime_add=tp_add.tv_sec;
			t_add=localtime(&curtime_add);

			//Add the data to the string
			Zero_str(str_date_time);
			sprintf(str_date_time,"%02d/%02d/%04d  "
					"%02d:%02d:%02d.%03ld\t",
					t_add->tm_mday,
					t_add->tm_mon,
					t_add->tm_year+1900,
					t_add->tm_hour,
					t_add->tm_min,
					t_add->tm_sec,
					tp_add.tv_usec/1000); 
			strcat(String_SendMeasureType_t,str_date_time); 

			for(j=0;j<NB_CH;j++)
			{
				Zero_str(str_value);

				if (!p_data->pconfig_all->pconfig_meas->select_ch[j])
					continue;

				// Find the udp data corresponding with the 
				// right channel id
				for(k=0 ; k<NB_CH; k++)
				{
					if(SendMeasureType_t_ch[k].stHeader.stMeasureHeader.ui8ChannelID
							== id_ch[j])
						break;
				}
				if(k>=NB_CH)
					sprintf(str_value,"%s\t","NULL");
				else
				{
					// Add the data point value to the string
					pthread_mutex_lock(p_data->pshared->mutex);
					p_data->pshared->ch_value[k] =
						SendMeasureType_t_ch[k].fMeasure[i];
					pthread_mutex_unlock(p_data->pshared->mutex);

					sprintf(str_value,"%lf\t",
							SendMeasureType_t_ch[k].fMeasure[i]);                      
				}
				strcat(String_SendMeasureType_t,str_value); 
			}

			strcat(String_SendMeasureType_t,"\n"); 

			nb_meas++;

			pthread_mutex_lock(p_data->pshared->mutex);
			p_data->pshared->nb_meas_done=nb_meas;
			pthread_mutex_unlock(p_data->pshared->mutex);

			if (p_data->pconfig_all->pconfig_meas->mode_meas > 0 &&
					p_data->pshared->nb_meas_done >= 
					p_data->pconfig_all->pconfig_meas->mode_meas)
			{
				size_written=fprintf(fp,"%s",String_SendMeasureType_t);
				fflush(stdout);
				pthread_mutex_lock(p_data->pshared->mutex);
				p_data->pshared->ok_record=0;
				pthread_mutex_unlock(p_data->pshared->mutex);

				pthread_exit(NULL);
			}

		}

		// Use the data collected for statistics
		stats(&SendMeasureType_t_ch[0], &SendMeasureType_t_ch[1], 
				nb_data_by_paquet);

		// Print the string containing data and value of each data point
		// in the last packet
		pthread_mutex_lock(p_data->pshared->mutex);
		size_written = fprintf(fp,"%s",String_SendMeasureType_t);
		fflush(stdout);
		p_data->pshared->size_save_file = size_file + size_written;
		pthread_mutex_unlock(p_data->pshared->mutex);

		if (p_data->pshared->size_save_file >= size_max_save_file)
		{
			if(Fermeture_ouverture_new_file(p_data)==0)
			{
				printf("%s\n","Bizarre!!!!!!!!!!!!!");
				pthread_mutex_lock(p_data->pshared->mutex);
				p_data->pshared->cmd_acq='s';
				p_data->pshared->ok_record=0;
				pthread_mutex_unlock(p_data->pshared->mutex);
				pthread_exit(NULL);
			}

			fp=p_data->pshared->fp;
		}

		pthread_mutex_lock(p_data->pshared->mutex);
		ok_record=p_data->pshared->ok_record;
		pthread_mutex_unlock(p_data->pshared->mutex);
	}

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	pthread_mutex_lock(p_data->pshared->mutex);
	p_data->pshared->ok_record=0;
	pthread_mutex_unlock(p_data->pshared->mutex);

	pthread_exit(NULL);
}

/********************************************************/
/*Fonction: stats                                       */
/*                                                      */
/********************************************************/
static void stats(struct stUDPSendMeasureType_t *ch1,
		struct stUDPSendMeasureType_t *ch2, int nb_measures)
{
	static time_t new_cycle = 0;
	int i;
	static uint32_t nb_data = 0;
	static double min[NB_CH] = {0}, max[NB_CH] = {0};
	static double sum[NB_CH] = {0}, sum_square[NB_CH] = {0};
	static struct sgf_data data_to_send[2];
	static pthread_t th[2];

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	nb_data += nb_measures;

	if(!new_cycle)
	{
		new_cycle = time(NULL) - SGF_SEND_PERIOD + 10*60;
		max[0] = ch1->fMeasure[0];
		min[0] = ch1->fMeasure[0];
		max[1] = ch2->fMeasure[0];
		min[1] = ch2->fMeasure[0];
	}
	for(i=0;i<nb_measures;i++)
	{
		//Update max and min if needed
		if(ch1->fMeasure[i] > max[0])
			max[0] = ch1->fMeasure[i];
		else if(ch1->fMeasure[i] < min[0])
			min[0] = ch1->fMeasure[i];
		if(ch2->fMeasure[i] > max[1])
			max[1] = ch2->fMeasure[i];
		else if(ch2->fMeasure[i] < min[1])
			min[1] = ch2->fMeasure[i];

		// Add the new values to the sums
		sum[0] += ((double)ch1->fMeasure[i]);
		sum_square[0] += ((double)ch1->fMeasure[i])*((double)ch1->fMeasure[i]);
		sum[1] +=(double) ch2->fMeasure[i];
		sum_square[1] +=(double) ch2->fMeasure[i]*((double)ch2->fMeasure[i]);
	}

	if(difftime(time(NULL), new_cycle) > SGF_SEND_PERIOD)
	{
		printf("Sent WLX2 stats\n");

		// Create the struct with the calculated statistics, 
		// and create the thread to send this struct
		for(i=0;i<NB_CH;i++)
		{
			data_to_send[i].mean = sum[i]/nb_data;
			data_to_send[i].std_dev = sqrt(sum_square[i]/nb_data -
					data_to_send[i].mean * data_to_send[i].mean);
			data_to_send[i].min = min[i];
			data_to_send[i].max = max[i];
			if(i == 0)
				data_to_send[i].id = WLX2_CH1;
			else
				data_to_send[i].id = WLX2_CH2;

			if(alive[SGF] == 1)
				pthread_create(&th[i], &attr, send_sigfox,
						(void*) &data_to_send[i]);
			sum[i] = 0;
			sum_square[i] = 0;
			min[i] = DBL_MAX;
			max[i] = -DBL_MAX;
			printf("Ch%i  --  Moy : %f\t Ecarts : %f\n", i, data_to_send[i].mean,
					data_to_send[i].std_dev);
		}

		nb_data = 0;
		new_cycle = time(NULL);
	}
	pthread_attr_destroy(&attr);
}


/********************************************************/
/*Fonction: Fermeture_ouverture_new_file                */
/*                                                      */
/********************************************************/
int Fermeture_ouverture_new_file(struct param_pgm *param)
{
	if(Close_file_Enregistrement_data(param))
	{
		pthread_mutex_lock(param->pshared->mutex);
		param->pshared->size_save_file=0.0;
		param->pshared->nb_save_file=param->pshared->nb_save_file+1;
		pthread_mutex_unlock(param->pshared->mutex);
	}
	else
	{
		printf("%s\n","Erreur lors de la fermeture du"
			       " fichier de sauvegarde");
		return 0;
	}

	if(Verif_free_space(param,0))
	
		return Open_file_Enregistrement_data(param);
	else
	{
		printf("%s\n","Erreur il n'y a plus assez d'espace"
			       " pour enregistrer les données");
		return 0;
	}
}



/********************************************************/
/*Fonction: Reception_data                              */
/*                                                      */
/********************************************************/
int Reception_data(struct parametres_connexion *param_connection,
	       	struct stUDPSendMeasureType_t *SendMeasureType_t, int ok_print)
{
	int ok=1;

	fd_set select_fds;
	int sock = param_connection->ID_socket_acquisition_data;
	FD_ZERO(&select_fds);
	FD_SET(sock, &select_fds);

	if(select(sock + 1, &select_fds,
				NULL, NULL, NULL) == 0)
	{
		printf("%s\n","Select has timed out ..."); 
		printf("%s\n","Pas de réponse de WLX2");
		return 0;
	}
	else
	{
		recvfrom(param_connection->ID_socket_acquisition_data,
				SendMeasureType_t,
				sizeof(struct stUDPSendMeasureType_t),
				0, NULL, NULL);

		if (SendMeasureType_t->stHeader.ui16DataID != _OPTICAL_MEASURE)
		{
			ok=0;
			printf("%s %d %s %d %s\n",
					"Erreur: DataID de la réponse reçue : ",
					SendMeasureType_t->stHeader.ui16DataID,
					" (", _OPTICAL_MEASURE, " attendu)");
		}
		if (ok_print)
			Print_stUDPSendMeasureType_t(SendMeasureType_t);

	}
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
	return 1;
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


	ui16DataSize = SendMeasureType_t->stHeader.stMeasureHeader.ui16DataSize;
	ui8DataType = SendMeasureType_t->stHeader.stMeasureHeader.ui8DataType;
	ui8ModuleID = SendMeasureType_t->stHeader.stMeasureHeader.ui8ModuleID;
	ui8ChannelID = SendMeasureType_t->stHeader.stMeasureHeader.ui8ChannelID;
	ui8MeasureUnit = SendMeasureType_t->stHeader.stMeasureHeader.ui8MeasureUnit;
	ui16Year = SendMeasureType_t->stHeader.stMeasureHeader.ui16Year;
	ui8Month = SendMeasureType_t->stHeader.stMeasureHeader.ui8Month;
	ui8Day = SendMeasureType_t->stHeader.stMeasureHeader.ui8Day;
	ui8Hour = SendMeasureType_t->stHeader.stMeasureHeader.ui8Hour;
	ui8Min = SendMeasureType_t->stHeader.stMeasureHeader.ui8Min;
	ui8Seconds = SendMeasureType_t->stHeader.stMeasureHeader.ui8Seconds;
	ui810thOfSeconds = SendMeasureType_t->stHeader.stMeasureHeader.ui810thOfSeconds;

	ui16DataID = SendMeasureType_t->stHeader.ui16DataID;
	ui8SegmentID = SendMeasureType_t->stHeader.ui8SegmentID;
	ui8SegmentQty = SendMeasureType_t->stHeader.ui8SegmentQty;

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

	return 1;
}






/********************************************************/
/*Fonction: Convert_stUDPSendMeasureType_t__to__string  */
/*                                                      */
/********************************************************/
void Convert_stUDPSendMeasureType_t__to__string(struct stUDPSendMeasureType_t *SendMeasureType_t, char *string_SendMeasureType_t)
{
	int i;
	uint16_t ui16Year;
	uint8_t ui8Month;
	uint8_t ui8Day;
	uint8_t ui8Hour;
	uint8_t ui8Min;
	uint8_t ui8Seconds;
	uint8_t ui810thOfSeconds;

/* 	For reference, these are not converted :
  	uint8_t ui8ChannelID;
	uint8_t ui8MeasureUnit;
	uint8_t ui8DataType;
	uint16_t ui16DataSize;
	uint8_t ui8ModuleID;
	uint8_t ui8SegmentQty;
	uint8_t ui8SegmentID;
	uint16_t ui16DataID;
*/

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
