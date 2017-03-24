/*
 * accelerometer.c
 *  Interface for the LSM9DS0 
 *  Give access to register address, scale factors and setup/read functions
 *
 * @TODO : Verifier que les valeurs de sensibilité données par la datasheet
 *         soient correctes : ex pour acc,
 *         scale = +-2g, sensitivity = 0.061mg
 *                  -> un int16_t contient 2^15*0.000061 g = 1.9988
 *         scale = +-16g, sensitivity = 0.732mg
 *                  -> un int16_t contient 2^15*0.000732 g = 24
 *
 * @TODO : Faut il pouvoir modifier la sensibilité et l'échelle de chaque
 * 		instruments via une interface ? ou hard coded comme maintenant
 */

#include "accelerometer.h"
static uint8_t get_axes_data(uint8_t *data, int16_t * buffer, double scale);
static uint8_t writeReg(uint8_t device_address, uint8_t reg, uint8_t *value);
static uint8_t readReg(uint8_t device_address, uint8_t reg, uint8_t *value);

/* Function : parse the data from the LSM9DS0 registers to get readable values
 * Params : 
 * 	-uint8_t *data : contains the 6 bytes of raw data taken from the LSM9DS0 
 * 	-uint8_t *buffer : 3*16bits signed containing the correct values
 * Return : error_code
 */
static uint8_t get_axes_data(uint8_t *data, int16_t * buffer, double scale)
{
	uint8_t i;

	// 6 registers containing data : 8msb x, 8 lsb x, 8 msb y, ...
	for(i = 0; i < 3;i++)
	{
		buffer[i] = (int16_t) (data[2*i] | (data[2*i+1] << 8));
		buffer[i] *= scale;
	}
	return 1;

}

/* Function : write something in a register through i2c
 * 	See datasheet LSM9DS0 p33 for register adress and write cmd info
 * Params : 
 * 	-uint8_t device_address : Slave address of the i2c device
 * 	-uint8_t reg: register to write to. if multiple writes, 1st register add
 * 	-uint8_t *value: array of bytes to be written
 * Return :error_code
 */
static uint8_t writeReg(uint8_t device_address, uint8_t reg, uint8_t *value)
{
	uint32_t i;
	size_t length = sizeof(value);
	uint8_t toSend[length+1];
	uint8_t return_code;

	// If MSB of the reg adress = 1, multiple write
	if (length > 1)
		reg = reg | 128;

	toSend[0] = reg;
	for(i = 0;i<length;i++)
		toSend[i+1] = value[i];

	bcm2835_i2c_setSlaveAddress(device_address);
	return_code = bcm2835_i2c_write(toSend, length+1);
	switch(return_code){
		case BCM2835_I2C_REASON_ERROR_NACK:
			printf("Non acknowledge");
			break;
		case BCM2835_I2C_REASON_ERROR_CLKT:
			printf("Clock sketch timeout");
			break;
		case BCM2835_I2C_REASON_ERROR_DATA:
			printf("Data missing");
			break;
		case BCM2835_I2C_REASON_OK:
			//printf("Success");
			break;
	}
	return return_code;
}

/* Function : read something in a register through i2c
 * 	See datasheet LSM9DS0 p33 for register adress and read cmd info
 * Params : 
 * 	-uint8_t device_address : Slave address of the i2c device
 * 	-uint8_t reg: register to read from. if multiple read, 1st register add
 * 	-uint8_t *value: array of bytes where the read is put
 * Return :error_code
 */
