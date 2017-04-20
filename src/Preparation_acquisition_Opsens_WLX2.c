#include "Preparation_acquisition_Opsens_WLX2.h" 
#include <dirent.h>
#include <sys/stat.h>
#include "Configuration_Programme_Opsens_WLX2.h"
#include "Connexion_Opsens_WLX2.h"
#include "Fonctions_Utiles.h"
#include "Load_Config_File_Opsens_WLX2.h"
#include "sigfox.h"
#include "Acquisition_Opsens_WLX2.h"

/********************************************************/
/*Fonction: Configuration_WLX2                          */
/*                                                      */
/********************************************************/
int Preparation_acquisition_WLX2(struct param_pgm *pparam_pgm, struct parametres_connexion *pparam_connection, struct config_all *pconfig_all, struct shared *pshared)
{
	Init_struct_param_pgm(pparam_pgm, pparam_connection, pconfig_all, pshared);
	if(Init_repertoire_pour_enregistrement_data(pparam_pgm)
			&& Verif_free_space(pparam_pgm, 1))
		return 1;
	return 0;
}







/********************************************************/
/*Fonction: Init_struct_shared                          */
/*                                                      */
/********************************************************/
int Init_struct_shared(struct shared *pshared, char *chemin, float *ch_zero,float *ch_offset,float *ch_value, pthread_mutex_t *mut)
{
	pshared->size_save_file=0;
	pshared->nb_save_file=0;
	pshared->ch_zero=ch_zero;
	pshared->ch_offset=ch_offset;
	pshared->ch_value=ch_value;
	pshared->size_max_free=0; 
	pshared->chemin=chemin;
	pshared->cmd_acq='p';
	pshared->mutex=mut;
	pshared->thread_enregistrement=0;

	return 1;
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
	return 0;
}






/********************************************************/
/*Fonction: thread_Acquisition_data                    */
/*				                       */
/********************************************************/
void* thread_Acquisition_data (void* arg)
{
	struct param_pgm  *p_data=arg;

	float zero_channel1,offset_channel1,zero_channel2,offset_channel2;

	//setup shared memory
	p_data->pshared->nb_meas_done=0;
	p_data->pshared->offset_modif=0;
	p_data->pshared->ok_record=0;

	if(p_data->pconfig_all->pconfig_meas->zeros[0] == 0)
		Zero_sensor(p_data->pparam_connection,1,1);

	// Setup the zero of both channel, save zero and offset to shared memory
	Get_zero_sensor(p_data->pparam_connection, 1, &zero_channel1);
	Get_zero_sensor(p_data->pparam_connection, 2, &zero_channel2);
	Get_offset_sensor(p_data->pparam_connection, 1, &offset_channel1);
	Get_offset_sensor(p_data->pparam_connection, 2, &offset_channel2);

	// Write new zeros values to the config file
	if(p_data->pconfig_all->pconfig_meas->zeros[0] == 0)
		save_zeros_offset(zero_channel1, zero_channel2);
	p_data->pshared->ch_zero[0] = zero_channel1;
	p_data->pshared->ch_zero[1] = zero_channel2;
	p_data->pshared->ch_offset[0] = offset_channel1;
	p_data->pshared->ch_offset[1] = offset_channel2; 

	printf("Zero 1 : %f\tZero 2 : %f\n", zero_channel1, zero_channel2);

	pthread_mutex_lock(p_data->pshared->mutex);
	pthread_mutex_unlock(p_data->pshared->mutex);
	Open_file_Enregistrement_data(p_data);

	sleep(1);
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
			pthread_exit(NULL);
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
	float size_max_1_file=0.0,res_size_max_free=0.0;
	char chemin[500]={'\0'};

	size_max_1_file=param->pconfig_all->pconfig_save_file->size_max_save_file;

	strcat(chemin,"\"");
	strcat(chemin,param->pshared->chemin);
	strcat(chemin,"\"");

	res_size_max_free=Calcul_free_space(chemin, ok_print);

	param->pshared->size_max_free=res_size_max_free;

	if (res_size_max_free <= size_max_1_file){
		return 0;
	}

	return 1;
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
	char rep_data[1024]="",buffer[26];
	char cwd[1024];
	struct tm* tm_info;
	time_t timer;

	time(&timer);
	tm_info=localtime(&timer);
	strftime(buffer, 26,"%d_%m_%Y",tm_info);

	if (param->pconfig_all->pconfig_save_file->usb)
	{
		strcpy(rep_data,param->pconfig_all->pconfig_save_file->rep_usb);strcat(rep_data,"/Data");
	}else{
		getcwd(cwd,sizeof(cwd));
		strcpy(rep_data,cwd);
		strcat(rep_data,"/Data");
	}

	rep=opendir(rep_data);
	if (rep == NULL) 
		mkdir(rep_data,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	else{
		if (closedir(rep)==-1) 
		{
			printf("Failed to close %s\n", rep_data);
			return 0;
		}
	}


	strcat(rep_data,"/");
	strcat(rep_data,param->pconfig_all->pconfig_save_file->nom_projet);

	rep=opendir(rep_data);
	if (rep == NULL) 
	{
		mkdir(rep_data, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}else{
		if (closedir(rep)==-1) 
		{
			printf("Failed to close %s\n", rep_data);
			return 0;
		}
	}


	strcat(rep_data,"/Data__");strcat(rep_data,buffer);


	rep=opendir(rep_data);
	if (rep == NULL) 
	{
		mkdir(rep_data,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	}else{
		if (closedir(rep)==-1) 
		{
			printf("Failed to close %s\n", rep_data);
			return 0;
		}
	}

	memcpy(param->pshared->chemin,rep_data,(strlen(rep_data)+1)*sizeof(char));

	Find_dernier_enregistrement(param);

	return 1;
}








/********************************************************/
/*Fonction: Find_dernier_enregistrement                 */
/*                                                      */ 
/********************************************************/
void Find_dernier_enregistrement(struct param_pgm *param)
{
	DIR *rep=NULL;
	int i,j,nb_file_save;
	int find;
	char *rep_data, *nomfile_data;
	char *tmp_string=NULL,*tmp2_string=NULL,*tmp3_string=NULL;
	int tmp_string_len,tmp2_string_len,tmp3_string_len;
	struct dirent* entry=NULL;

	rep_data=param->pshared->chemin;
	nomfile_data=param->pconfig_all->pconfig_save_file->nomfic;


	rep=opendir(rep_data);
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


