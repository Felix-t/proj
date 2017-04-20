
#define __USE_XOPEN
#define _GNU_SOURCE

#include "Configuration_Module_Opsens_WLX2.h" 
#include <regex.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>

/********************************************************/
/*Fonction: Configuration_WLX2                          */
/*                                                      */
/********************************************************/
int Configuration_WLX2(struct parametres_connexion *param_connection, struct config_all *pconfig_all)
{
	int ok;


	ok=Configuration_WLX2_SAMPLingrate_MEASureRATE(param_connection, pconfig_all);



	if (ok)
	{
		ok=Configuration_WLX2_channel(param_connection, pconfig_all);
	}

	//	if (ok)
	//	{
	//		ok=Configuration_WLX2_date_time(param_connection);;
	//	}


	return ok;
}





/********************************************************/
/*Fonction: Configuration_WLX2_SAMPLingrate_MEASureRATE */
/*                                                      */
/********************************************************/
int Configuration_WLX2_SAMPLingrate_MEASureRATE(struct parametres_connexion *param_connection, struct config_all *pconfig_all)
{
	int ok=1;

	sleep(1);

	printf("\t%s\n","->Paramétrage de la fréquence à laquelle les mesures sont faites au niveau de chaque canal  ");
	if(Set_SAMPLingrate(param_connection, pconfig_all)==0)
	{
		ok=0;
		printf("\t%s\n", "...Echec");
		goto fin_Configuration_WLX2_SAMPLingrate_MEASureRATE;
	}

	sleep(1);

	printf("\t%s\n", "...ok");
	printf("\t%s\n","->Paramétrage de la fréquence à laquelle les mesures sont transmises de la carte vers le PC sur le lien Ethernet  ");
	if(Set_MEASureRATE(param_connection, pconfig_all)==0)
	{
		ok=0;
		printf("\t%s\n", "...Echec");
		goto fin_Configuration_WLX2_SAMPLingrate_MEASureRATE;
	}


	printf("\t%s\n", "...ok");

fin_Configuration_WLX2_SAMPLingrate_MEASureRATE:;

						return ok;
}



/********************************************************/
/*Fonction: Set_SAMPLingrate                           */
/*                                                      */
/********************************************************/
int Set_SAMPLingrate(struct parametres_connexion *param_connection, struct config_all *pconfig_all)
{
	int ok,nb_answer=NB_CH;
	char prefixe_command[100]="SYSTem:SAMPlingrate ";
	char command[100]={'\0'};
	int SAMPLingrate;
	char wait_reponse[100]={'\0'};
	char answer_Get_SAMPLingrate[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};


	SAMPLingrate=pconfig_all->pconfig_meas->SAMPLingrate;
	sprintf(wait_reponse, "%d", SAMPLingrate);

	strcat(command,prefixe_command);
	strcat(command,wait_reponse); 

	Get_SAMPLingrate(param_connection,answer_Get_SAMPLingrate);
	printf("\tSAMPLingrate: %s",answer_Get_SAMPLingrate);

	ok=1;
	if(Compare_2str(answer_Get_SAMPLingrate,wait_reponse)==0)
	{
		Zero_str(answer_Get_SAMPLingrate);
		printf("\t%s\t ","-- Modification -->");
		printf("SAMPLingrate: %s\n",wait_reponse);
		if(Send_command_and_receive_answer_2(param_connection,command, nb_answer,answer_2, 0))
		{  	
			//  printf("***---%s %s\n",answer_2[0],answer_2[1]);
			//  if(Compare_2str(reponse_ok, answer_2[0])==0){ok=0;goto ok_0;} 
			if(Compare_2str(wait_reponse, answer_2[0])==0){ok=0;goto ok_0;} 
		}else{ok=0;printf("%s %s %s\n","Echec: La commande ",command," n'a pas été prise en compte par WLX-2");goto ok_0;}

		printf("\t%s", "Vérification de la configuration de SAMPLingrate  ");


		Close_socket(param_connection->ID_socket_command);
		sleep(5);
		Init_param_connexion(param_connection);





		sleep(10);

		Zero_str(answer_Get_SAMPLingrate);
		Get_SAMPLingrate(param_connection,answer_Get_SAMPLingrate);
		printf("SAMPLingrate: %s",answer_Get_SAMPLingrate);
		if(Compare_2str(answer_Get_SAMPLingrate,wait_reponse)==0){ok=0;}else{printf("\t%s\n","... ok");}
	}

	//printf("%s\t","");

ok_0:;





     return ok;
}


