#ifndef SIGFOX_H
#define SIGFOX_H

#define SIZE_SIGFOX_MSG 12 // bytes
#define MAX_NB_MSG 140 //daily max number of messages that can be sent with sgf

//TODO :  fichier sigfox
//REAL : #define SGF_SEND_PERIOD 24*3600/MAX_NB_MSG // 140 messages/jour : 3600*24/140
//Test : 
	//#define SGF_SEND_PERIOD 20
	
	
// Message sigfox recap (see excel sheet for complete info) :
// TIME :3| alive + id + msg_type :1| float min or mean :4| float max or dev :4

/* Function : Thread created by the acquisition threads to transmit data to
 *  	the sigfox thread.
 * Params : struct sgf_data. Only fields min, max mean, std_dev and id are used
 *  It is assumed that the struct pointed by args will be changed only every
 *  SGF_SEND_PERIOD, and so that the sgf thread will read it before that happens
*/
void * send_sigfox(void *args);

/* Function : Thread created by main to transmit data through the sigfox network
 * 	Receives data through the global sgf_msg var, then send it
 * Params : An array indicating whether each acquisition thread is alive or not
*/
void * sigfox(void *args);

#endif
