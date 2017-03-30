#include "Preparation_acquisition_Opsens_WLX2.h" 

/********************************************************/
/*Fonction: Configuration_WLX2                          */
/*                                                      */
/********************************************************/
int Preparation_acquisition_WLX2(struct param_pgm *pparam_pgm, struct parametres_connexion *pparam_connection, struct config_all *pconfig_all, struct shared *pshared)
{
	int ok=1;
	printf("1");
	Init_struct_param_pgm(pparam_pgm, pparam_connection, pconfig_all, pshared);
	printf("2\n");
	ok=Init_repertoire_pour_enregistrement_data(pparam_pgm);

	printf("end prepa acquisition");
	if(ok)
	{
		ok=Verif_free_space(pparam_pgm,1);
	}
	printf("end prepa acquisition");
	return ok;
}







/********************************************************/
/*Fonction: Init_struct_shared                          */
/*                                                      */
/********************************************************/
int Init_struct_shared(struct shared *pshared, char *chemin, float *ch_zero,float *ch_offset,float *ch_value, pthread_mutex_t *mutex)
{
	int ok;
	pshared->size_save_file=0;
	pshared->nb_save_file=0;
	pshared->ch_zero=ch_zero;
	pshared->ch_offset=ch_offset;
	pshared->ch_value=ch_value;
	pshared->size_max_free=0; 
	pshared->chemin=chemin;
	pshared->cmd_acq='p';
	pshared->mutex=mutex;
	pshared->thread_enregistrement=0;



	if(pthread_mutex_init (pshared->mutex, NULL)==0){ok=1;}else{ok=0;};




	return ok;
}



/********************************************************/
/*Fonction: Init_struct_param_pgm                       */
/*                                                      */
/********************************************************/
void Init_struct_param_pgm(struct param_pgm *pparam_pgm, struct parametres_connexion *pparam_connection, struct config_all *pconfig_all, struct shared *pshared)
{
	pparam_pgm->pparam_connection=pparam_connection;
	pparam_pgm->pconfig_all=pconfig_all;
	pparam_pgm->pshared=pshared;
}


/********************************************************/
/*Fonction: Lancement_thread_acquistion                 */
/*                                                      */
/********************************************************/
int Lancement_thread_acquistion(struct param_pgm *param)
{
	pthread_t xthread_Acquisition_data;

	if (!pthread_create(&xthread_Acquisition_data, NULL,
				thread_Acquisition_data, param) 
		&& !pthread_join(xthread_Acquisition_data,NULL)) 
		return 1;
	return -1;
}








/********************************************************/
/*Fonction: thread_Acquisition_data                    */
/*				                       */
/********************************************************/
void* thread_Acquisition_data (void* arg)
{
	struct param_pgm  *p_data=arg;
	struct tm* tm_info;
	time_t timer;

	float zero_channel,offset_channel,zero_channel1,offset_channel1,zero_channel2,offset_channel2;
	char cmd_receive,cmd_receive_before,buffer[26];
	char reponse_state[20]="State:-201";

	char answer_2[2][_RECEIVE_BUFF_SIZE]={'\0','\0'};

	Send_command_and_receive_answer_2(p_data->pparam_connection,
		       	"MEASure:RUN?", 2 ,answer_2, 0);
	if (answer_2[0][0]!='0')
	{
		Zero_str(answer_2[0]);Zero_str(answer_2[1]);
		Send_command_and_receive_answer_2(p_data->pparam_connection,
				"MEASure:STOP", 2 ,answer_2, 0);
	}

	// Code a modifier pour retourner qqc quand l'interrogateur n'est pas pret
	/*while (Find_substr_in_str(answer_2[0],reponse_state)==1)
	{
		getchar(); 

		if(Close_socket(p_data->pparam_connection->ID_socket_command) && 
				!sleep(5) && 
				Init_param_connexion(p_data->pparam_connection))
		{
			sleep(5);
			Zero_str(answer_2[0]);
			Send_command_and_receive_answer_2(
					p_data->pparam_connection,
					"MEASure:RUN?", 2 ,answer_2, 0);
		}else{
			goto fin_thread_Acquisition_data;
		}
	}*/

	//setup shared memory
	p_data->pshared->nb_meas_done=0;
	p_data->pshared->offset_modif=0;
	p_data->pshared->ok_record=0;

	Open_file_Enregistrement_data(p_data);

	// Setup the zero of both channel, save zero and offset to shared memory
	Get_zero_sensor(p_data->pparam_connection, 1, 
			&zero_channel1);
	Get_zero_sensor(p_data->pparam_connection, 2, 
			&zero_channel2);
	Get_offset_sensor(p_data->pparam_connection, 1, 
			&offset_channel1);
	Get_offset_sensor(p_data->pparam_connection, 2, 
			&offset_channel2);

	p_data->pshared->ch_zero[0]=zero_channel1;
	p_data->pshared->ch_zero[1]=zero_channel2;
	p_data->pshared->ch_offset[0]=offset_channel1;
	p_data->pshared->ch_offset[1]=offset_channel2; 

	// Debut de 'c'
	if(Run_Thread_Enregistrement_data(p_data))
	{
		pthread_mutex_lock(p_data->pshared->mutex);
		p_data->pshared->ok_record=1;
		pthread_mutex_unlock(p_data->pshared->mutex);

		if (!Measure_start_infinite(p_data->pparam_connection))
		{
			pthread_mutex_lock(p_data->pshared->mutex);
			p_data->pshared->ok_record=0;
			p_data->pshared->cmd_acq='s';
			pthread_mutex_unlock(p_data->pshared->mutex);
			goto fin_thread_Acquisition_data;
		}
	}
	else
	{
		pthread_mutex_lock(p_data->pshared->mutex);
		p_data->pshared->ok_record=0;
		pthread_mutex_unlock(p_data->pshared->mutex); 
	}
	while (!end_program)//check main program end condition	
	{sleep(10);}

	pthread_mutex_lock(p_data->pshared->mutex);
	p_data->pshared->ok_record=0;
	pthread_mutex_unlock(p_data->pshared->mutex);

	Stop_Thread_Enregistrement_data(p_data);

fin_thread_Acquisition_data:

	pthread_exit(NULL);
}









