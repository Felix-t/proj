#include "Connexion_Opsens_WLX2.h"
#include "Fonctions_Utiles.h" 




/********************************************************/
/*Fonction: Kill_old_process                           */
/*                                                      */
/********************************************************/
int Kill_old_process_Pgm_Opsens(char *name_process)
{
	int ok;
	char pref_command[100]="sh Source/Script_Kill_Previous_Process_Opsens_WLX2.sh ";
	char command[100]="";

	strcat(command,pref_command);
	strcat(command," ");
	strcat(command,name_process);

	//printf("%s\n",command);
	ok=Execute_file_sh(command);

	return ok;
}



/********************************************************/
/*Fonction: Make_and_test_connexion                     */
/*                                                      */
/********************************************************/
int Make_and_test_connexion(struct parametres_connexion *param_connection, int ok_print)
{
	int ok;


	ok=Init_connexion(param_connection);

	ok=1;
	if (ok)
	{
		ok=Test_connexion(param_connection,ok_print);
	}
	return ok;
}





/********************************************************/
/*Fonction: Init_connexion                              */
/*                                                      */
/********************************************************/
int Init_connexion(struct parametres_connexion *param_connection)
{
	int ok = 1;
	char command[100]="sh Source/Script__Init_Connexion_Opsens_WLX2.sh";

	//ok=Execute_file_sh(command);

	if (ok)
	{
		ok=Init_param_connexion(param_connection);
	}

	return ok;
}



/********************************************************/
/*Fonction: Init_param_connexion                        */
/*                                                      */
/********************************************************/
int Init_param_connexion(struct parametres_connexion *param_connection)
{
	int ok=1;
	int ID_socket_command = 0, ID_socket_acquisition_data = 0;

	ID_socket_command=Open_socket();
	if (ID_socket_command==-1){ok=0;goto fin_Init_param_connexion;}else{param_connection->ID_socket_command=ID_socket_command;}

	ID_socket_acquisition_data=Open_socket();
	if (ID_socket_acquisition_data==-1)
	{
		ok=0;
		goto fin_Init_param_connexion;
	}
	else
	{
		param_connection->ID_socket_acquisition_data=ID_socket_acquisition_data;
	}


	if(ok)
	{
		memset(&param_connection->Server_Address,0,sizeof(struct sockaddr_in));
		param_connection->Server_Address.sin_family=AF_INET;
		param_connection->Server_Address.sin_port=htons(_COMMANDS_PORT);
		param_connection->Server_Address.sin_addr.s_addr=inet_addr(IP_SERVER);

		memset(&param_connection->Client_Address,0,sizeof(struct sockaddr_in));
		param_connection->Client_Address.sin_family=AF_INET;
		param_connection->Client_Address.sin_port=htons(_COMMANDS_ANSWER_PORT);
		param_connection->Client_Address.sin_addr.s_addr=inet_addr(IP_CLIENT);

		bind(param_connection->ID_socket_command,(struct sockaddr *) &param_connection->Client_Address,sizeof(struct sockaddr_in));

		memset(&param_connection->Acquisition_data_Address,0,sizeof(struct sockaddr_in));
		param_connection->Acquisition_data_Address.sin_family=AF_INET;
		param_connection->Acquisition_data_Address.sin_port=htons(_ACQUISITION_DATA_PORT);
		//param_connection->Acquisition_data_Address.sin_addr.s_addr=inet_addr(IP_ACQUISITION_DATA);
		param_connection->Acquisition_data_Address.sin_addr.s_addr=INADDR_ANY;

		bind(param_connection->ID_socket_acquisition_data,(struct sockaddr *) &param_connection->Acquisition_data_Address,sizeof(struct sockaddr_in));

		struct ip_mreq multicast_group;

		multicast_group.imr_multiaddr.s_addr=inet_addr(IP_ACQUISITION_DATA);
		multicast_group.imr_interface.s_addr=inet_addr(IP_CLIENT);

		if (setsockopt(param_connection->ID_socket_acquisition_data, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *) &multicast_group, sizeof(multicast_group))<0)
		{
			printf("%s\n","Erreur adding multicast group");
			ok=0; 
		}



	}



fin_Init_param_connexion:;

			 return ok;
}