/********************************************************/
/*Fonction: Get_SAMPLingrate                             */
/*                                                      */
/********************************************************/
int Get_SAMPLingrate(struct parametres_connexion *param_connection, char *answer)
{
	int nb_answer=NB_CH;
	char command[100]="SYSTem:SAMPlingrate?";
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};

	Zero_str(answer);


	if (!Send_command_and_receive_answer_2(param_connection,
				command, nb_answer,answer_2, 0))
		return 0;

	memcpy(answer,answer_2[0],strlen(answer_2[0])*sizeof(char));

	if(!Verification_response_from_device_is_err(answer))
		return 0;	

	return 1;
}





/********************************************************/
/*Fonction: Set_MEASureRATE                           */
/*                                                      */
/********************************************************/
int Set_MEASureRATE(struct parametres_connexion *param_connection, struct config_all *pconfig_all)
{
	char prefixe_command[100]="SYSTem:MEASure:RATE ";
	char command[100]={'\0'};
	int MEASureRATE;
	char wait_reponse[100]={'\0'};
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};
	char answer_Get_MEASureRATE[100]={'\0'};

	MEASureRATE=pconfig_all->pconfig_meas->MEASureRATE;
	sprintf(wait_reponse, "%d", MEASureRATE);
	
	strcat(command,prefixe_command);
	strcat(command,wait_reponse); 

	Get_MEASureRATE(param_connection,answer_Get_MEASureRATE);
	printf("\tMEASureRATE: %s",answer_Get_MEASureRATE);

	if(Compare_2str(answer_Get_MEASureRATE,wait_reponse)==0)
	{
		Zero_str(answer_Get_MEASureRATE);

		printf("\t%s\t ","-- Modification -->");
		printf("MEASureRATE: %s\n",wait_reponse);

		if(!Send_command_and_receive_answer_2(param_connection,command,
					NB_CH,answer_2, 0))
		{  	
			printf("\tEchec: La commande %s n'a pas été "
				       "prise en compte par WLX-2\n",command);
			return 0;
		}
		if(!Compare_2str(wait_reponse, answer_2[0]))
			return 0;

		sleep(1);

		printf("\t%s", "Vérification de la configuration de ");

		Zero_str(answer_Get_MEASureRATE);
		Get_MEASureRATE(param_connection,answer_Get_MEASureRATE);
		printf("MEASureRATE: %s",answer_Get_MEASureRATE);

		if(!Compare_2str(answer_Get_MEASureRATE,wait_reponse))
			return 0;

		printf("\t... ok\n");
	}

     return 1;
}


/********************************************************/
/*Fonction: Get_MEASureRATE                             */
/*                                                      */
/********************************************************/
int Get_MEASureRATE(struct parametres_connexion *param_connection, char *answer)
{
	char command[100]="SYSTem:MEASure:RATE?";
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};
	
	Zero_str(answer);
	if(!Send_command_and_receive_answer_2(param_connection,command,
			       NB_CH,answer_2, 0))
		return 0;

	memcpy(answer,answer_2[0],strlen(answer_2[0])*sizeof(char));

	if(!Verification_response_from_device_is_err(answer))
		return 0;

	return 1;
}




/********************************************************/
/*Fonction: Configuration_WLX2_channel                  */
/*                                                      */
/********************************************************/
int Configuration_WLX2_channel(struct parametres_connexion *param_connection, struct config_all *pconfig_all)
{
	int ok=1;

	sleep(1);

	//printf("\t%s\n","->Suppression de la configuration \"capteur\" précédente : ");
	//	if(Delete_all_sensor(param_connection)==0)
	//	{
	//		ok=0;
	//		printf("\t%s\n", "...Echec");
	//		goto fin_Configuration_WLX2_channel;
	//	}
	//
	//	sleep(1);
	//
	//	printf("\t%s\n", "...ok");
	//	printf("\t%s\n","->Ajout des nouveaux capteurs : ");
	//	if(Add_sensors(param_connection, pconfig_all)==0)
	//	{
	//		ok=0;
	//		printf("\t%s\n", "...Echec");
	//		goto fin_Configuration_WLX2_channel;
	//	}
	//
	//	sleep(1);
	//
	//	printf("\t%s\n", "...ok");
	//	printf("\t%s\n","->Association des capteurs aux cannaux de mesure: ");
	//	if(Associate_sensors_to_channel(param_connection, pconfig_all)==0)
	//	{
	//		ok=0;
	//		printf("\t%s\n", "...Echec");
	//		goto fin_Configuration_WLX2_channel;
	//	}
	//
	//	sleep(1);
	//
	//	printf("\t%s\n", "...ok");
	//	printf("\t%s\n","->Activation des cannaux de mesure: ");
	//	if(Activation_channels(param_connection, pconfig_all)==0)
	//	{
	//		ok=0;
	//		printf("\t%s\n", "...Echec");
	//		goto fin_Configuration_WLX2_channel;
	//	}
	//
	//	sleep(1);
	//
	printf("\t%s\n", "...ok");



	//Verif_liste_sensors(param_connection);

	//fin_Configuration_WLX2_channel:;

	return ok;
}