static uint8_t readReg(uint8_t device_address, uint8_t reg, uint8_t *value)
{
	uint32_t i;
	size_t length = 6;
	uint8_t return_code;

	// If MSB of the reg adress = 1, multiple read
	if (length > 1)
		reg = reg | 128;
	bcm2835_i2c_setSlaveAddress(device_address);
	return_code = bcm2835_i2c_read_register_rs(&reg, value, length);
	switch(return_code){
		case BCM2835_I2C_REASON_ERROR_NACK:
			printf("Non acknowledge");
			break;
		case BCM2835_I2C_REASON_ERROR_CLKT:
			printf("Clock sketch timeout");
			break;
		case BCM2835_I2C_REASON_ERROR_DATA:
			printf("Data missing");
			break;
		case BCM2835_I2C_REASON_OK:
			//printf("Success\n");
			break;
	}
	return return_code;
}
static scale_config *get_scale()
{
	static scale_config scale[3];
	if(scale[0].value == 0.0)
	{
		scale[ACC] = SCALE_ACC_4G;
		scale[GYR] = SCALE_GYR_245DPS;
		scale[MAG] = SCALE_MAG_2GAUSS;
	}	
	//[3] = {SCALE_ACC_4G,
	//SCALE_GYR_245DPS,
	//SCALE_MAG_4GAUSS};

	return scale;
}
/* Function : Set the scale for the specified LSM9D0 instrument
 * 	The hardware needs to be reconfigured with setup() afterwards	
 * Params :  * 	uint8_t instrument : 0 is accelerometer, 1 is gyrometer, 2 magnetometer
 * 	double new_scale is the new scale value for the specified instrument
 * 		See possible choices in accelerometer.h
 * Return : The current scale value for the specified instrument
 */
static double set_scale(enum instrument inst, scale_config new_scale)
{
	if(inst != ACC || inst != GYR || inst != MAG)
		return 0;
	scale_config *scale = get_scale();
	//scale[inst] = new_scale; must be done member by member ? probably
}

uint8_t read_all(int16_t **buffer)
{
	if (!read_accelerometer(buffer[0]))
	{
		printf("Accelerometer read error");
		return 0;
	}
	if (!read_gyrometer(buffer[1]))
	{
		printf("Gyrometer read error");
		return 0;
	}
	if (MAG_ACQ && !read_magnetometer(buffer[2]))
	{
		printf("Magnetometer read error");
		return 0;
	}

	return 1;
}

uint8_t read_accelerometer(int16_t *buffer)
{
	uint8_t i;

	float scale_value = get_scale()[ACC].value;

	// 6 registers containing data : 8msb x, 8 lsb x, 8 msb y, ...
	uint8_t data[6];
	if(readReg(ACC_ADDRESS, OUT_X_L_A, data) == BCM2835_I2C_REASON_OK)
	{
		return get_axes_data(data, buffer, scale_value);
	}
	return 0;
}

uint8_t read_gyrometer(int16_t *buffer)
{
	uint8_t i;

	float scale_value = get_scale()[GYR].value;

	// 6 registers containing data : 8msb x, 8 lsb x, 8 msb y, ...
	uint8_t data[6];
	if(readReg(GYR_ADDRESS, OUT_X_L_G, data) == BCM2835_I2C_REASON_OK)
	{
		return get_axes_data(data, buffer, scale_value);
	}
	return 0;
}

uint8_t read_magnetometer(int16_t *buffer)
{
	uint8_t i;

	float scale_value = get_scale()[MAG].value;

	// 6 registers containing data : 8msb x, 8 lsb x, 8 msb y, ...
	uint8_t data[6];
	if(readReg(MAG_ADDRESS, OUT_X_L_M, data) == BCM2835_I2C_REASON_OK)
	{
		return get_axes_data(data, buffer, scale_value);
	}
	return 0;
}

uint8_t setup_all()
{
	bcm2835_i2c_begin();

	if (!setup_accelerometer())
	{
		printf("Accelerometer initialization error");
		return 0;
	}
	if (!setup_gyrometer())
	{
		printf("Gyrometer initialization error");
		return 0;
	}
	if (MAG_ACQ && !setup_magnetometer())
	{
		printf("Magnetometer initialization error");
		return 0;
	}

	return 1;
}

