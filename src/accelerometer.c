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
 */

#include "accelerometer.h"
static void get_axes_data(uint8_t *data, float * buffer, double scale);
static uint8_t writeReg(uint8_t device_address, uint8_t reg, uint8_t *value);
static uint8_t readReg(uint8_t device_address, uint8_t reg, uint8_t *value);
static scale_config *get_scale();
static void acq_cleanup(void *arg);
static void print_cleanup(void *arg);
static uint8_t change_scale(int16_t x, int16_t y, int16_t z);
static void stat(int16_t x, int16_t y, int16_t z, uint8_t reset);


/* Function : parse the data from the LSM9DS0 registers to get readable values
 * Params : 
 * 	-uint8_t *data : contains the 6 bytes of raw data taken from the LSM9DS0 
 * 	-uint8_t *buffer : 3*16bits signed containing the correct values
 * 	-double scale : scale_config.value, conversion rate mG/digit 
 * 			(or dps/digit, Gauss/digit)
 * Return : error_code
 */
static void get_axes_data(uint8_t *data, float * buffer, double scale)
{
	uint8_t i;

	// 6 registers containing data : 8msb x, 8 lsb x, 8 msb y, ...
	for(i = 0; i < 3;i++)
	{
		buffer[i] = (int16_t) (data[2*i] | (data[2*i+1] << 8));
		buffer[i] *= scale;
	}
}


/* Function : Change the scale if the data >80% current scale or  <20% current
 * scale, to prevent/reduce overflow
 * Params :x, y, z are the accelerometer average or values
 * Return : 0 if no change needed, 1 otherwise
*/
static uint8_t change_scale(int16_t x, int16_t y, int16_t z)
{
	float range;
	scale_config new_scale;
	float value = get_scale()[ACC].value;
	uint8_t reg = get_scale()[ACC].reg_config;
	//if (max == 0)
		//max = value*UP_SCALE;
	//if (min == 0)
		//min = value*DOWN_SCALE;
		//if(x > max || y > max || z > max)
		//{
	switch (reg)
	{
		case 0:
			range = 2.0;
			break;
		case 8:
			range = 4.0;
			break;
		case 16:
			range = 6.0;
			break;
		case 24:
			range = 8.0;
			break;
		case 32:
			range = 16.0;
			break;
	}
	if(x < range*UP_SCALE || y < range*UP_SCALE || z < range*UP_SCALE)
	{
		switch (reg)
		{
			case 0:
				new_scale = SCALE_ACC_4G;
				break;
			case 8:
				new_scale = SCALE_ACC_6G;
				break;
			case 16:
				new_scale = SCALE_ACC_8G;
				break;
			case 24:
				new_scale = SCALE_ACC_16G;
				break;
		}
	}
	if(x < range*DOWN_SCALE || y < range*DOWN_SCALE || z < range*DOWN_SCALE)
	{
		switch (reg)
		{
			case 8:
				new_scale = SCALE_ACC_2G;
				break;
			case 16:
				new_scale = SCALE_ACC_4G;
				break;
			case 24:
				new_scale = SCALE_ACC_6G;
				break;
			case 32:
				new_scale = SCALE_ACC_8G;
				break;
		}
	}
	else 
		return 0;

	set_scale(ACC, new_scale);
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
	char toSend[length+1];
	uint8_t return_code;

	// If MSB of the reg adress = 1, multiple write
	if (length > 1)
		reg = reg | 128;

	toSend[0] = reg;
	for(i = 0;i<length;i++)
		toSend[i+1] = (char) value[i];

	bcm2835_i2c_setSlaveAddress(device_address);
	return_code = bcm2835_i2c_write(toSend, length+1);
	switch(return_code){
		case BCM2835_I2C_REASON_ERROR_NACK:
			printf("Non acknowledge\n");
			return 0;
			break;
		case BCM2835_I2C_REASON_ERROR_CLKT:
			printf("Clock sketch timeout\n");
			return 0;
			break;
		case BCM2835_I2C_REASON_ERROR_DATA:
			printf("Data missing\n");
			return 0;
			break;
		case BCM2835_I2C_REASON_OK:
			//printf("Success");
			break;
	}
	return 1;
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
	size_t length = 6;
	uint8_t return_code;

	// If MSB of the reg adress = 1, multiple read
	if (length > 1)
		reg = reg | 128;
	bcm2835_i2c_setSlaveAddress(device_address);
	return_code = bcm2835_i2c_read_register_rs((char *)&reg,(char *) value, length);
	switch(return_code){
		case BCM2835_I2C_REASON_ERROR_NACK:
			printf("Non acknowledge\n");
			return 0;
			break;
		case BCM2835_I2C_REASON_ERROR_CLKT:
			printf("Clock sketch timeout\n");
			return 0;
			break;
		case BCM2835_I2C_REASON_ERROR_DATA:
			printf("Data missing\n");
			return 0;
			break;
		case BCM2835_I2C_REASON_OK:
			//printf("Success\n");
			break;
	}
	return 1;
}


