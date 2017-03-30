#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>

struct public_command_scpi 
{
	uint16_t DataID;
	uint16_t SegmentID;
	uint16_t CommandSize;
	char cmd[255];
};




#define BUFLEN 512  //Max length of buffer
#define PORT 50001  //The port on which to listen for incoming data
#define SERVER "10.0.0.15"
#define PORT_OUT 50000  //The port on which to send data
/*int main(void)
{
	struct sockaddr_in si_me, si_other;

	int s, i, slen = sizeof(si_other) , recv_len;
	char buf[BUFLEN];

	//create a UDP socket
	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		printf("end of socket");
	}

	// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind socket to port
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
		printf(" end of bind");
	}

	//keep listening for data
	while(1)
	{
		printf("Waiting for data...");
		fflush(stdout);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
		{
			printf("recvfrom()");
		}

		//print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("Data: %s\n" , buf);

		//now reply the client with the same data
		if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
		{
			printf("sendto()");
		}
	}

	close(s);
	return 0;
}*/

void print_cmd(char *cmd);

int main()
{	
	char command_to_send[255],terminaison[3]="\r\n";

	memset(command_to_send,0,255);
	strcat(command_to_send,"SYSTem:IDN?");
	strcat(command_to_send,terminaison);

	char cmd_str[262];
	struct public_command_scpi cmd =
	{
		.DataID=1000,
		.SegmentID=0x0101,
		.CommandSize=12,
	};

	memset(cmd.cmd,0,sizeof(cmd.cmd));
	memcpy(cmd.cmd,command_to_send,cmd.CommandSize);//printf("%s %s\n",command,"xxxx ");
	memcpy(cmd_str, &cmd, 30);
	struct sockaddr_in si_other, si_me;
	int s, i, slen=sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];

	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		printf("socket");
	}

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT_OUT);

	if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}

	//send the message
	if (sendto(s, cmd_str, 30 , 0 , (struct sockaddr *) &si_other, slen)==-1)
	{
		printf("sendto()");
	}

	print_cmd(cmd_str);	
	//receive a reply and print it
	//clear the buffer by filling null, it might have previously received data
	memset(buf,'\0', BUFLEN);

	puts(buf);
	//// zero out the structure
	memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	//bind socket to port
	if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
	{
		printf("bind");
	}
	int recv_len;
	//try to receive some data, this is a blocking call
	if ((recv_len = recvfrom(s, buf, BUFLEN, 0,NULL, NULL)) == -1)
	{
		printf("recvfrom()");
	}

	printf("recu");
	close(s);
	return 0;
}

void print_cmd(char *cmd)
{
	int i;
	for(i=0;i<20;i++)
	{
		if (i < 6)
			printf("%i\n", (uint16_t) (cmd[i] + (cmd[++i] << 8)));
		else
			printf("%c", (char) cmd[i]);
	}

}