uint8_t setup_accelerometer()
{
	uint8_t config_reg;
	uint8_t i;

	uint8_t scale = get_scale()[ACC].reg_config;
	// continuous update & 50Hz data rate - z,y,x axis enabled, 
	config_reg = 0b01010111;	       
	if(writeReg(ACC_ADDRESS, CTRL_REG1_XM, &config_reg) != OK)
	{
		return 0;
	}
	// Bandwidth anti-alias 773Hz, 16G scale, self test normal mode
	config_reg = scale;
	if(writeReg(ACC_ADDRESS, CTRL_REG2_XM, &config_reg) != OK)
	{
		return 0;
	}

	return 1;
}

uint8_t setup_gyrometer()
{
	uint8_t config_reg;
	uint8_t i;

	uint8_t scale = get_scale()[GYR].reg_config;

	// 95 Hz output datarate, 12.5 cutoff - normal mode - z,y,x axis enabled 
	config_reg = 0b00001111;	       
	if(writeReg(GYR_ADDRESS, CTRL_REG1_G, &config_reg) != OK)
	{
		return 0;
	}

	// Continuous update, scale 245dps
	config_reg = scale;    
	if(writeReg(GYR_ADDRESS, CTRL_REG4_G, &config_reg) != OK)
	{
		return 0;
	}

	return 1;
}

uint8_t setup_magnetometer()
{
	uint8_t config_reg;
	uint8_t i;

	uint8_t scale = get_scale()[MAG].reg_config;

	// temperature enable - Mag high res - 50 Hz output datarate
	config_reg = 0b11110000;
	if(writeReg(MAG_ADDRESS, CTRL_REG5_XM, &config_reg) != OK)
		return MAG_FAIL;

	// Magnetic full scale
	config_reg = scale;    
	if(writeReg(MAG_ADDRESS, CTRL_REG6_XM, &config_reg) != OK)
		return MAG_FAIL;

	// mode continuous for magnetic sensor (enable mag sensor) 
	config_reg = 0;    
	if(writeReg(MAG_ADDRESS, CTRL_REG7_XM, &config_reg) != OK)
		return MAG_FAIL;
	return OK;
}



void *acq_GYR_ACC()
{	
	struct timespec tt;
	tt.tv_sec = 0;
	tt.tv_nsec = (long) 1000000000.0/INPUT_DATA_RATE;
	printf("Thread LSM9D0 created id : %i\n", syscall(__NR_gettid));
	uint8_t i;
	uint8_t nb_acq = 2; //3 if magnetometer is used
	uint8_t pos = 0;
	int16_t **buffer; //contains the data from the LSMD9
	struct data_acq message_queue[QUEUE_SIZE];

	double scale;

	pthread_t print_thread;

	if (MAG_ACQ != 0)
		nb_acq = 3;

	//Allocate mem for X Y Z measure for gyro + acc (+mag if defined)
	buffer = malloc(nb_acq * 3 * sizeof(int16_t *));
	buffer[0] = malloc(3 * sizeof(int16_t));
	for(i = 1; i < nb_acq; i++)
		buffer[i] = buffer[0] + i * 3;

	setup_all();
	pthread_create(&print_thread, NULL, 
			print_to_file, (void *) message_queue);


	sleep(1);

	while(!end_program)
	{
		read_all(buffer);
		nanosleep(&tt, NULL);
		//ADD to queue
		message_queue[pos].x_acc = buffer[0][0];
		message_queue[pos].y_acc = buffer[0][1];
		message_queue[pos].z_acc = buffer[0][2];
		message_queue[pos].x_gyr = buffer[1][0];
		message_queue[pos].y_gyr = buffer[1][1];
		message_queue[pos].z_gyr = buffer[1][2];
		if(MAG_ACQ)
		{
			message_queue[pos].x_mag = buffer[2][0];
			message_queue[pos].y_mag = buffer[2][1];
			message_queue[pos].z_mag = buffer[2][2];
		}
		message_queue[pos].write = 1;
		pos = pos++ == 199 ? 0 : pos;
	}

	pthread_join(print_thread, NULL);
	free(buffer[0]);
	free(buffer);
	bcm2835_i2c_end();
	pthread_exit((void *) 1);
}