/********************************************************/
/*Fonction: Reset_connexion                            */
/*                                                      */
/********************************************************/
int Reset_connexion(struct parametres_connexion *param_connection)
{

	int ok,ok1,ok2,ok3;
	char command[100]="sh Source/Script__Reset_Connexion_Opsens_WLX2.sh";

	ok1=Close_socket(param_connection->ID_socket_command);

	ok2=Close_socket(param_connection->ID_socket_acquisition_data);

	ok3=Execute_file_sh(command);

	ok=ok1*ok2*ok3;

	return ok;
}



/********************************************************/
/*Fonction: Open socket                                 */
/*                                                      */
/********************************************************/
//int Open_socket(struct parametres_connexion *param_connection)
int Open_socket()
{
	int ok=1,MySocket;

	/*struct timeval tv;
	  tv.tv_sec=0;
	  tv.tv_usec=TIMEOUT_MS;*/

	if((MySocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP))==-1)
	{printf("%s\n","Echec de l'ouverture d'un socket");ok=0; goto fin_Open_socket;}

	/*if (setsockopt(MySocket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv))<0)
	  {printf("%s\n","Echec du paramètrage TIMEOUT_MS du socket");ok=0; goto fin_Open_socket;}
	  */

	//param_connection->ID_socket=MySocket;

fin_Open_socket:;

		//return ok;
		return MySocket;
}



/********************************************************/
/*Fonction: close socket                                 */
/*                                                      */
/********************************************************/
int Close_socket(int ID_socket)
{
	int ok,x;

	if (close(ID_socket)==0)
	{ok=1;}else{printf("%s\n","Echec de la fermeture du socket");ok=0;}

	return ok;
}



/********************************************************/
/*Fonction: Print_public_scpi_command                   */
/*                                                      */
/********************************************************/
void Print_public_scpi_command(struct public_command_scpi *cmd)
{
	int i;
	char command[_UDP_CMD_MAX_LEN];

	printf("%s\n","Commande envoyée à WLX2: ");
	printf("\t%s %d\n","Data ID: ", cmd->DataID);
	printf("\t%s %d\n","Segment ID: ", cmd->SegmentID);
	printf("\t%s %d\n","Command size: ", cmd->CommandSize);
	for(i=0;i<cmd->CommandSize;i++)
	{
		command[i]=cmd->Client_command[i];
	}
	command[cmd->CommandSize]='\0';
	//printf("%s %s\n","Client_command: ", cmd->Client_command);
	printf("\t%s %s\n","Client_command: ", command);
}



/********************************************************/
/*Fonction: Print_public_scpi_response                   */
/*                                                      */
/********************************************************/
void Print_public_scpi_response(struct public_response_scpi *ans)
{
	printf("%s\n","Réponse de WLX2: ");
	printf("\t%s %d\n","Data ID: ", ans->DataID);
	printf("\t%s %d\n","Segment ID: ", ans->SegmentID);
	printf("\t%s %d\n","ResponseSize: ", ans->ResponseSize);
	printf("\t%s %s\n","Received_command: ", ans->Received_command);
	printf("\t%s %s\n","Server_response: ", ans->Server_response);
}



/********************************************************/
/*Fonction: Zero_Print_public_scpi_response                   */
/*                                                      */
/********************************************************/
void Zero_public_scpi_response(struct public_response_scpi *ans)
{
	//printf("%s\n","Réponse de WLX2: ");

	ans->DataID=0;
	ans->SegmentID=0;
	ans->ResponseSize=0;
	memset(ans->Received_command, 0, _UDP_CMD_MAX_LEN);
	memset(ans->Server_response, 0, _RECEIVE_BUFF_SIZE); 
	//Zero_str(ans->Received_command);
	//Zero_str(ans->Server_response);
}



/********************************************************/
/*Fonction: Str_command__to__struct_public_command_scpi */
/*                                                      */
/********************************************************/
int Str_command__to__struct_public_command_scpi(const char *command, struct public_command_scpi *cmd)
{
	int ok,size_cmd;
	int len_command;
	char command_to_send[_UDP_CMD_MAX_LEN],terminaison[3]="\r\n";

	memset(command_to_send,0,_UDP_CMD_MAX_LEN);
	strcat(command_to_send,command);
	strcat(command_to_send,terminaison);

	ok=1;
	Zero_str(cmd->Client_command);
	cmd->DataID=_PUBLIC_SCPI_CMD;
	cmd->SegmentID=0x0101;
	cmd->CommandSize=strlen(command_to_send)+1;//printf("Command length %d\n",cmd->CommandSize);

	memcpy(cmd->Client_command,command_to_send,cmd->CommandSize);//printf("%s %s\n",command,"xxxx ");

	return ok;
}




