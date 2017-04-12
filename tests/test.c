#include <time.h>
#include <stdio.h>

int main()
{
	time_t t = time(NULL);
	printf("%i", t);
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