/* Function : Initialize and return a pointer indicating current scales
 * Return : array of scale_config[ACC/GYR/MAG]
 */
static scale_config *get_scale()
{
	static scale_config scale[3];
	if(scale[0].value == 0.0)
	{
		scale[ACC] = SCALE_ACC_4G;
		scale[GYR] = SCALE_GYR_245DPS;
		scale[MAG] = SCALE_MAG_2GAUSS;
	}	

	return scale;
}


static void acq_cleanup(void *arg)
{	
	printf("LSM9DS0 cleanup routine");
	struct acq_cleanup_args *ptr = arg;

	bcm2835_i2c_end();
	pthread_join(*(ptr->print_thread), NULL);
	free(ptr->buffer[0]);
	free(ptr->buffer);
	*(ptr->alive) = 0;	
}


static void print_cleanup(void *arg)
{
	printf("USB data writing cleanup routine");
	fclose((FILE *) arg);
}


uint8_t set_scale(enum instrument inst, scale_config new_scale)
{
	if(inst != ACC || inst != GYR || inst != MAG)
		return 0;
	scale_config *scale = get_scale();
	scale[inst].value = new_scale.value;
	scale[inst].reg_config = new_scale.reg_config;
	return 1;
}


uint8_t read_all(float **buffer)
{
	if (!read_accelerometer(buffer[0]))
	{
		printf("Accelerometer read error\n");
		return 0;
	}
	if (!read_gyrometer(buffer[1]))
	{
		printf("Gyrometer read error\n");
		return 0;
	}
	if (MAG_ACQ && !read_magnetometer(buffer[2]))
	{
		printf("Magnetometer read error\n");
		return 0;
	}

	return 1;
}


uint8_t read_accelerometer(float *buffer)
{
	float scale_value = get_scale()[ACC].value;

	// 6 registers containing data : 8msb x, 8 lsb x, 8 msb y, ...
	uint8_t data[6];
	if(readReg(ACC_ADDRESS, OUT_X_L_A, data) == BCM2835_I2C_REASON_OK)
	{
		get_axes_data(data, buffer, scale_value);
		return 1;
	}
	return 0;
}


uint8_t read_gyrometer(float *buffer)
{
	float scale_value = get_scale()[GYR].value;

	// 6 registers containing data : 8msb x, 8 lsb x, 8 msb y, ...
	uint8_t data[6];
	if(readReg(GYR_ADDRESS, OUT_X_L_G, data) == BCM2835_I2C_REASON_OK)
	{
		get_axes_data(data, buffer, scale_value);
		return 1;
	}
	return 0;
}


uint8_t read_magnetometer(float *buffer)
{
	float scale_value = get_scale()[MAG].value;

	// 6 registers containing data : 8msb x, 8 lsb x, 8 msb y, ...
	uint8_t data[6];
	if(readReg(MAG_ADDRESS, OUT_X_L_M, data) == BCM2835_I2C_REASON_OK)
	{
		get_axes_data(data, buffer, scale_value);
		return 1;
	}
	return 0;
}


uint8_t setup_all()
{
	if(!bcm2835_i2c_begin())
		return 0;

	if (!setup_accelerometer())
	{
		printf("Accelerometer initialization error\n");
		return 0;
	}
	if (!setup_gyrometer())
	{
		printf("Gyrometer initialization error\n");
		return 0;
	}
	if (MAG_ACQ && !setup_magnetometer())
	{
		printf("Magnetometer initialization error\n");
		return 0;
	}

	return 1;
}