/********************************************************/
/*Fonction: Envoi_command                               */
/*                                                      */
/********************************************************/
int Envoi_command(struct parametres_connexion *param_connection, const char *command, int ok_print)
{
	int ok=1,size_cmd;

	struct public_command_scpi cmd =
	{
		.DataID=0,
		.SegmentID=0,
		.CommandSize=0,
	};
	Zero_str(cmd.Client_command);

	if(Str_command__to__struct_public_command_scpi(command,&cmd)==0)
	{
		ok=0; goto fin_Envoi_command;
	}

	size_cmd=sizeof(struct public_command_scpi)-(_SEND_BUFF_SIZE-cmd.CommandSize);
	//printf("oooo%s\n",cmd.Client_command);
	if(sendto(param_connection->ID_socket_command, &cmd, size_cmd, 0, (struct sockaddr *)&param_connection->Server_Address,sizeof(param_connection->Server_Address))==-1) 
	{
		printf("%s %s %s\n","Echec de l'envoi de la commande \'",cmd.Client_command,"\' à WLX2");ok=0; goto fin_Envoi_command;
	}

	if (ok_print){Print_public_scpi_command(&cmd);}

fin_Envoi_command:;

		  return ok;

}




/********************************************************/
/*Fonction: Reception_answer                            */
/*                                                      */
/********************************************************/
int Reception_answer(struct parametres_connexion *param_connection,  struct public_response_scpi *ans, int ok_print)
{
	int ok=1,nb_ans;

	fd_set select_fds;
	struct timeval timeout;

	FD_ZERO(&select_fds);
	FD_SET(param_connection->ID_socket_command, &select_fds);

	timeout.tv_sec=TIMEOUT_S;
	timeout.tv_usec=TIMEOUT_MS*1000;

	if(select(sizeof(struct public_response_scpi), &select_fds, NULL, NULL, &timeout) == 0)
	{
		//printf("%s\n","Select has timed out ..."); 
		printf("%s\n","Pas de réponse de WLX2");
		ok=0;
	}else{

		recvfrom(param_connection->ID_socket_command,ans,sizeof(struct public_response_scpi),0,NULL,NULL);

		if (ans->DataID !=_PUBLIC_SPCI_CMD_RESP)
		{
			ok=0;
			printf("%s %d %s %d %s\n","Erreur: DataID de la réponse reçue : ",ans->DataID," (",_PUBLIC_SPCI_CMD_RESP," attendu)");
		}

		//Suppression_suffixes_response_from_device(ans);
		Suppression_suffixes_of_Server_response(ans);

		if (ok_print){Print_public_scpi_response(ans);}

	}

fin_Reception_answer:;

		     return ok;

}




/********************************************************/
/*Fonction: Reception_answer_2                           */
/*                                                      */
/********************************************************/
int Reception_answer_2(struct parametres_connexion *param_connection,  struct public_response_scpi *ans, int ok_print)
{
	int ok=1,nb_ans;

	fd_set select_fds;
	struct timeval timeout;

	FD_ZERO(&select_fds);
	FD_SET(param_connection->ID_socket_command, &select_fds);

	timeout.tv_sec=TIMEOUT_S;
	timeout.tv_usec=TIMEOUT_MS*1000;

	if(select(param_connection->ID_socket_command + 1, &select_fds, NULL, NULL, &timeout) == 0)
	{

		ok=0;

	}else{

		recvfrom(param_connection->ID_socket_command,ans,sizeof(struct public_response_scpi),0,NULL,NULL);

		//printf("*****%s %s\n",ans->Received_command,ans->Server_response);

		Suppression_suffixes_of_Server_response(ans);

		//printf("-----%s\n",ans->Server_response);

		if (ok_print){Print_public_scpi_response(ans);}

	}

fin_Reception_answer:;

		     return ok;

}