void *print_to_file(void * arg)
{
	struct timespec tt;
	tt.tv_sec = 0;
	tt.tv_nsec = (long) 1000000000.0/INPUT_DATA_RATE;

	const char *path = "";
	char *cfg = "PATH_LSM9DS0_DATA";
	FILE *fp;

	//transfer from i2c thread to print thread is done through a message
	//queue which contains up to 200 records. This thread reads data when 
	//the write var is set to 1.
	struct data_acq *message_queue = (struct data_acq*) arg;
	uint8_t pos = 0;

	//the path to the file saving data is specified in the config
	get_cfg_str(&path, &cfg, 1);
	if(!(fp = fopen(path, "a")))
	{
		printf("Can't open file %s\n", path);
		pthread_exit((void *) 0);
	}

	//Add header 
	if(MAG_ACQ)
		fprintf(fp, "MAG_X, MAG_Y, MAG_Z,\
				ACC_X, ACC_Y, ACC_Z,\
				GYR_X, GYR_Y, GYR_Z\n");
	else
		fprintf(fp, "ACC_X, ACC_Y, ACC_Z, GYR_X, GYR_Y, GYR_Z\n");

	while(!end_program)
	{
		//Try to print at the same rate data is send by i2c
		nanosleep(&tt, NULL);

		while(message_queue[pos].write == 1)
		{	
			if(MAG_ACQ)
				fprintf(fp, "%i,%i,%i,",
						message_queue[pos].x_mag,
						message_queue[pos].y_mag,
						message_queue[pos].z_mag);

			fprintf(fp, "%i,%i,%i,%i,%i,%i\n", 
					message_queue[pos].x_acc,
					message_queue[pos].y_acc,
					message_queue[pos].z_acc,
					message_queue[pos].x_gyr,
					message_queue[pos].y_gyr,
					message_queue[pos].z_gyr);
			message_queue[pos].write = 0;
			//Garder pos entre 0 et 200
			pos = pos++ == 199 ? 0 : pos;
		}
	}
	fclose(fp);
	pthread_exit((void *) 1);
}


/*int main()
  {
  uint8_t address = 0x1d;
  uint8_t i;
  uint8_t data[6] = {200, 150, 12, 178, 85, 252};
/*	
get_axes_data(data, buffer, SCALE_MAG_8GAUSS);
for(i=0;i<3;i++)
{
printf("%i\n", buffer[i]);
}
*//*
     bcm2835_init();
     acq_GYR_ACC();
/*bcm2835_i2c_begin(); 	
bcm2835_i2c_setSlaveAddress(address);
setup_accelerometer();
setup_gyrometer();

int16_t **buffer; //contains the data from the LSMD9
//Allocate mem for X Y Z measure for gyro + acc (+mag if defined)
buffer = malloc(2 * sizeof(int16_t *));
buffer[0] = malloc(3 * 2 * sizeof(int16_t));


for(i = 1; i < 2; i++)
buffer[i] = buffer[0] + i * 3;

for (i=0;i<200;i++)
{
printf("Buffer address main : %i\n", *buffer);
read_all(buffer);
//read_accelerometer(buffer[0],0);
usleep(20);
printf("ACC : x : %i  y : %i  z : %i\n", buffer[0][0], buffer[0][1], buffer[0][2]);
//read_gyrometer(buffer[1],0);
printf("GYR : x : %i  y : %i  z : %i\n", buffer[1][0], buffer[1][1], buffer[1][2]);
}
free(buffer[0]);
free(buffer);
bcm2835_i2c_end();*/
//	bcm2835_close();
//}