uint8_t setup_accelerometer()
{
	uint8_t config_reg;

	// continuous update & 50Hz data rate - z,y,x axis enabled, 
	config_reg = 0b01010111;	       
	if(!writeReg(ACC_ADDRESS, CTRL_REG1_XM, &config_reg))
	{
		return 0;
	}
	// Bandwidth anti-alias 773Hz, 16G scale, self test normal mode
	config_reg = get_scale()[ACC].reg_config;
	if(!writeReg(ACC_ADDRESS, CTRL_REG2_XM, &config_reg))
	{
		return 0;
	}

	return 1;
}


uint8_t setup_gyrometer()
{
	uint8_t config_reg;

	// 95 Hz output datarate, 12.5 cutoff - normal mode - z,y,x axis enabled 
	config_reg = 0b00001111;	       
	if(!writeReg(GYR_ADDRESS, CTRL_REG1_G, &config_reg))
	{
		return 0;
	}

	// Continuous update, scale 245dps
	config_reg = get_scale()[GYR].reg_config;
	if(!writeReg(GYR_ADDRESS, CTRL_REG4_G, &config_reg))
	{
		return 0;
	}

	return 1;
}


uint8_t setup_magnetometer()
{
	uint8_t config_reg;

	// temperature enable - Mag high res - 50 Hz output datarate
	config_reg = 0b11110000;
	if(!writeReg(MAG_ADDRESS, CTRL_REG5_XM, &config_reg))
		return 0;

	// Magnetic full scale
	config_reg = get_scale()[MAG].reg_config;
	if(!writeReg(MAG_ADDRESS, CTRL_REG6_XM, &config_reg))
		return 0;

	// mode continuous for magnetic sensor (enable mag sensor) 
	config_reg = 0;    
	if(!writeReg(MAG_ADDRESS, CTRL_REG7_XM, &config_reg))
		return 0;
	return 1;
}


void *acq_GYR_ACC(void * arg)
{
	printf("Thread LSM9D0 created id : %li\n", syscall(__NR_gettid));

	struct timespec tt;
	tt.tv_sec = 0;
	tt.tv_nsec = (long) 1000000000.0/INPUT_DATA_RATE;

	uint8_t i, pos = 0, nb_cycle = 0;
	uint8_t nb_acq = MAG_ACQ ? 3 : 2; //3 if magnetometer is used
	float **buffer; //contains the data from the LSMD9
	int64_t sum_square[3], sum[3];
	double mean[3], std_dev[3];
	struct data_acq message_queue[QUEUE_SIZE];

	pthread_t print_thread;

	// Allocate mem to buffer getting data from sensors :
	if((buffer = malloc(nb_acq * sizeof(float *))) == NULL 
			|| nb_acq == 0)
	{
		printf("malloc failed\n");
		return 0;
	}
	// (x, y, z) = 3 for (ACC, GYR, (MAG))=2or3 containing a float
	if((buffer[0] = malloc(3 * nb_acq * sizeof(float))) == NULL)
	{
		printf("malloc failed\n");
		return 0;
	}
	for(i = 1; i < nb_acq; i++)
		buffer[i] = buffer[0] + i * 3;

	// Setup the cleanup handlers
	struct acq_cleanup_args args;
	args.alive = arg;
	*args.alive = 1;
	args.buffer = buffer;
	args.print_thread = &print_thread;
	pthread_cleanup_push(acq_cleanup, &args);

	// Setup i2c, then setup ctrl registers of the lsm9ds0, create thread
	if(!setup_all() || pthread_create(&print_thread, NULL,	print_to_file,
				(void *) message_queue))
	{
		bcm2835_i2c_end();
		printf("Exiting acq_Gyr_Acc thread\n");
		pthread_exit((void *) 0);
	}

	sleep(1);

	while(!end_program)
	{
		read_all(buffer);
		message_queue[pos].acq_time = time(NULL);
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
		message_queue[pos].read_allowed = 1;

		if(pos++ == QUEUE_SIZE)
		{
			pos = 0;
			stat(buffer[0][0], buffer[0][1], buffer[0][2], 1);
		}
		else
			stat(buffer[0][0], buffer[0][1], buffer[0][2], 0);

		nanosleep(&tt, NULL);
	}

pthread_cleanup_pop(1);
pthread_exit((void *) 1);
}