/********************************************************/
/*Fonction: Send_command_and_receive_answer             */
/*                                                      */
/********************************************************/
int Send_command_and_receive_answer(struct parametres_connexion *param_connection, const char *command, char *answer, int ok_print)
{
	int ok,ok_command,i_max_tentative,n_max_tentative=3;
	int i,len_command,len_answer;
	char Receivedcommand[_UDP_CMD_MAX_LEN]="",Serverresponse[_RECEIVE_BUFF_SIZE]=""; 

	struct public_response_scpi ans=
	{
		.DataID=0,
		.SegmentID=0,
		.ResponseSize=0,
	};
	memcpy(ans.Received_command,Serverresponse,sizeof(strlen(Serverresponse)));
	memcpy(ans.Server_response,Receivedcommand,sizeof(strlen(Receivedcommand)));

	i_max_tentative=1;
	ok=1;
	//printf("%s","\t");
	while(i_max_tentative<=n_max_tentative)
	{
		Zero_public_scpi_response(&ans);
		printf("%d\n",i_max_tentative);
		if (Envoi_command(param_connection, command, ok_print)==0) 
		{
			printf("%s","\t");ok=0;break;
		}else{
			if (Reception_answer(param_connection,&ans,ok_print)==0) 
			{
				printf("%s","\t");ok=0;break;
			}else{
				if (Verification_response_from_device_is_err(ans.Server_response)==0)
				{
					ok=0;break;
				}else{
					if (Compare_2str(command,ans.Received_command)==1) 
					{
						memcpy(answer,ans.Server_response,strlen(ans.Server_response)*sizeof(char));
						break;
					}else{
						//printf("%d\n",i_max_tentative);
						i_max_tentative++;
					}
				}
			}
		}
	}   

	if (i_max_tentative>n_max_tentative)
	{
		printf("\n%s\n","Humm ... c'est embarrassant: la commande reçue par WLX2 est différente de celle qui a été envoyée");
		printf("%s %s\n","Commande envoyée: ",command);
		//printf("%s %s\n","Commande reçue par WLX2",received_command);
		ok=0;
	}

	return ok;
}










pthread_cond_t condition_Send_command_and_Receive_answer=PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex_Send_command_and_Receive_answer=PTHREAD_MUTEX_INITIALIZER;
void* thread_Send_command (void* arg);
void* thread_Receive_answer (void* arg);



/********************************************************/
/*Fonction: Send_command_and_receive_answer_2           */
/*                                                      */
/********************************************************/
int Send_command_and_receive_answer_2(struct parametres_connexion *param_connection, const char *command, int nb_answer, char answer[2][_RECEIVE_BUFF_SIZE], int ok_print)
{
	int ok;

	pthread_t xthread_Send_command;
	pthread_t xthread_Receive_answer;

	struct struct_SC_RA param_SC_RA={
		.answer[0] ="",
		.answer[1] = "",
		.nb_answer=nb_answer,
		.ok_print=ok_print,
		.ok_command=0,
		.ok_answer=0,
		.command=command,
		.param_connection=param_connection
	};


// 	if (!pthread_create(&xthread_Send_command, NULL, thread_Send_command, &param_SC_RA)
// 		&& !pthread_create (&xthread_Receive_answer, NULL, thread_Receive_answer, &param_SC_RA) 
// 		&& !pthread_join(xthread_Send_command,NULL)
//		&& !pthread_join(xthread_Receive_answer,NULL)
//		&& param_SC_RA.ok_answer*param_SC_RA.ok_command)
//	{
//		strcat(answer[0], param_SC_RA.answer[0]);
//		strcat(answer[1], param_SC_RA.answer[1]);
// 	}
//
// 	param_SC_RA->ok_command = Envoi_command(param_SC_RA->param_connection,
// 						param_SC_RA->command, 
// 						param_SC_RA->ok_print);
// 	sleep(0.1);
// 	
//
//
//
//
//

	ok=pthread_create (&xthread_Send_command, NULL, thread_Send_command, &param_SC_RA); 
	if (ok==0)
	{
		ok=pthread_create (&xthread_Receive_answer, NULL, thread_Receive_answer, &param_SC_RA); 
	}else{ok=1;}

	if (ok==0)
	{
		ok=pthread_join(xthread_Send_command,NULL);
	}else{ok=1;}

	if (ok==0)
	{
		ok=pthread_join(xthread_Receive_answer,NULL);
	}else{ok=1;}

	if (ok==1){ok=-1;}
	if (ok==0){ok=1;}

	if(ok){ok=param_SC_RA.ok_answer*param_SC_RA.ok_command;}
	//printf("%d %d\n",param_SC_RA.ok_answer,param_SC_RA.ok_command);

	if(ok)
	{
		//memcpy(answer[0],param_SC_RA.answer[0],strlen(param_SC_RA.answer[0])*sizeof(char));
		//memcpy(answer[1],param_SC_RA.answer[1],strlen(param_SC_RA.answer[1])*sizeof(char));
		strcat(answer[0],param_SC_RA.answer[0]);
		strcat(answer[1],param_SC_RA.answer[1]);
		//printf("answer[0],answer[1] %s %s\n",answer[0],answer[1]);
		//printf("param_SC_RA.answer[0],param_SC_RA.answer[1] %s %s\n",param_SC_RA.answer[0],param_SC_RA.answer[1]);
	}

	return ok;

}




