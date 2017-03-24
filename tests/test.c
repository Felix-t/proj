#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <bcm2835.h>

#include <math.h>
uint8_t i = 0;
#define NB_MEASURES 500
#define MEASURE_FREQUENCY 370

enum instrument {ACC,GYR,MAG} ;

static double set_scale(enum instrument inst, double new_scale)
{
	printf("%i\n", inst);
}

int main()
{
	char regaddr = 0x21;
	uint8_t buf[1];
	bcm2835_init();
	bcm2835_i2c_begin();
	printf("ABC");
	bcm2835_i2c_setSlaveAddress(0x1D);
	printf("DEF");
	bcm2835_i2c_read_register_rs(&regaddr, buf, 1);
	printf("\n %u \n", buf[0]);
	bcm2835_i2c_end();
	bcm2835_close();
}
