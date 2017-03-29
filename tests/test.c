#include <time.h>
#include <stdio.h>
#include <stdlib.h>
 
int main(void)
{
	FILE *fp;
	fp = fopen("test.txt", "r");
	char *i;
	char *j;
	i=malloc(1000*sizeof(char));
	j=malloc(1000*sizeof(char));
	printf("abcd : %i\n%s\n%s\n", fscanf(fp, "%s: %s", i, j), i, j);
	fclose(fp);
}