/********************************************************/
/*Fonction: thread_Send_command                         */
/*                                                      */
/********************************************************/
void* thread_Send_command (void* arg)
{
	struct struct_SC_RA *param_SC_RA=arg;
	int ok;

	//printf("%s\n","Debut thread_Send_command ");
	//sleep(1.0);
	//printf("%s\n","Debut thread_Send_command ");

	pthread_mutex_lock(&mutex_Send_command_and_Receive_answer);
	pthread_cond_wait(&condition_Send_command_and_Receive_answer,&mutex_Send_command_and_Receive_answer);
	param_SC_RA->ok_command=Envoi_command(param_SC_RA->param_connection, param_SC_RA->command, param_SC_RA->ok_print);
	pthread_mutex_unlock(&mutex_Send_command_and_Receive_answer);


	//printf("%s\n",param_SC_RA->command);
	//printf("%s\n","ok thread_Send_command");



	//printf("%s\n","fin thread_Send_command");
	pthread_exit(NULL);

}




/********************************************************/
/*Fonction: thread_Receive_answer                       */
/*                                                      */
/********************************************************/
void* thread_Receive_answer (void* arg)
{
	struct struct_SC_RA *param_SC_RA=arg;
	int i,*ok_answer,i_answer_received,ok_command,ok_reception_answer;
	int nb_answer_await,numero_channel_answer_await,numero_channel_answer,len_command,len_answer;
	char Receivedcommand[_UDP_CMD_MAX_LEN],Serverresponse[_RECEIVE_BUFF_SIZE]; 


	find_nb_answer_await_and_channel (param_SC_RA->command, &nb_answer_await, &numero_channel_answer_await);
	//printf("%d %d\n",nb_answer_await, numero_channel_answer_await);

	struct public_response_scpi ans=
	{
		.DataID=0,
		.SegmentID=0,
		.ResponseSize=0,
	};

	Zero_str(ans.Received_command);
	Zero_str(ans.Server_response);

	//@TODO : Free that ?
	ok_answer=malloc(nb_answer_await*sizeof(int));

	for(i=0;i<nb_answer_await;i++)
	{
		Zero_str(param_SC_RA->answer[i]);
	}

	Reception_answer_2(param_SC_RA->param_connection,&ans,param_SC_RA->ok_print); //pour vider le buffer du socket si besoin

	ans.DataID=0;
	ans.SegmentID=0;
	ans.ResponseSize=0;
	Zero_str(ans.Received_command);
	Zero_str(ans.Server_response);

	i_answer_received=0;
	while (i_answer_received<nb_answer_await)
	{
		ans.DataID=0;ans.SegmentID=0;ans.ResponseSize=0;
		Zero_str(ans.Received_command);Zero_str(ans.Server_response);

		if (nb_answer_await==2){numero_channel_answer_await=i_answer_received;}

		if (i_answer_received==0)
		{
			pthread_mutex_lock(&mutex_Send_command_and_Receive_answer);
			pthread_cond_signal(&condition_Send_command_and_Receive_answer);
			pthread_mutex_unlock(&mutex_Send_command_and_Receive_answer);
		}
		//sleep(0.1); // = sleep(0)
		ok_reception_answer=Reception_answer_2(param_SC_RA->param_connection,&ans,param_SC_RA->ok_print);

		//printf("ok_reception_answer %d\n",ok_reception_answer);
		//printf("%s\n",ans.Received_command);printf("ooo%s\n",ans.Server_response);

		if (ok_reception_answer)
		{
			//param_SC_RA->ok_answer=1;
			ok_answer[i_answer_received]=1;

			numero_channel_answer=Find_channel_in_Received_command(ans.Received_command,"#");
			//printf("numero_channel_answer %d numero_channel_answer_await %d\n",numero_channel_answer,numero_channel_answer_await);

			Suppression_suffixes_of_Received_command(&ans);

			if (Compare_2str(param_SC_RA->command,ans.Received_command)==0) 
			{
				printf("\n%s\n","Humm ... c'est embarrassant: la commande reçue par WLX2 est différente de celle qui a été envoyée");
				printf("%s %s\n","Commande envoyée: ",param_SC_RA->command);
				printf("%s %s\n","Commande reçue par WLX2",ans.Received_command);
				ok_answer[i_answer_received]=0;
				param_SC_RA->ok_answer=0;goto fin_thread_Receive_answer;
			}


			if(Verification_response_from_device_is_err(ans.Server_response)==0){ok_answer[i_answer_received]=0;}

			//printf("nb_answer_await %d\n",nb_answer_await);
			//printf("numero_channel_answer %d\n",numero_channel_answer);
			//printf("numero_channel_answer_await+1 %d\n",numero_channel_answer_await);
			if((nb_answer_await==1)&&(numero_channel_answer==(numero_channel_answer_await)))
			{   
				strcat(param_SC_RA->answer[numero_channel_answer-1],ans.Server_response);
				//memcpy(param_SC_RA->answer[numero_channel_answer-1],ans.Server_response,strlen(ans.Server_response)*sizeof(char));
			}

			if (nb_answer_await==2)
			{
				strcat(param_SC_RA->answer[numero_channel_answer-1],ans.Server_response);
			}



			//printf("%d %s\n",numero_channel_answer, ans.Server_response);
			//  }

	}else{
		//param_SC_RA->ok_answer=0;goto fin_thread_Receive_answer;
		ok_answer[i_answer_received]=0;
	}

	i_answer_received++;
} 


param_SC_RA->ok_answer=0;
i_answer_received=0;
while (i_answer_received<nb_answer_await)
{
	if(ok_answer[i_answer_received]==1){param_SC_RA->ok_answer=1;break;}
	i_answer_received++;
}



fin_thread_Receive_answer : ;
free(ok_answer);
pthread_exit(NULL);

}