/********************************************************/
/*Fonction: Get_sensors_list                            */
/*                                                      */
/********************************************************/
int Get_sensors_list(struct parametres_connexion *param_connection, char *answer)
{
	int ok,nb_answer=NB_CH;

	char command[100]="SENSor:LIST?";
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};

	//printf("%s %s\n","Command to the instrument: ", command);
	Zero_str(answer);
	//printf("%s %s\n","Response of the instrument: ", answer);
	//ok=Send_command_and_receive_answer(param_connection,command, answer, 0);
	ok=Send_command_and_receive_answer_2(param_connection,command, nb_answer,answer_2, 0);

	if (ok)
	{
		if(Verification_response_from_device_is_err(answer_2[0])==0);
		{
			ok=0;
		}
	}


	strcat(answer,"Sensor list: \n");strcat(answer,answer_2[0]);strcat(answer,"\n");
	//strcat(answer,"Canal 2: ");strcat(answer,answer_2[1]);strcat(answer,"\n");

	return ok;
}



/********************************************************/
/*Fonction: Analyse_get_sensors_list                    */
/*                                                      */
/********************************************************/
int Analyse_get_sensors_list(char *str_sensor_list, struct sensor_def sensor_list[2])
{
	int i,j,i_nb_sensor = 0,nb_sensor=2;
	char str[100]={'\0'},str_2[2][100]={"\0","\0"};
	char *str_p, *token_tab_h, *token_cr;

	strcat(str,str_sensor_list);

	str_p=strstr(str,"\n");
	Zero_str(str);
	strcat(str,str_p);

	token_cr=strtok(str,"\r");

	while(token_cr != NULL)
	{
		i_nb_sensor++;
		strcat(str_2[i_nb_sensor-1],token_cr);
		token_cr=strtok(NULL,"\r");
		if(i_nb_sensor>=nb_sensor){break;}
	}

	for (i=0;i<nb_sensor;i++)
	{
		token_tab_h=strtok(str_2[i],"\t");
		j=-1;
		while(token_tab_h!= NULL)
		{
			if(j==0){strcat(sensor_list[i].sensor_type,token_tab_h);}
			if(j==1){sensor_list[i].sensor_GF0=atoi(token_tab_h);}
			if(j==2){sensor_list[i].sensor_GF1=atoi(token_tab_h);}
			if(j==3){sensor_list[i].sensor_GF2=atoi(token_tab_h);}
			if(j==4){sensor_list[i].sensor_GF3=atoi(token_tab_h);}
			j++;

			token_tab_h=strtok(NULL,"\t");
		}
	}
	return 1;
}



/********************************************************/
/*Fonction: Delete_all_sensor                           */
/*                                                      */
/********************************************************/
int Delete_all_sensor(struct parametres_connexion *param_connection)
{
	char command[100]="SENSor:DELete:ALL";
	char reponse_ok[3]="ok", answer[100];
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};

	Zero_str(answer);

	if(!Send_command_and_receive_answer_2(param_connection,command, 
				NB_CH,answer_2, 0))
		return 0;

	if(!Compare_2str(reponse_ok, answer_2[0]))
		return Verification_response_from_device_is_err(answer_2[0]);

	return 1;
}