/********************************************************/
/*Fonction: thread_Wait_Command                         */
/*				                                              */
/********************************************************/
void* thread_Wait_Command (void* arg)
{
	char cmd_user,cmd_user_send;
	struct param_pgm  *p_data=arg;


	fd_set readfds;
	int num_readable;
	struct timeval tv;
	int fd_stdin;
	int num_bytes;
	int MAXBYTES=80;
	char buf[MAXBYTES];

	fd_stdin = fileno(stdin);

	cmd_user=p_data->pshared->cmd_acq;
	cmd_user_send=cmd_user;
	while (cmd_user != 's')
	{
		FD_ZERO(&readfds);
		FD_SET(fileno(stdin), &readfds);

		tv.tv_sec = 1;
		tv.tv_usec = 0;

		//fscanf(stdin," %c",&cmd_user_send);


		while(p_data->pshared->offset_modif==1)
		{
			sleep(1);
		}

		num_readable = select(fd_stdin + 1, &readfds, NULL, NULL, &tv);
		if (num_readable == -1) {
			break;
		}
		if (num_readable == 0) {
			cmd_user_send=p_data->pshared->cmd_acq;
		} else {
			num_bytes = read(fd_stdin, buf, MAXBYTES);
			cmd_user_send=buf[0];
			//fflush(stdin);
			if (num_bytes < 0) {
				break;
			}
		}



		//if ((cmd_user_send !='c')&& (cmd_user_send !='p')&&(cmd_user_send !='i')&&(cmd_user_send !='s')&& (cmd_user_send !='0')&&(cmd_user_send !='1')&&(cmd_user_send !='2')){cmd_user_send ='c';}
		//if (cmd_user_send!=cmd_user)
		if ((cmd_user_send =='o')||(cmd_user_send =='c')||(cmd_user_send =='p')||(cmd_user_send =='i')||(cmd_user_send =='s')||(cmd_user_send =='0')||(cmd_user_send =='1')||(cmd_user_send =='2'))
		{
			pthread_mutex_lock(p_data->pshared->mutex);
			p_data->pshared->cmd_acq=cmd_user_send;
			cmd_user=cmd_user_send;
			pthread_mutex_unlock(p_data->pshared->mutex);
		}else{
		}


		//usleep(1000000);
	}



	pthread_exit(NULL);
}






/********************************************************/
/*Fonction: Verif_free_space          */      
/*                                                     */
/********************************************************/
int Verif_free_space(struct param_pgm *param, int ok_print)
{
	int ok=1,nb_file_enregistrable=0;
	float size_max_1_file=0.0,res_size_max_free=0.0;
	char chemin[500]={'\0'};

	size_max_1_file=param->pconfig_all->pconfig_save_file->size_max_save_file;

	strcat(chemin,"\"");
	strcat(chemin,param->pshared->chemin);
	strcat(chemin,"\"");

	/*
	   if (param->pconfig_all->pconfig_save_file->usb)
	   {
	   strcat(chemin,"\"");
	   strcat(chemin,param->pconfig_all->pconfig_save_file->rep_usb);
	   strcat(chemin,"\"");
	   }else{
	   strcat(chemin,"/home/pi");
	   */

	res_size_max_free=Calcul_free_space(chemin, ok_print);

	param->pshared->size_max_free=res_size_max_free;

	if (res_size_max_free <= size_max_1_file){
		ok=0;
	}else{
		if(ok_print)
		{
			nb_file_enregistrable=(int)(res_size_max_free/size_max_1_file);
		}
	}

	return ok;
}






/********************************************************/
/*Fonction: Envoi_SMS_alert                             */
/*                                                      */
/********************************************************/
void Envoi_SMS_alert()
{
	FILE * pp;

	if ((pp = popen("python2.7 ../Include/Script__Envoi_alert_SMS_Opsens.py", "r")) != NULL)
	{
		pclose(pp);
	}
}




