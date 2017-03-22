#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

static void *test()
{
	sleep(5);
	printf("ID : %i\n", pthread_self());
	pthread_exit((void *) 1);
}


int main()
{
	const char *path = "grosfichier.txt";	
	FILE *fp;
	int i;
	if(fp = fopen(path, "a"))
	{
		for(i=0;i<100000000;i++)
		{
			fprintf(fp, "0%i,%i,%i,%i,%i,%i\n",i,i,i,i,i,i);
		}
	}
}