/********************************************************/
/*Fonction: Add_sensor                                  */
/*                                                      */
/********************************************************/
int Add_sensors(struct parametres_connexion *param_connection, struct config_all *pconfig_all)
{
	int ok,i,j,nb_answer=NB_CH;
	char prefixe_command[100]="SENSor:ADD ";
	char command[100]={'\0'};
	char temp_str[100]={'\0'};
	char reponse_ok[3]="ok", answer[100];
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};
	char GF0[10],GF1[10],GF2[10],GF3[10];
	char type_jauge[10];
	char temp2_str[100]={'\0'};

	struct sensor_def sensor_list[2];


	for(i=1;i<=NB_CH;i++)
	{

		j=i-1;
		Zero_str(command);Zero_str(type_jauge);Zero_str(temp_str);
		Zero_str(GF0);Zero_str(GF1);Zero_str(GF2);Zero_str(GF3);

		strcat(command,prefixe_command);
		sprintf(temp_str, "%02d", i);
		strcat(command,temp_str); 
		strcat(command,", ");

		memcpy(type_jauge,pconfig_all->pconfig_meas->type_jauge_ch[j],strlen(pconfig_all->pconfig_meas->type_jauge_ch[j])*sizeof(char));
		strcat(command,type_jauge);
		strcat(command,", ");

		sprintf(GF0, "%d", pconfig_all->pconfig_meas->GFx_jauge_ch[j][0]);
		sprintf(GF1, "%d", pconfig_all->pconfig_meas->GFx_jauge_ch[j][1]);
		sprintf(GF2, "%d", pconfig_all->pconfig_meas->GFx_jauge_ch[j][2]);
		sprintf(GF3, "%d", pconfig_all->pconfig_meas->GFx_jauge_ch[j][3]);

		strcat(command,GF0);strcat(command,", ");
		strcat(command,GF1);strcat(command,", ");
		strcat(command,GF2);strcat(command,", ");
		strcat(command,GF3);strcat(command,"\0");


		Zero_str(temp2_str);
		strcat(temp2_str,"GF0=");strcat(temp2_str,GF0);strcat(temp2_str," ");
		strcat(temp2_str,"GF1=");strcat(temp2_str,GF1);strcat(temp2_str," ");
		strcat(temp2_str,"GF2=");strcat(temp2_str,GF2);strcat(temp2_str," ");
		strcat(temp2_str,"GF3=");strcat(temp2_str,GF3);



		printf("\t%s %d %s %s %s %s\n","capteur ",i," : ",pconfig_all->pconfig_meas->type_jauge_ch[j],pconfig_all->pconfig_meas->numero_jauge_ch[j],temp2_str);

		Zero_str(answer);

		ok=Send_command_and_receive_answer_2(param_connection,command, nb_answer,answer_2, 0);

		if(ok)
		{
			ok=Compare_2str(reponse_ok, answer_2[0]);
		}else{
			ok=0;
			break;
		}

	}


	printf("\t%s", "Vérification de la configuration des capteurs ");

	Get_sensors_list(param_connection, answer);
	//printf("%s\n",answer);

	Analyse_get_sensors_list(answer, sensor_list);

	for (i=0;i<2;i++)
	{
		//printf("%s %d %d %d %d\n",sensor_list[i].sensor_type,sensor_list[i].sensor_GF0,sensor_list[i].sensor_GF1,sensor_list[i].sensor_GF2,sensor_list[i].sensor_GF3);

		ok=1;
		ok=ok*Compare_2str(pconfig_all->pconfig_meas->type_jauge_ch[i],sensor_list[i].sensor_type);
		if (pconfig_all->pconfig_meas->GFx_jauge_ch[i][0]!=sensor_list[i].sensor_GF0){ok=0;}
		if (pconfig_all->pconfig_meas->GFx_jauge_ch[i][1]!=sensor_list[i].sensor_GF1){ok=0;}
		if (pconfig_all->pconfig_meas->GFx_jauge_ch[i][2]!=sensor_list[i].sensor_GF2){ok=0;}
		if (pconfig_all->pconfig_meas->GFx_jauge_ch[i][3]!=sensor_list[i].sensor_GF3){ok=0;}
		//printf("OK %d\n",ok);

	}

	if (ok){printf("\t%s\n","...ok");}else{printf("\t%s\n","...error");}


	return ok;
}




/********************************************************/
/*Fonction: Get_associate_sensors_to_channel                */
/*                                                      */
/********************************************************/
int Get_associate_sensors_to_channel(struct parametres_connexion *param_connection, struct config_all *pconfig_all, int number_channel)
{
	int response_sensor_used;
	char prefixe_command[100]="CHannel"; char suffixe_command[100]=":SENSor?";
	char command[100];
	char temp_str[100];
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};


	Zero_str(command);
	strcat(command,prefixe_command);
	sprintf(temp_str, "%d", number_channel);
	strcat(command,temp_str); 
	strcat(command,suffixe_command);


	if(Send_command_and_receive_answer_2(param_connection,command,
			NB_CH,answer_2, 0))
	{
		if (number_channel==1) 
			response_sensor_used=atoi(answer_2[0]);
		if (number_channel==2) 
			response_sensor_used=atoi(answer_2[1]);
	}
	else{
		response_sensor_used=0;
	}

	return response_sensor_used;
}




