#ifndef CONNEXION__H
#define CONNEXION__H

#ifndef HEADERS__H
#include "Headers.h"
#endif

#ifndef CONFIGURATION_PGM__H
#include "Configuration_Programme_Opsens_WLX2.h"
#endif

#include <netinet/tcp.h>
#include <netinet/in.h>
#include <arpa/inet.h>



#pragma pack(1)

#define _UDP_MAX_PACKET_SIZE 1452 
#define _UDP_CMD_MAX_LEN 255

#define _SEND_BUFF_SIZE (_UDP_MAX_PACKET_SIZE-(2*sizeof(uint16_t))-(2*sizeof(uint8_t)))
#define _RECEIVE_BUFF_SIZE (_SEND_BUFF_SIZE-_UDP_CMD_MAX_LEN)

#define _PUBLIC_SCPI_CMD 1000
#define _PUBLIC_SPCI_CMD_RESP 1001
#define _OPTICAL_MEASURE 3001

#define _COMMANDS_PORT 50000
#define _COMMANDS_ANSWER_PORT 50001
#define _ACQUISITION_DATA_PORT 50002

#define IP_SERVER "10.0.0.15"
#define IP_CLIENT "10.0.0.1"
#define IP_ACQUISITION_DATA "239.0.0.1"


#define TIMEOUT_S 1 /*seconde*/
#define TIMEOUT_MS 1 /*milliseconde*/
//timeout=TIMEOUT_S*1000+TIMEOUT_MS milliseconde

struct public_command_scpi {
uint16_t DataID;
uint16_t SegmentID;
uint16_t CommandSize;
char Client_command[_SEND_BUFF_SIZE]; 
};


struct public_response_scpi {
uint16_t DataID;
uint16_t SegmentID;
uint16_t ResponseSize;
char Received_command[_UDP_CMD_MAX_LEN]; 
char Server_response[_RECEIVE_BUFF_SIZE]; 
};


struct parametres_connexion {
int ID_socket_command;
int ID_socket_acquisition_data;
struct sockaddr_in Server_Address;
struct sockaddr_in Client_Address;
struct sockaddr_in Acquisition_data_Address;
};



struct struct_SC_RA
{
int nb_answer;
int ok_print;
int ok_command;
int ok_answer;
char answer[2][_RECEIVE_BUFF_SIZE];
const char *command;
struct parametres_connexion *param_connection;
};




int Kill_old_process_Pgm_Opsens(char *name_process);

void Print_public_scpi_response(struct public_response_scpi *ans);

void Print_public_scpi_command(struct public_command_scpi *cmd);

int Open_socket();

int Close_socket(int ID_socket);

int Make_and_test_connexion(struct parametres_connexion *param_connection, int ok_print);

int Init_connexion(struct parametres_connexion *param_connection);

int Init_param_connexion(struct parametres_connexion *param_connection);

int Reset_connexion(struct parametres_connexion *param_connection);

int Test_connexion(struct parametres_connexion *param_connection, int ok_print);

int Str_command__to__struct_public_command_scpi(const char *command, struct public_command_scpi *cmd); 

void Zero_public_scpi_response(struct public_response_scpi *ans);

int Envoi_command(struct parametres_connexion *param_connection, const char *command, int ok_print);

int Reception_answer(struct parametres_connexion *param_connection,  struct public_response_scpi *ans, int ok_print);

int Reception_answer_2(struct parametres_connexion *param_connection,  struct public_response_scpi *ans, int ok_print);

int Send_command_and_receive_answer(struct parametres_connexion *param_connection, const char *command, char *answer, int ok_print);

int Send_command_and_receive_answer_2(struct parametres_connexion *param_connection, const char *command, int nb_answer, char answer[2][_RECEIVE_BUFF_SIZE], int ok_print);

//void Suppression_suffixes_response_from_device(struct public_response_scpi * resp);

void Suppression_suffixes_of_Received_command(struct public_response_scpi * resp);

void Suppression_suffixes_of_Server_response(struct public_response_scpi * resp);

int Find_channel_in_Received_command (char *str, char *substr);

int Verification_response_from_device_is_err (char *answer);

void find_nb_answer_await_and_channel  (const char *command, int *nb_answer_await, int *numero_channel);

#endif