/********************************************************/
/*Fonction: find_nb_answer_await_and_channel            */
/*                                                      */
/********************************************************/
void find_nb_answer_await_and_channel  (const char *command, int *nb_answer_await, int *numero_channel)
{
	int ok_find_search1,ok_find_search2;
	char str_to_find[2][10]={"01:","02:"};

	ok_find_search1=Find_substr_in_str((char *)command,str_to_find[0]);
	ok_find_search2=Find_substr_in_str((char *)command,str_to_find[1]);

	if (ok_find_search1||ok_find_search2)
	{
		*nb_answer_await=1;
		if(ok_find_search1){*numero_channel=1;}else{*numero_channel=2;}

	}else{
		*nb_answer_await=2;
		*numero_channel=0;
	}

}




/********************************************************/
/*Fonction: Send_command_and_receive_answer_2           */
/*                                                      */
/********************************************************/
/*int Send_command_and_receive_answer_2(struct parametres_connexion *param_connection, const char *command, int nb_answer, char answer[2][_RECEIVE_BUFF_SIZE], int ok_print)
  {
  int ok,ok_answers,*ok_answer,i_answer_received,ok_command;
  int i,nb_channel_answer,len_command,len_answer;
  char Receivedcommand[_UDP_CMD_MAX_LEN],Serverresponse[_RECEIVE_BUFF_SIZE]; 

  ok_answer=malloc(nb_answer*sizeof(int));
  for (i=0;i<nb_answer;i++){ok_answer[i]=1;}

  struct public_response_scpi ans=
  {
  .DataID=0,
  .SegmentID=0,
  .ResponseSize=0,
  };
  Zero_str(ans.Received_command);
  Zero_str(ans.Server_response);

  ok_command=1;
//printf("%s","\t");

i_answer_received=0;

//printf("COMMANDE %s\n",command);

if (Envoi_command(param_connection, command, ok_print)==0) {printf("%s","\t");ok_command=0;}
//printf("ok_command: %d\n",ok_command);

if (ok_command)
{
while (i_answer_received<nb_answer)
{
ans.DataID=0;ans.SegmentID=0;ans.ResponseSize=0;
Zero_str(ans.Received_command);Zero_str(ans.Server_response);

if (Reception_answer_2(param_connection,&ans,ok_print)==0) 
{
ok_answer[i_answer_received]=0;
}else{
//printf("%d-- %s %s\n", i_answer_received+1, ans.Received_command, ans.Server_response);

if(Verification_response_from_device_is_err(ans.Server_response)==0){ok_answer[i_answer_received]=0;}

nb_channel_answer=Find_channel_in_Received_command(ans.Received_command,"#");

//printf("%s %d\n","nb_channel_answer: ",nb_channel_answer);
Suppression_suffixes_of_Received_command(&ans);

if (Compare_2str(command,ans.Received_command)==1) 
{
//memcpy(answer[nb_channel_answer-1],ans.Server_response,strlen(ans.Server_response)*sizeof(char));
strcpy(answer[nb_channel_answer-1],ans.Server_response);
}else{
printf("\n%s\n","Humm ... c'est embarrassant: la commande reçue par WLX2 est différente de celle qui a été envoyée");
printf("%s %s\n","Commande envoyée: ",command);
printf("%s %s\n","Commande reçue par WLX2",ans.Received_command);
ok_answer[i_answer_received]=0;
}
}
i_answer_received++;
}
}



ok_answers=0;
for (i=0;i<nb_answer;i++)
{
if(ok_answer[i]){ok_answers=1;break;}
}

ok=ok_command*ok_answers;

//for(i=0;i<nb_answer;i++)
//{
//printf("%s%d: %s %d\n","Answer",i,answer[i], ok_answer[i]);
//}


free(ok_answer);
ok_answer=NULL;

return ok;
}
*/