/********************************************************/
/*Fonction: Associate_sensors_to_channel                */
/*                                                      */
/********************************************************/
int Associate_sensors_to_channel(struct parametres_connexion *param_connection, struct config_all *pconfig_all)
{
	int ok,i,nb_answer=NB_CH;
	int response_get_associate_sensors_to_channel;
	char prefixe_command[100]="CHannel";
	char command[100];
	char temp_str[100];
	char reponse_ok[3]="ok", answer[100];
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};

	/*
	   printf("%s\n"," ");
	   const char command5a[_UDP_CMD_MAX_LEN]="SENSor:LIST?";
	   char answer5a[_RECEIVE_BUFF_SIZE];

	   Send_command_and_receive_answer(param_connection, command5a, answer5a, 1);
	   */


	for(i=1;i<=NB_CH;i++)
	{
		Zero_str(temp_str);Zero_str(command);
		strcat(command,prefixe_command);
		sprintf(temp_str, "%d", i);
		strcat(command,temp_str); 
		strcat(command,":SENSOR ");
		strcat(command,temp_str);


		printf("\tcanal  %d  -> capteur  %d \n", i, i);

		Zero_str(answer);
		ok=Send_command_and_receive_answer_2(param_connection,command, nb_answer,answer_2, 0);
		if(ok)
		{
			ok=Compare_2str(reponse_ok, answer_2[0]);

		}else{printf("%s\n","... error");}
	}


	if (ok)
	{
		printf("\t%s\t","Vérification de l'association des capteurs aux cannaux de mesure ");

		for(i=1;i<=NB_CH;i++)
		{
			response_get_associate_sensors_to_channel=Get_associate_sensors_to_channel(param_connection,pconfig_all,i);
			if (response_get_associate_sensors_to_channel!=i){printf("%s\n","... error");ok=0;break;}
		}
		if (ok){printf("%s\n","... ok");}
	}



	return ok;
}





/********************************************************/
/*Fonction: Get_status_channel                          */
/*                                                      */
/********************************************************/
int Get_status_channel(struct parametres_connexion *param_connection, struct config_all *pconfig_all, int number_channel)
{
	int response_sensor_used;
	char prefixe_command[100]="CHannel"; char suffixe_command[100]=":ENABle?";
	char command[100];
	char temp_str[100];
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};


	Zero_str(command);
	strcat(command,prefixe_command);
	sprintf(temp_str, "%d", number_channel);
	strcat(command,temp_str); 
	strcat(command,suffixe_command);

	if(Send_command_and_receive_answer_2(param_connection,command,
				NB_CH,answer_2, 0))
	{
		if (number_channel==1) 
			response_sensor_used=atoi(answer_2[0]);
		if (number_channel==2) 
			response_sensor_used=atoi(answer_2[1]);
	}
	else
		response_sensor_used=0;

	return response_sensor_used;
}





/********************************************************/
/*Fonction: Desactivation_channel                          */
/*                                                      */
/********************************************************/
int Desactivation_channel(struct parametres_connexion *param_connection, int i_channel)
{
	char prefixe_command[100]="CHannel";
	char command[100];
	char temp_str[100];
	char reponse_ok[3]="ok", answer[100];
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};

	Zero_str(temp_str);Zero_str(command);

	strcat(command,prefixe_command);
	sprintf(temp_str, "%d", i_channel);
	strcat(command,temp_str); 
	strcat(command,":DISAble ");

	Zero_str(answer);

	if(Send_command_and_receive_answer_2(param_connection,command,
			NB_CH,answer_2, 0))
		return Compare_2str(reponse_ok, answer_2[0]);

	return 0;
}









/********************************************************/
/*Fonction: Activation_channel                          */
/*                                                      */
/********************************************************/
int Activation_channel(struct parametres_connexion *param_connection, int i_channel)
{
	char prefixe_command[100]="CHannel";
	char command[100];
	char temp_str[100];
	char reponse_ok[3]="ok", answer[100];
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};

	Zero_str(temp_str);Zero_str(command);

	strcat(command,prefixe_command);
	sprintf(temp_str, "%d", i_channel);
	strcat(command,temp_str); 
	strcat(command,":ENABLE ");


	Zero_str(answer);
	if(Send_command_and_receive_answer_2(param_connection,command,
				NB_CH,answer_2, 0))
		return Compare_2str(reponse_ok, answer_2[0]);

	return 0;
}







/********************************************************/
/*Fonction: Activation_channels                         */
/*                                                      */
/********************************************************/
int Activation_channels(struct parametres_connexion *param_connection, struct config_all *pconfig_all)
{
	int i,j;

	for(i=1;i<=NB_CH;i++)
	{
		j=i-1;

		if(pconfig_all->pconfig_meas->select_ch[j]==1)
		{
			if(!Activation_channel(param_connection, i))
				return 0;
			printf("\t%s %d %s\n","canal ",i," activé");
		}
		else
		{
			Desactivation_channel(param_connection, i);
			printf("\t%s %d %s\n","canal ",i," non activé");
		}
	}


	printf("\t%s\t","Vérification de l'activation des cannaux de mesure ");

	for(i=1;i<=NB_CH;i++)
	{
		j=i-1;

		if(Get_status_channel(param_connection, pconfig_all, i) !=
				pconfig_all->pconfig_meas->select_ch[j])
		{
			printf("%s\n","... error");
			return 0;
		}
	}
	printf("... ok\n");

	return 1;
}






