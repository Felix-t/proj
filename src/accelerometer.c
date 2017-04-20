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
static uint8_t change_scale(int16_t x, int16_t y, int16_t z);
static inline float calc_mean(double sum, int nb);
static inline float calc_std_dev(double sum, int nb, float mean);


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
	uint8_t reg = get_scale()[ACC].reg_config;
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
	if(x > range*UP_SCALE || y > range*UP_SCALE || z > range*UP_SCALE)
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
	free(ptr->buffer[0].data[0]);
	free(ptr->buffer[0].data);
	*(ptr->alive) = 0;	
}


static inline float calc_mean(double sum, int nb)
{
	return sum/nb;
}


static inline float calc_std_dev(double sum, int nb, float mean)
{
	return sqrt(sum/nb-mean*mean);
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
	if (LSM9DS0_MAG_ENABLE && !read_magnetometer(buffer[2]))
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
	if(!readReg(ACC_ADDRESS, OUT_X_L_A, data) == BCM2835_I2C_REASON_OK)
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
	if(readReg(GYR_ADDRESS, OUT_X_L_G, data))
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
	if (LSM9DS0_MAG_ENABLE && !setup_magnetometer())
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


void stats(float **data, uint8_t reset)
{
	int32_t i, j;
	static struct sgf_data tab_data[2+LSM9DS0_MAG_ENABLE][3];
	static pthread_t threads[2+LSM9DS0_MAG_ENABLE][3];

	static double sum[2+LSM9DS0_MAG_ENABLE][3];
	static double sum_square[2+LSM9DS0_MAG_ENABLE][3];
	static double sum_total[2+LSM9DS0_MAG_ENABLE][3];
	static double sum_square_total[2+LSM9DS0_MAG_ENABLE][3];
	
	static double min[2+LSM9DS0_MAG_ENABLE][3];
	static double max[2+LSM9DS0_MAG_ENABLE][3];
	static double mean[2+LSM9DS0_MAG_ENABLE][3];
	static double std_dev[2+LSM9DS0_MAG_ENABLE][3];
	
	static uint32_t count = 0;
	static time_t new_cycle = NULL;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (!new_cycle) //init new_cycle on first call of the function
		new_cycle = time(NULL);

	// Add new data to the sums, actualize min and max if needed
	for (i = 0; i < 2+LSM9DS0_MAG_ENABLE; i++)
	{
		for(j = 0; j < 3; j++)
		{
			if(data[i][j] > max[i][j])
				max[i][j] = data[i][j];
			else if(data[i][j] < min[i][j])
				min[i][j] = data[i][j];
			sum[i][j] += data[i][j];
			sum_square[i][j] += data[i][j]*data[i][j];
		}
	}

	// Every QUEUE_SIZE measures, calculate the standard deviation for ACC
	// to see if the scale needs to be changed
	// When data needs to be sent through sigfox, computes the mean and 
	// the standard deviation on a longer period for ACC and GYR (and MAG)
	if (reset)
	{
		count++;
		for(i = 0; i < 3; i++)
		{
			mean[ACC][i] = calc_mean(sum[ACC][i],
					QUEUE_SIZE);
			std_dev[ACC][i] = calc_std_dev(
					sum_square[ACC][i], 
					QUEUE_SIZE, 
					mean[ACC][i]);
			//adds temp_sums to total_sums and reset them
			for(j = 0; j < 2 + LSM9DS0_MAG_ENABLE; j++)
			{
				sum_total[j][i] += sum[j][i];
				sum_square_total[j][i] += sum_square[j][i];
				sum[j][i] = 0;
				sum_square[j][i] = 0;
			}
		}
		if(change_scale(std_dev[ACC][X], std_dev[ACC][Y], std_dev[ACC][Z]))
			setup_all();

		//Computes total mean and dev on the period
		//Send these values to sigfox
		if(difftime(time(NULL), new_cycle) > SGF_SEND_PERIOD)
		{
			for(i = 0; i < 2 + LSM9DS0_MAG_ENABLE; i++)
			{
				for(j = 0; j < 3; j++)
				{

					tab_data[i][j].mean = calc_mean(
							sum_total[i][j],
							QUEUE_SIZE*count);
					tab_data[i][j].std_dev = calc_std_dev(
							sum_square_total[i][j], 
							QUEUE_SIZE*count, 
							mean[i][j]);
					tab_data[i][j].min = min[i][j];
					tab_data[i][j].max = max[i][j];
					sum_total[i][j] = 0;
					sum_square_total[i][j] = 0;
					min[i][j] = 0;
					max[i][j] = 0;
					pthread_create(&threads[i][j], &attr,
							send_sigfox,
							(void*) &tab_data[i][j]);
				}
			}
			//send to sigfox
			new_cycle = time(NULL);
			count = 0;
		}
		pthread_attr_destroy(&attr);
	}
}


void *acq_GYR_ACC(void * arg)
{
	printf("Thread LSM9D0 created id : %li\n", syscall(__NR_gettid));

	struct timespec tt;
	tt.tv_sec = 0;
	tt.tv_nsec = (long) 1000000000.0/INPUT_DATA_RATE;

	uint8_t i, j, pos = 0;
	uint8_t nb_acq = LSM9DS0_MAG_ENABLE ? 3 : 2; //3 if magnetometer is used
	struct data_acq message_queue[QUEUE_SIZE];

	pthread_t print_thread;

	// Allocate mem to buffer getting data from sensors :
	// (x, y, z) = 3 for (ACC, GYR, (MAG))=2or3 containing a float
	if(!(message_queue[0].data = malloc(nb_acq*QUEUE_SIZE*sizeof(float *)))
			|| !(message_queue[0].data[0] = 
				malloc(nb_acq*QUEUE_SIZE*3*sizeof(float))))
	{
		printf("malloc failed\n");
		return 0;
	}

	for(i = 0; i< QUEUE_SIZE; i++)
	{
		message_queue[i].data = message_queue[0].data +	nb_acq*i;
		for(j = 0; j < nb_acq; j++)
			message_queue[i].data[j] = message_queue[0].data[0] 
				+ (2*i+j)*3;
	}
	/*  
	    message_queue |_________________[0]_______________|____________[1]___________|
	    .data         |____[0]____|_____[1]___|____[2]____|_____[0]___|...|...
	    .data[]       |[0]|[1]|[2]|[0]|[1]|[2]|[0]|[1]|[2]|[0]|[1]|[2]|[0]|...|...


*/
	// Setup the cleanup handlers
	struct acq_cleanup_args args;
	args.alive = arg;
	*args.alive = 1;
	args.buffer = message_queue;
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
		read_all(message_queue[pos].data);
		message_queue[pos].acq_time = time(NULL);
		//ADD to queue
		message_queue[pos].read_allowed = 1;

		if(pos + 1 == QUEUE_SIZE)
		{
			stats(message_queue[pos].data, 1);
			pos = 0;
			printf("mark1\n");
		}
		else
		{
			stats(message_queue[pos].data, 0);
		}
		nanosleep(&tt, NULL);
		pos++;
	}

	pthread_cleanup_pop(1);
	pthread_exit((void *) 1);
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

	//the path to the file saving data is specified in the config
	get_cfg_str(&path, &cfg, 1);
	if(!(fp = fopen(path, "a")))
	{
		printf("Can't open file %s\n", path);
		pthread_exit((void *) 0);
	}

	//Add header 
	fprintf(fp, "TIME, ACC_X, ACC_Y, ACC_Z, GYR_X, GYR_Y, GYR_Z");
	if(LSM9DS0_MAG_ENABLE)
		fprintf(fp, "MAG_X, MAG_Y, MAG_Z, ");
	fprintf(fp, "\n");

	while(!end_program)
	{
		//Try to print at the same rate data is send by i2c
		nanosleep(&tt, NULL);

		while(message_queue[pos].read_allowed == 1)
		{	
			fprintf(fp, "%li,", &message_queue[pos].acq_time);
			fprintf(fp, "%g,%g,%g,%g,%g,%g", 
					message_queue[pos].data[ACC][X],
					message_queue[pos].data[ACC][Y],
					message_queue[pos].data[ACC][Z],
					message_queue[pos].data[GYR][X],
					message_queue[pos].data[GYR][Y],
					message_queue[pos].data[GYR][Z]);
			if(LSM9DS0_MAG_ENABLE)
				fprintf(fp, "%g,%g,%g,",
						message_queue[pos].data[MAG][X],
						message_queue[pos].data[MAG][Y],
						message_queue[pos].data[MAG][Z]);
			fprintf(fp, "\n");
			message_queue[pos].read_allowed = 0;
			//Garder pos entre 0 et QUEUE_SIZE
			pos = pos++ == QUEUE_SIZE ? 0 : pos;
		}

	}
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