/********************************************************/
/*Fonction: Test_connexion                              */
/*                                                      */
/********************************************************/
int Test_connexion(struct parametres_connexion *param_connection, int ok_print)
{
	int i,ok,test_answer,nb_answer=NB_CH;
	char command[_UDP_CMD_MAX_LEN]="SYSTem:IDN?";
	char answer_attendu_by_channel[NB_CH][100];
	memcpy(answer_attendu_by_channel[0],
			param_connection->module_idn, 100);
	strcat(answer_attendu_by_channel[0], "-01");
	if(NB_CH > 1)
	{
		memcpy(answer_attendu_by_channel[1], 
				param_connection->module_idn, 100);
		strcat(answer_attendu_by_channel[1], "-02");
	}
	//struct public_response_scpi answer;
	char answer[_RECEIVE_BUFF_SIZE] = {0};
	char answer_2[2][_RECEIVE_BUFF_SIZE] = {{0}};

	printf("\t%s\n","->Test de la connexion:");
	printf("\t\t%s %s\n\t\t","Envoi de la commande: ",command);


	//ok=Send_command_and_receive_answer(param_connection, command, answer, ok_print);
	ok=Send_command_and_receive_answer_2(param_connection, command, nb_answer ,answer_2, ok_print);

	if(ok)
	{
		printf("%s\n","Réponse de WLX2: ");


		for (i=1;i<=nb_answer;i++)
		{

			printf("\t\t%s%d%s%s","- pour le canal ",i,": ",answer_2[i-1]);

			if (strlen(answer_2[i-1])>0)
			{
				test_answer=Find_substr_in_str(answer_2[i-1],answer_attendu_by_channel[i-1]);

				if (test_answer)
				{
					printf("%s\n"," ... ok");
					ok=ok*1;
				}else{
					printf("%s\n"," ... Réponse incorrecte");
					ok=ok*0;
				}
			}else{
				printf("%s\n","Pas de réponse de WLX2");
				ok=ok*0;
			}
		}
	}
	printf("make and test ok = %i", ok);
	return ok;
}






/********************************************************/
/*Find_channel_in_Received_command                      */
/*                                                      */
/********************************************************/
int Find_channel_in_Received_command (char *str, char *substr)
{
	int i,numero_channel;
	char *temp_str;
	char *ret;
	ret=strstr(str,substr);
	if (ret!=NULL)
	{
		temp_str=malloc((strlen(ret))*sizeof(char));

		for(i=1;i<strlen(ret);i++)
		{
			temp_str[i-1]=ret[i];
		}
		temp_str[i-1]='\0';

		numero_channel=atoi(temp_str);
		free(temp_str);
		temp_str=NULL;
	}else{
		numero_channel=0;
	}

	return numero_channel;
}