/********************************************************/
/*Fonction: Configuration date et time                  */
/*                                                      */
/********************************************************/
int Configuration_WLX2_date_time(struct parametres_connexion *param_connection)
{
	int ok=1,ok_saisie,ok_date_time_modif;
	int ok_diff_time,choix_modif_date_time; 
	char str_ans_time[9]={'\0'},str_ans_date[11]={'\0'};
	char c,modif_date_time[100]={'\0'};
	char *curent_time_RPI,current_time_WLX2[100]={'\0'};

	char *choix_possible_modif_date_time_WLX2[5]={"o","n-d","n-h","n-d-h","n-h-d"};
	int nb_choix_possible_modif_date_time_WLX2=5;

	char *choix_possible_modif_date_time_RPI[2]={"o","n"};
	int nb_choix_possible_modif_date_time_RPI=2;

	curent_time_RPI=malloc(100*sizeof(char));
	//current_time_WLX2=malloc(100*sizeof(char));current_time_WLX2[0]='\0';
	//Zero_str(current_time_WLX2);

	ok_saisie=0;
	ok_date_time_modif=0;
	while((c=fgetc(stdin)) !='\n' && c != EOF){};//caractère de fin à supprimer lors de la précédente saisie

	while(!ok_date_time_modif)
	{
		printf("\t%s","->Date et heure (WXL2): ");

		Zero_str(current_time_WLX2);
		Get_date_time_from_WLX2(param_connection, current_time_WLX2);
		printf("%s\n",current_time_WLX2);

		printf("\t%s\n\t...","Est-ce correct? ['o' pour continuer ou 'n-X' pour les modifier (X= 'h' pour l'heure ou 'd' pour la date)]");
		ok_saisie=0;

		while(!ok_saisie)
		{
			Zero_str(modif_date_time);
			fgets_stdin(modif_date_time,sizeof(modif_date_time));

			choix_modif_date_time=Verification_saisie(modif_date_time, choix_possible_modif_date_time_WLX2, nb_choix_possible_modif_date_time_WLX2);
			if (choix_modif_date_time==5){choix_modif_date_time=4;}
			//printf("%s %d\n",modif_date_time,choix_modif_date_time);

			switch(choix_modif_date_time)
			{
				case 0:
					printf("\t%s\n\t...","Saisie incorrecte: Veuillez taper 'o' pour continuer ou 'n-X(X)' pour les modifier (X= 'h' pour l'heure ou 'd' pour la date)");
					ok_saisie=0;
					break;

				case 1:
					ok_saisie=1;
					ok_date_time_modif=1;
					break;

				case 2:
					Date_time_saisie(param_connection, 2, str_ans_time, str_ans_date);
					ok_saisie=1;
					//printf("%s\n",str_ans_date);
					Change_date_WLX2(param_connection,str_ans_date);
					break;

				case 3:
					Date_time_saisie(param_connection, 1, str_ans_time, str_ans_date);
					Change_time_WLX2(param_connection,str_ans_time);
					ok_saisie=1;
					break;

				case 4:
					Date_time_saisie(param_connection, 1, str_ans_time, str_ans_date);
					Change_time_WLX2(param_connection,str_ans_time);
					Date_time_saisie(param_connection, 2, str_ans_time, str_ans_date);
					Change_date_WLX2(param_connection,str_ans_date);
					ok_saisie=1;
					break;

			}
		}
	}


	Zero_str(current_time_WLX2);
	Get_date_time_from_WLX2(param_connection, current_time_WLX2);

	Get_date_time_from_RPI(curent_time_RPI);
	//printf("%s %s\n",curent_time_RPI,current_time_WLX2);
	ok_diff_time=Compare_date_time_RPI_WLX2(curent_time_RPI,current_time_WLX2);
	if (ok_diff_time==0)
	{
		printf("\t%s\n","Warning: La date et l'heure du RPI sont différentes de celles de WLX2");


		ok_saisie=0;
		ok_date_time_modif=0;

		/*while(!ok_date_time_modif)
		  {*/
		Zero_str(current_time_WLX2);
		Get_date_time_from_WLX2(param_connection, current_time_WLX2);
		Zero_str(curent_time_RPI);
		Get_date_time_from_RPI(curent_time_RPI);

		printf("\t%s %s\n","WLX2: ",current_time_WLX2);
		printf("\t%s %s\n","RPI :",curent_time_RPI);
		printf("\t%s\n\t...","Souhaitez-vous caler la date et l'heure du RPI sur celles de WLX2? ['o' ou 'n']");
		//fgets_stdin(modif_date_time,sizeof(modif_date_time));

		ok_saisie=0;
		while(!ok_saisie)
		{
			Zero_str(modif_date_time);
			fgets_stdin(modif_date_time,sizeof(modif_date_time));

			choix_modif_date_time=Verification_saisie(modif_date_time, choix_possible_modif_date_time_RPI, nb_choix_possible_modif_date_time_RPI);
			//printf("%s %d\n",modif_date_time,choix_modif_date_time);

			switch(choix_modif_date_time)
			{
				case 0:
					printf("\t%s\n\t...","Saisie incorrecte: Veuillez taper 'o' ou 'n'");
					ok_saisie=0;
					break;

				case 1:
					Change_date_time_RPI(current_time_WLX2);
					ok_saisie=1;
					printf("\t%s\n","... Modification de la date du RPI effectuée");
					break;

				case 2:
					ok_saisie=1;
					//ok_date_time_modif=1;
					break;
			}
		}


	}
	free(curent_time_RPI);
	return ok;
}





