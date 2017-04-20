#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main()
{
	int i;
	uint8_t *a = malloc(sizeof(uint8_t *)*5);
	uint16_t *c = malloc(sizeof(uint16_t *) *5);
	for (i=0;i<5;i++)
		printf("8int : %i - 16int : %i\n", a +i, c +i);
}
/*
   struct sgf_data{
   uint8_t write_allowed;
   identity id;
   float min;
   float max;
   float mean;
   float std_dev;
   pthread_mutex_t mutex;
   };*/