/********************************************************/
/*Fonction: Suppression_suffixes_of_Server_response     */
/*                                                      */
/********************************************************/
void Suppression_suffixes_of_Server_response(struct public_response_scpi * resp)
{
	int i,j;

	int pos_Server_response,len_Server_response;

	char reponse_ok[3]="ok";

	len_Server_response=strlen(resp->Server_response);

	pos_Server_response=Find_position_substr_in_str(resp->Server_response, "\x04");

	if(pos_Server_response>=0)
	{
		for(i=(pos_Server_response);i<len_Server_response;i++)
		{
			resp->Server_response[i]='\0';
		}

		if ((pos_Server_response>=2)&&(resp->Server_response[pos_Server_response-2]=='\r')&&(resp->Server_response[pos_Server_response-1]=='\n'))
		{
			resp->Server_response[pos_Server_response-2]='\0';
			resp->Server_response[pos_Server_response-1]='\0';
		}


		if (pos_Server_response==0){memcpy(resp->Server_response,reponse_ok,sizeof(reponse_ok));}
	}
}






/********************************************************/
/*Fonction: Suppression_suffixes_of_Received_command    */
/*                                                      */
/********************************************************/
void Suppression_suffixes_of_Received_command(struct public_response_scpi * resp)
{
	int i,j;
	int pos_Received_command,len_Received_command;

	char reponse_ok[3]="ok";

	len_Received_command=strlen(resp->Received_command);


	pos_Received_command=Find_position_substr_in_str(resp->Received_command, "\r");
	//printf("*****->%d %d\n",pos_Received_command,len_Received_command);

	if(pos_Received_command>-1)
	{
		for(i=pos_Received_command;i<len_Received_command;i++)
		{
			resp->Received_command[i]='\0';
		}
	}
}












/********************************************************/
/*Fonction: Suppression_suffixes_response_from_device   */
/*                                                      */
/********************************************************/
/*void Suppression_suffixes_response_from_device(struct public_response_scpi * resp)
  {
  int i,j;
  int pos_Received_command,len_Received_command;
  int pos_Server_response,len_Server_response;

  char reponse_ok[3]="ok";

  len_Received_command=strlen(resp->Received_command);
  len_Server_response=strlen(resp->Server_response);

  pos_Received_command=Find_position_substr_in_str(resp->Received_command, "\r");
//printf("*****->%d %d\n",pos_Received_command,len_Received_command);

if(pos_Received_command>-1)
{
for(i=pos_Received_command;i<len_Received_command;i++)
{
resp->Received_command[i]='\0';
}
}



pos_Server_response=Find_position_substr_in_str(resp->Server_response, "\x04");
//printf("*****->%d %d\n",pos_Server_response,len_Server_response);
if(pos_Server_response>-1)
{
for(i=pos_Server_response;i<len_Server_response;i++)
{
resp->Server_response[i]='\0';
}
if (pos_Server_response==0){memcpy(resp->Server_response,reponse_ok,sizeof(reponse_ok));}
}

pos_Server_response=Find_position_substr_in_str(resp->Server_response, "\n");
//printf("*****->%d %d\n",pos_Server_response,len_Server_response);
if(pos_Server_response>-1)
{
for(i=pos_Server_response;i<len_Server_response;i++)
{
resp->Server_response[i]='\0';
}
if (pos_Server_response==0){memcpy(resp->Server_response,reponse_ok,sizeof(reponse_ok));}
}

pos_Server_response=Find_position_substr_in_str(resp->Server_response, "\r");
//printf("*****->%d %d\n",pos_Server_response,len_Server_response);
if(pos_Server_response>-1)
{
for(i=pos_Server_response;i<len_Server_response;i++)
{
resp->Server_response[i]='\0';
}
if (pos_Server_response==0){memcpy(resp->Server_response,reponse_ok,sizeof(reponse_ok));}
}


//printf("res %s %s\n",resp->Received_command,resp->Server_response);

//printf("**** %s ****\n",resp->Server_response);

}

*/








/********************************************************/
/*Fonction: Verification response from device is err     */
/*                                                      */
/********************************************************/
int Verification_response_from_device_is_err (char *answer)
{
	int ok=1,len;

	len=strlen(answer);
	if (len>=3)
	{
		if ((answer[0]=='E')&&(answer[1]=='r')&&(answer[2]=='r'))
		{
			//printf("%s %s\n","Réponse de WLX2: ", answer);
			//printf("\n%s\n","Humm ... c'est embarrassant: une erreur s'est produite");
			ok=0;
		}
	}

	return ok;
}