/********************************************************/
/*Fonction: Get date time de WLX2                      */
/*                                                      */
/********************************************************/
int Get_date_time_from_WLX2(struct parametres_connexion *param_connection, char *current_time_WLX2)
{
	int ok,nb_answer=NB_CH;;
	char str_ans_time[9]={'\0'},str_ans_date[11]={'\0'};
	char espace_str[2]=" ";
	char temp_string[50]={'\0'};

	char answer_date[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};
	char answer_time[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};

	//ok=Send_command_and_receive_answer(param_connection,"DATE?", str_ans_date, 0);
	ok=Send_command_and_receive_answer_2(param_connection,"SYSTem:DATE?", nb_answer ,answer_date, 0);
	strcat(str_ans_date,answer_date[0]);


	if(ok)
	{
		//ok=Send_command_and_receive_answer(param_connection,"TIME?", str_ans_time, 0);
		ok=Send_command_and_receive_answer_2(param_connection,"SYSTem:TIME?", nb_answer ,answer_time, 0);
		strcat(str_ans_time,answer_time[0]);

	}


	if (ok)
	{
		strcat(temp_string,str_ans_date);
		strcat(temp_string,espace_str);
		strcat(temp_string,str_ans_time);
		//printf("\n\t%s\n",current_time_WLX2);
	}

	Zero_str(current_time_WLX2);
	memcpy(current_time_WLX2,temp_string,strlen(temp_string)+1);
	return ok;
}










/********************************************************/
/*Fonction:   date_time_saisie                          */
/*                                                      */
/********************************************************/
int Date_time_saisie(struct parametres_connexion *param_connection, int choix_modif, char *str_ans_time,char *str_ans_date)
{
	/*choix_modif=0 ->date et time ; choix_modif=1 ->time ; choix_modif=2 ->date*/
	int ok,ok_time=0,ok_date=0;
	int len;
	char modif_time[9],modif_date[11];
	char *str_regex_modif_time="[0-2][0-9]\\:[0-5][0-9]\\:[0-5][0-9]";
	char *str_regex_modif_date="20[1-9][1-9]\\-[0-1][0-9]\\-[0-3][0-9]";
	regex_t preg_time, preg_date;


	if ((choix_modif==0)||(choix_modif==1))
	{
		regcomp(&preg_time, str_regex_modif_time, REG_NOSUB);
		printf("\t%s\n\t...","Indiquer la nouvelle heure au format [HH:MM:SS]");
		fgets_stdin(modif_time,sizeof(modif_time));
		while (ok_time==0)
		{
			len=strlen(modif_time);
			if((len==8)&& (regexec(&preg_time,modif_time, 0, NULL, 0)==0)) 
			{
				ok_time=1;
				regfree(&preg_time);
				memcpy(str_ans_time,modif_time,strlen(modif_time)*sizeof(char));
			}else{
				printf("\t%s\n\t...","Saisie incorrecte : indiquer la nouvelle heure au format [HH:MM:SS]");
				fgets_stdin(modif_time,sizeof(modif_time));
			}
		}
	}

	if ((choix_modif==0)||(choix_modif==2))
	{
		regcomp(&preg_date, str_regex_modif_date, REG_NOSUB);
		printf("\t%s\n\t...","Indiquer la nouvelle date au format [YYYY-MM-DD]");
		fgets_stdin(modif_date,sizeof(modif_date));

		while (ok_date==0)
		{
			len=strlen(modif_date);
			if((len==10) && (regexec(&preg_date,modif_date, 0, NULL, 0)==0))
			{
				ok_date=1;
				regfree(&preg_date);
				memcpy(str_ans_date,modif_date,strlen(modif_date)*sizeof(char));
			}else{
				//modif_date[0]='\0';
				printf("\t%s\n\t...","Saisie incorrecte : indiquer la nouvelle date au format [YYYY-MM-DD]");
				fgets_stdin(modif_date,sizeof(modif_date));
			}
		}
	}


	ok=1;
	return ok;
}



