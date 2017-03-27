#include <stdio.h>
#include <pthread.h>
#include <sys/types.h> 
#include <stdint.h>

 
int main(void)
{
	_Atomic uint8_t alive[2] = {2,3};
	printf("Alive : %u\n alive[0] %u\n &alive[1] %u\n",
			alive, alive[0], &alive[1]);
}