/********************************************************/
/*Fonction: Init_repertoire_pour_enregistrement_data    */
/*                                                      */ 
/********************************************************/
int Init_repertoire_pour_enregistrement_data(struct param_pgm *param)
{
	DIR *rep=NULL;
	int ok=1,tmp_string_len,ok_usb;
	int file_save_exist,nb_file_save;
	char rep_data[1024]="",buffer[26];
	char cwd[1024],tempchar[1024];
	struct dirent* entry=NULL;
	struct stat stabuf;
	struct tm* tm_info;
	time_t timer;

	time(&timer);
	tm_info=localtime(&timer);
	strftime(buffer, 26,"%d_%m_%Y",tm_info);

	printf("1\n");
	ok_usb=param->pconfig_all->pconfig_save_file->usb;
	if (ok_usb)
	{
		strcpy(rep_data,param->pconfig_all->pconfig_save_file->rep_usb);strcat(rep_data,"/Data");
	}else{
		getcwd(cwd,sizeof(cwd));
		strcpy(rep_data,cwd);strcat(rep_data,"/Data");
	}

	printf("2\n");
	rep=opendir(rep_data);
	if (rep == NULL) 
	{
		mkdir(rep_data,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}else{
		if (closedir(rep)==-1) exit(-1);
	}


	printf("3\n");
	strcat(rep_data,"/");strcat(rep_data,param->pconfig_all->pconfig_save_file->nom_projet);

	rep=opendir(rep_data);
	if (rep == NULL) 
	{
		mkdir(rep_data,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}else{
		if (closedir(rep)==-1) {ok=0;};
	}

	printf("4\n");

	strcat(rep_data,"/Data__");strcat(rep_data,buffer);


	rep=opendir(rep_data);
	if (rep == NULL) 
	{
		mkdir(rep_data,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}else{
		if (closedir(rep)==-1) {ok=0;};
	}


	printf("5\n");
	memcpy(param->pshared->chemin,rep_data,(strlen(rep_data)+1)*sizeof(char));

	//nb_file_save=find_last_file_save(rep_data, nomfile_save);
	Find_dernier_enregistrement(param);

	printf("6\n");
	//return nb_file_save;
	return ok;
}








/********************************************************/
/*Fonction: Find_dernier_enregistrement                 */
/*                                                      */ 
/********************************************************/
void Find_dernier_enregistrement(struct param_pgm *param)
{
	DIR *rep=NULL;
	int i,j,file_save_exist,nb_file_save;
	int find;
	char *rep_data, *nomfile_data;
	char *tmp_string=NULL,*tmp2_string=NULL,*tmp3_string=NULL;
	int tmp_string_len,tmp2_string_len,tmp3_string_len,ch='_';
	struct dirent* entry=NULL;

	rep_data=param->pshared->chemin;
	nomfile_data=param->pconfig_all->pconfig_save_file->nomfic;


	rep=opendir(rep_data);
	file_save_exist=0;
	nb_file_save=0;

	while((entry=readdir(rep))!= NULL)
	{
		if (strcmp(".",entry->d_name) == 0 || strcmp("..",entry->d_name) == 0) continue;
		tmp_string=malloc((strlen(entry->d_name)+1)*sizeof(char));
		for(i=0;i<strlen(entry->d_name);i++)
		{
			tmp_string[i]=entry->d_name[i];
		}
		tmp_string[i]='\0';

		//getchar();
		tmp_string_len=strlen(tmp_string);
		if (tmp_string_len>=1)
		{
			find=0;

			i=tmp_string_len;
			while((tmp_string[i]!='_')&&((i-1)>=0))
			{
				i=i-1;
			}


			if (i>0)
			{
				if(tmp_string[i-1]=='_') 
				{
					tmp2_string_len=i-1;
					tmp2_string=malloc((tmp2_string_len+1)*sizeof(char));
					for(j=0;j<tmp2_string_len;j++)
					{
						tmp2_string[j]=tmp_string[j];
					}
					tmp2_string[j]='\0';

					if (strcmp(nomfile_data,tmp2_string) == 0)
					{
						tmp3_string_len=tmp_string_len-i;

						tmp3_string=malloc((tmp3_string_len+1)*sizeof(char));
						for(j=0;j<tmp3_string_len;j++)
						{
							tmp3_string[j]=tmp_string[i+j+1];
						}
						tmp3_string[j]='\0';
						find=atoi(tmp3_string);
						if (find>nb_file_save) {nb_file_save=find;}
						tmp3_string=NULL;
						free(tmp3_string);
					}
					find=1;
					tmp2_string=NULL;
					free(tmp2_string);
				}
			}
		}

		tmp_string=NULL;
		free(tmp_string);
		//getchar();
	}
	if (closedir(rep)==-1) {exit(-1);}

	param->pshared->nb_save_file=nb_file_save;
	//return nb_file_save;
}