static void stat(int16_t x, int16_t y, int16_t z, uint8_t reset)
{
	int32_t i;
	static int64_t sum[3], sum_square[3];
	static int64_t sum_total[3], sum_square_total[3];

	static double mean[3], std_dev[3];
	static int16_t max[3], min[3];

	static time_t new_cycle = NULL;

	if (!new_cycle)
		new_cycle = time(NULL);
	if(x > max[0])
		max[0] = x;
	else if(x < min[0])
		min[0] = x;
	if(y > max[1])
		max[1] = y;
	else if(y < min[1])
		min[1] = y;
	if(z > max[2])
		max[2] = z;
	else if(z < min[2])
		min[2] = z;
	sum[0] += x;
	sum[1] += y;
	sum[2] += z;
	sum_square[0] += x*x;
	sum_square[1] += y*y;
	sum_square[2] += z*z;

	if (reset)
	{
		//Calcul mean and dev, 
		//adds temp_sums to total_sums and reset them
		for(i = 0; i < 3; i++)
		{
			mean[i] = sum[i] / QUEUE_SIZE;
			std_dev[i] = sqrt(sum_square[i]/QUEUE_SIZE -
					mean[i]);
			sum_total[i] += sum[i];
			sum_square_total[i] += sum_square[i];
			sum[i] = 0;
			sum_square[i] = 0;
			//send_to_sgf()
		}
		if(change_scale(std_dev[0], std_dev[1], std_dev[2]))
			setup_all();

		//Computes total mean and dev on the period
		//Send these values to sigfox
		if(difftime(time(NULL), new_cycle) > SGF_SEND_PERIOD)
		{
			for(i = 0; i < 3; i++)
			{
				mean[i] = sum_total[i] / QUEUE_SIZE;
				std_dev[i] =sqrt(sum_square_total[i]/QUEUE_SIZE
					      -mean[i]);
				sum_total[i] = 0;
				sum_square_total[i] = 0;
				//send_to_sgf()
			}
			//send to sigfox
		}


	}
}

void *print_to_file(void * arg)
{
	printf("Thread saving to file created, id : %li\n",
			syscall(__NR_gettid));

	struct timespec tt;
	tt.tv_sec = 0;
	tt.tv_nsec = (long) 1000000000.0/INPUT_DATA_RATE;

	const char *path = "";
	char *cfg = "PATH_LSM9DS0_DATA";
	FILE *fp;
	//transfer from i2c thread to print thread is done through a message
	//queue which contains up to QUEUE_SIZE records. 
	// This thread reads data when message_queue[i].read_allowed var is set to 1.
	struct data_acq *message_queue = (struct data_acq*) arg;
	uint8_t pos = 0;

	pthread_cleanup_push(print_cleanup, fp);

	//the path to the file saving data is specified in the config
	get_cfg_str(&path, &cfg, 1);
	if(!(fp = fopen(path, "a")))
	{
		printf("Can't open file %s\n", path);
		pthread_exit((void *) 0);
	}

	//Add header 
	if(MAG_ACQ)
		fprintf(fp, "MAG_X, MAG_Y, MAG_Z, ");
	fprintf(fp, "ACC_X, ACC_Y, ACC_Z, GYR_X, GYR_Y, GYR_Z\n");

	while(!end_program)
	{
		//Try to print at the same rate data is send by i2c
		nanosleep(&tt, NULL);

		while(message_queue[pos].read_allowed == 1)
		{	
			fprintf(fp, "%li,", &message_queue[pos].acq_time);
			if(MAG_ACQ)
				fprintf(fp, "%g,%g,%g,",
						message_queue[pos].x_mag,
						message_queue[pos].y_mag,
						message_queue[pos].z_mag);
			fprintf(fp, "%g,%g,%g,%g,%g,%g\n", 
					message_queue[pos].x_acc,
					message_queue[pos].y_acc,
					message_queue[pos].z_acc,
					message_queue[pos].x_gyr,
					message_queue[pos].y_gyr,
					message_queue[pos].z_gyr);
			message_queue[pos].read_allowed = 0;
			//Garder pos entre 0 et QUEUE_SIZE
			pos = pos++ == QUEUE_SIZE ? 0 : pos;
		}

	}
	pthread_cleanup_pop(1);
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
