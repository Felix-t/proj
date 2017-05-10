#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "arm.h"


int main()
{
	arm_t myArm;
	armType_t armType;
	armError_t err = ARM_ERR_NONE;
	uint8_t rev[16] = "";
	uint64_t sn = 0;
	uint16_t rfFreq = 0;
	uint8_t rfPower = 0;
	
	//Init armapi
	err=armInit(&myArm, "/dev/ttyUSB0");

	err=armInfo(&myArm, &armType, rev, &sn, &rfFreq, &rfPower);

	//Print information
	switch(armType)
	{
		case ARM_TYPE_N8_LP:
		printf("ARM_TYPE_N8_LP Detected.\n");
		break;
		
		case ARM_TYPE_N8_LD:
		printf("ARM_TYPE_N8_LD Detected.\n");
		break;
		
		case ARM_TYPE_N8_LW:
		printf("ARM_TYPE_N8_LW Detected.\n");
		break;
		
		default:
		printf("No arm type detected.\n");
		break;
	}	
	err=armDeInit(&myArm);

}