/********************************************************/
/*Fonction: Get date time du RPI                      */
/*                                                      */
/********************************************************/
int Get_date_time_from_RPI(char *current_time)
{
	int day,month,year,hour,minute,second;

	struct timeval tp;
	struct tm *t;

	time_t curtime;

	gettimeofday(&tp,0);
	curtime=tp.tv_sec;
	t=localtime(&curtime);

	day=t->tm_mday;
	month=t->tm_mon+1;
	year=t->tm_year+1900;
	hour=t->tm_hour;
	minute=t->tm_min;
	second=t->tm_sec;

	sprintf(current_time,"%04d-%02d-%02d %02d:%02d:%02d",year,month,day,hour,minute,second);

	return 1;
}



/********************************************************/
/*Fonction: Comparaison date time du RPI avec celle de WLX2*/
/*                                                      */
/********************************************************/
int Compare_date_time_RPI_WLX2(char *current_time_RPI, char *current_time_WLX2)
{
	struct tm tm_current_time_RPI,tm_current_time_WLX2;
	double diff_t;

	strptime(current_time_RPI,"%Y-%m-%d %H:%M:%S",&tm_current_time_RPI);
	strptime(current_time_WLX2,"%Y-%m-%d %H:%M:%S",&tm_current_time_WLX2);

	diff_t=difftime(mktime(&tm_current_time_RPI),mktime(&tm_current_time_WLX2)); 

	if (fabs(diff_t)>60)
		return 0;
	else
		return 1;
}




/********************************************************/
/*Fonction: Change date time du RPI                     */
/*                                                      */
/********************************************************/
int Change_date_time_RPI(char *current_time)
{
	int ok=1;
	int day,month,year,hour,minute,second;

	char command_date[100]="",command_time[100]="";
	char date[20],time[20];
	struct tm *tm_current_time;

	tm_current_time=malloc(sizeof(struct tm));

	strptime(current_time,"%Y-%m-%d %H:%M:%S",tm_current_time);

	day=tm_current_time->tm_mday;
	month=tm_current_time->tm_mon+1;
	year=tm_current_time->tm_year+1900;
	hour=tm_current_time->tm_hour;
	minute=tm_current_time->tm_min;
	second=tm_current_time->tm_sec;

	free(tm_current_time);
	sprintf(date,"%04d-%02d-%02d",year,month,day);
	sprintf(time,"%02d:%02d:%02d",hour,minute,second);


	strcat(command_date,"sh Source/Script__Update_date_time_RPI.sh ");
	strcat(command_date,"\"");strcat(command_date,date);strcat(command_date,"\"");
	//printf("%s\n",command_date);

	if (Execute_file_sh_sans_retour_info(command_date))
	{
		strcat(command_time,"sh Source/Script__Update_date_time_RPI.sh ");
		strcat(command_time,"\"");
		strcat(command_time,time);
		strcat(command_time,"\"");

		ok=Execute_file_sh_sans_retour_info(command_time);
	}

	if (ok==0)
	{
		printf("\t%s\n","Echec des modifications date et time du RPI");
	}

	return ok;
}




/********************************************************/
/*Fonction: Change date de WLX2                  */
/*                                                      */
/********************************************************/
int Change_date_WLX2(struct parametres_connexion *param_connection, char *current_date)
{
	char command[100]="SYSTem:DATE ";
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};
	char *c;

	for (c=current_date;*c;c++)
	{
		if (*c=='-'){memset(c,',',1);}
	}

	strcat(command,current_date);

	return Send_command_and_receive_answer_2(param_connection,command,
			NB_CH,answer_2, 0);
}




/********************************************************/
/*Fonction: Change time de WLX2                  */
/*                                                      */
/********************************************************/
int Change_time_WLX2(struct parametres_connexion *param_connection, char *current_time)
{

	char command[100]="SYSTem:TIME ";
	char answer_2[2][_RECEIVE_BUFF_SIZE]={"\0","\0"};
	char *c;

	for (c=current_time;*c;c++)
	{
		if (*c==':')
			memset(c,',',1);
	}

	strcat(command,current_time);

	return Send_command_and_receive_answer_2(param_connection,command,
			NB_CH,answer_2, 0);
}
