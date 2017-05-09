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
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>

static uint8_t writeReg(uint8_t device_address, uint8_t reg, uint8_t *value, size_t length);
static uint8_t readReg(uint8_t device_address, uint8_t reg, uint8_t *value, size_t length);
static uint8_t get_scale(enum instrument inst, scale_config *scale);
static void acq_cleanup(void *arg);
static uint8_t change_acc_scale(double *mean, double *std_dev);
static inline float calc_mean(double sum, int nb);
static inline float calc_std_dev(double sum, int nb, float mean);
static void * stats(void * args);
static uint8_t get_float_data(int16_t **data, float **buffer, uint8_t new_scale);
static uint8_t add_header(FILE *fp);
static uint8_t open_new_file(FILE **fp);


/* Function : parse the data from the LSM9DS0 registers to get readable values
 * Params : 
 * 	-uint8_t *data : contains the 6 bytes of raw data taken from the LSM9DS0 
 * 	-uint8_t *buffer : 3*16bits signed containing the correct values
 * 	-uint8_t new_scale : 0 if lookup scale register, ACC scale otherwise
 * Return : error_code
 */
static uint8_t get_float_data(int16_t **data, float **buffer, uint8_t new_scale)
{
	uint8_t i, j;
	uint8_t nb_acq = 2 + LSM9DS0_MAG_ENABLE;

	static scale_config scale[3];
	
	for(j = 0; j < nb_acq; j++)
	{
		if(new_scale != 0 && !get_scale(j, &scale[j]))
			return 0;
		for(i = 0; i < 3;i++)
		{
			buffer[j][i] = scale[j].value*data[j][i];
		}
	}
	return 1;
}


/* Function : Change the scale if the data >80% current scale or  <20% current
 * scale, to prevent/reduce overflow
 * Params :x, y, z are the accelerometer average or values
 * Return : 0 if no change needed, 1 otherwise
 */
static uint8_t change_acc_scale(double *mean, double *std_dev)
{
	double x = fabs(mean[0]) + std_dev[0];
	double y = fabs(mean[1]) + std_dev[1];
	double z = fabs(mean[2]) + std_dev[2];

	double range[5] = {2000.0,4000.0,6000.0,8000.0,16000};
	uint8_t i;

	for(i = 0; i < 4; i++)
	{
		if(x < range[i]*UP_SCALE 
				&& y < range[i]*UP_SCALE 
				&& z < range[i]*UP_SCALE)
			return i+1;
	}
	return 0;
	/*
	get_scale(ACC, &new_scale);
	switch (new_scale.reg_config)
	{
		case 0:
			range = 2000.0;
			break;
		case 8:
			range = 4000.0;
			break;
		case 16:
			range = 6000.0;
			break;
		case 24:
			range = 8000.0;
			break;
		case 32:
			range = 16000.0;
			break;
	}
	if(new_scale.reg_config != 32 
			&& ( x > range*UP_SCALE 
			|| y > range*UP_SCALE 
			|| z > range*UP_SCALE))
		return (new_scale.reg_config >> 3) + 2;
	else if(new_scale.reg_config != 0 
			&& x < range*DOWN_SCALE 
			&& y < range*DOWN_SCALE 
			&& z < range*DOWN_SCALE)
		return (new_scale.reg_config >> 3);
	else 
		return 0;

	set_scale(ACC, &new_scale);
	*/
	//return 1;
}


/* Function : write something in a register through i2c
 * 	See datasheet LSM9DS0 p33 for register adress and write cmd info
 * Params : 
 * 	-uint8_t device_address : Slave address of the i2c device
 * 	-uint8_t reg: register to write to. if multiple writes, 1st register add
 * 	-uint8_t *value: array of bytes to be written
 * Return :error_code
 */
static uint8_t writeReg(uint8_t device_address, uint8_t reg, uint8_t *value, size_t length)
{
	uint32_t i;
	uint8_t toSend[length+1];
	uint8_t return_code;

	// If MSB of the reg adress = 1, multiple write
	if (length > 1)
		reg |= 128;

	toSend[0] = reg;
	for(i = 0;i<length;i++)
		toSend[i+1] = value[i];

	bcm2835_i2c_setSlaveAddress(device_address);
	return_code = bcm2835_i2c_write((char *)toSend, length+1);
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
		default:
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
static uint8_t readReg(uint8_t device_address, uint8_t reg, uint8_t *value, size_t length)
{
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
			// If error, fill the data with 0 : otherwise the values
			// may be the same as the second before
			memset(value, 0, 6);
			return 0;
			break;
		case BCM2835_I2C_REASON_OK:
			//printf("Success\n");
			break;
		default:
			break;
	}
	return 1;
}


/* Function : Initialize and return a pointer indicating current scales
 * Return : array of scale_config[ACC/GYR/MAG]
 */
static uint8_t get_scale(enum instrument inst, scale_config *scale)
{
	if(inst != ACC && inst != GYR && inst != MAG)
		return 0;
	switch(inst){
		case ACC:
			if(!readReg(ACC_ADDRESS, CTRL_REG2_XM, &scale->reg_config, 1))
				return 0;
			scale->value = 0.061 * (1 + (scale->reg_config >> 3));
			if(scale->reg_config == 0b00100000)
				scale->value = 0.488;
			break;
		case GYR:
			if(!readReg(GYR_ADDRESS, CTRL_REG4_G, &scale->reg_config, 1))
				return 0;
			scale->value = 0.00875 * (1 + (scale->reg_config >> 4));
			if(scale->reg_config == 0b00100000)
				scale->value = 0.07000;
			break;
		case MAG:
			if(!readReg(MAG_ADDRESS, CTRL_REG6_XM, &scale->reg_config, 1))
				return 0;
			scale->value = 0.16 * (scale->reg_config >> 4);
			if(scale->reg_config == 0)
				scale->value = 0.08;
			break;
	};
	return 1;
}


static void acq_cleanup(void *arg)
{	
	printf("\tLSM9DS0 cleanup routine\n");
	struct acq_cleanup_args *ptr = arg;

	pthread_join(*(ptr->print_thread), NULL);
	pthread_join(*(ptr->stats_thread), NULL);
	free(ptr->data_to_free[0]);
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


uint8_t set_scale(enum instrument inst, scale_config *new_scale)
{
	if(inst != ACC || inst != GYR || inst != MAG)
		return 0;
	switch(inst){
		case ACC:
			writeReg(ACC_ADDRESS, CTRL_REG2_XM, &new_scale->reg_config, 1);
			break;
		case GYR:
			writeReg(GYR_ADDRESS, CTRL_REG4_G, &new_scale->reg_config, 1);
			break;
		case MAG:
			writeReg(MAG_ADDRESS, CTRL_REG6_XM, &new_scale->reg_config, 1);
			break;
	};
	return 1;
}


uint8_t read_all(int16_t **buffer)
{
	//printf("1\n");
	if (!read_data(ACC, buffer[ACC]))
	{
		printf("Accelerometer read error\n");
		return 0;
	}
	//printf("2\n");
	if (!read_data(GYR, buffer[GYR]))
	{
		printf("Gyrometer read error\n");
		return 0;
	}
	//printf("3\n");
	if (LSM9DS0_MAG_ENABLE && !read_data(MAG, buffer[MAG]))
	{
		printf("Magnetometer read error\n");
		return 0;
	}
	//printf("4\n");

	return 1;
}

static inline int16_t convert_reg_to_int16(uint8_t reg1, uint8_t reg2)
{
	int16_t res = (reg1 | (reg2 << 8));
	return res;
}

uint8_t read_data(enum instrument inst, int16_t *buffer)
{
	uint8_t add, reg;
	uint8_t data[6];

	switch(inst){
		case ACC:
			add = ACC_ADDRESS;
			reg = OUT_X_L_A;
			break;
		case GYR:
			add = GYR_ADDRESS;
			reg = OUT_X_L_G;
			break;
		case MAG:
			add = MAG_ADDRESS;
			reg = OUT_X_L_M;
			break;
	};
	
	// 6 registers containing data : 8msb x, 8 lsb x, 8 msb y, ...
	if(!readReg(add, reg, data, 6))
	{
		return 0;
	}
		buffer[0] = convert_reg_to_int16(data[0], data[1]);
	buffer[1] = convert_reg_to_int16(data[2], data[3]);
	buffer[2] = convert_reg_to_int16(data[4], data[5]);
	return 1;
}


uint8_t setup_all()
{
	scale_config scale = SCALE_ACC_4G;
	if (!setup_accelerometer(&scale))
	{
		printf("Accelerometer initialization error\n");
		return 0;
	}
	scale = SCALE_GYR_245DPS;
	if (!setup_gyrometer(&scale))
	{
		printf("Gyrometer initialization error\n");
		return 0;
	}
	scale = SCALE_MAG_2GAUSS;
	if (LSM9DS0_MAG_ENABLE && !setup_magnetometer(&scale))
	{
		printf("Magnetometer initialization error\n");
		return 0;
	}

	return 1;
}


uint8_t setup_accelerometer(scale_config *scale)
{
	uint8_t config_reg;

	// continuous update & 50Hz data rate - z,y,x axis enabled, 
	config_reg = 0b01010111;	       
	if(!writeReg(ACC_ADDRESS, CTRL_REG1_XM, &config_reg, 1))
	{
		return 0;
	}
	// Bandwidth anti-alias 773Hz, 16G scale, self test normal mode
	if(!writeReg(ACC_ADDRESS, CTRL_REG2_XM, &scale->reg_config, 1))
	{
		return 0;
	}

	return 1;
}


uint8_t setup_gyrometer(scale_config *scale)
{
	uint8_t config_reg[5] = {0,0,0,0,0};
	
	// 95 Hz output datarate, 12.5 cutoff - normal mode - z,y,x axis enabled 
	config_reg[0] = 0b00001111;
	if(!writeReg(GYR_ADDRESS, CTRL_REG1_G, &config_reg[0], 1))
	{
		return 0;
	}

	// Continuous update, scale xdps
	if(!writeReg(GYR_ADDRESS, CTRL_REG4_G, &scale->reg_config, 1))
	{
		return 0;
	}
	
	if(!writeReg(GYR_ADDRESS, CTRL_REG2_G, &config_reg[1], 4))
	{
		return 0;
	}

	return 1;
}


uint8_t setup_magnetometer(scale_config *scale)
{
	uint8_t config_reg;

	// temperature enable - Mag high res - 50 Hz output datarate
	config_reg = 0b11110000;
	if(!writeReg(MAG_ADDRESS, CTRL_REG5_XM, &config_reg, 1))
		return 0;

	// Magnetic full scale
	if(!writeReg(MAG_ADDRESS, CTRL_REG6_XM, &scale->reg_config, 1))
		return 0;

	// mode continuous for magnetic sensor (enable mag sensor) 
	config_reg = 0;    
	if(!writeReg(MAG_ADDRESS, CTRL_REG7_XM, &config_reg, 1))
		return 0;
	return 1;
}


static void * stats(void * args)
{
	sleep(1);
	printf("Thread stats lsm9ds0 created id : %li\n", syscall(__NR_gettid));

	int32_t i = 0, j = 0, pos = 0;

	struct stats_struct *stats_args = (struct stats_struct*) args;

	_Atomic uint8_t *new_scale = &stats_args->change_scale;
	struct data_acq *message_queue = stats_args->message_queue;

	struct sgf_data tab_data[2+LSM9DS0_MAG_ENABLE][3];
	pthread_t threads[6 + 3*LSM9DS0_MAG_ENABLE];

	double sum[2+LSM9DS0_MAG_ENABLE][3] = {{0}, {0}};
	double sum_square[2+LSM9DS0_MAG_ENABLE][3] = {{0}, {0}};
	double sum_total[2+LSM9DS0_MAG_ENABLE][3] = {{0}, {0}};
	double sum_square_total[2+LSM9DS0_MAG_ENABLE][3] = {{0}, {0}};

	double min[2+LSM9DS0_MAG_ENABLE][3] = {{0}, {0}};
	double max[2+LSM9DS0_MAG_ENABLE][3] = {{0}, {0}};
	double mean[2+LSM9DS0_MAG_ENABLE][3] = {{0}, {0}};
	double std_dev[2+LSM9DS0_MAG_ENABLE][3] = {{0}, {0}};

	printf("Mark 7");
	for(i = 0;i<6+LSM9DS0_MAG_ENABLE*3;i++)
		min[i/3][i%3] = 16000.0;
	uint32_t count = 0;
	time_t new_cycle = time(NULL);

	struct timespec tt = {
		.tv_sec = 0,
		.tv_nsec = (long) 18000000.0
	};

	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	printf("Mark 8");

	sleep(1);

	while(!end_program)
	{
		//Try to print at the same rate data is send by i2c
		nanosleep(&tt, NULL);

		if(message_queue[pos].read_allowed == 0)
			continue;

		count++;

		// Add new data to the sums, actualize min and max if needed
		for (i = 0; i < 2+LSM9DS0_MAG_ENABLE; i++)
		{
			for(j = 0; j < 3; j++)
			{
				if(message_queue[pos].data[i][j] > max[i][j])
					max[i][j] = message_queue[pos].data[i][j];
				else if(message_queue[pos].data[i][j] < min[i][j])
					min[i][j] = message_queue[pos].data[i][j];
				sum[i][j] += message_queue[pos].data[i][j];
				sum_square[i][j] += message_queue[pos].data[i][j] *
					message_queue[pos].data[i][j];
			}
		}

		// Every X measures (interval(sec)*input_rate(measures/sec)) :
		//   -Calculate the standard deviation for ACC
		//   -See if the scale needs to be changed
		//   -Send data to sigfox
		if (count%(INPUT_DATA_RATE*INTERVAL_CALC_SCALE) == 0)
		{
			printf("\n\t Change scale\n");
			for(i = 0; i < 3; i++)
			{
				mean[ACC][i] = calc_mean(sum[ACC][i],
						INPUT_DATA_RATE*INTERVAL_CALC_SCALE);
				std_dev[ACC][i] = calc_std_dev(
						sum_square[ACC][i], 
						INPUT_DATA_RATE*INTERVAL_CALC_SCALE, 
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
			*new_scale = change_acc_scale(mean[ACC], std_dev[ACC]);

			//Computes total mean and dev on the period
			//Send these values to sigfox
			if(difftime(time(NULL), new_cycle) > SGF_SEND_PERIOD)
			{
				printf("\n\tStats send sgf\n");
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
						if(SGF_ENABLE)
							pthread_create(&threads[3*i + j], &attr,
								send_sigfox,
								(void*) &tab_data[i][j]);
					}
				}
				//send to sigfox
				new_cycle = time(NULL);
				count = 0;
			}
		}
		pos = (pos + 1) % QUEUE_SIZE;
	}
	pthread_attr_destroy(&attr);
	sleep(1); // Wait before delete tab_data if a thread send_sigfox still needs it
	pthread_exit((void *) 1);
}


void *acq_GYR_ACC(void * arg)
{
	printf("Thread LSM9D0 created id : %li\n", syscall(__NR_gettid));

	struct timespec tt = {
		.tv_sec = 0,
		.tv_nsec = (long) 18000000.0
	};

	scale_config acc_scale = SCALE_ACC_4G;
	uint8_t i = 0, j = 0, k = 0, pos = 0;
	uint8_t nb_acq = LSM9DS0_MAG_ENABLE ? 3 : 2; //3 if magnetometer is used
	int16_t* data[nb_acq];

	pthread_t print_thread, stats_thread;

	struct data_acq message_queue[QUEUE_SIZE] ={
		{.read_allowed = 0}
	};


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
		{
			message_queue[i].data[j] = message_queue[0].data[0] 
				+ (2*i+j)*3;
			for(k = 0; k < 3; k++)
				message_queue[i].data[j][k] = 0;
		}
	}

	/*  
	    message_queue |_________________[0]_______________|____________[1]_______...
	    .data         |____[0]____|_____[1]___|____[2]____|_____[0]___|...|...
	    []            |[0]|[1]|[2]|[0]|[1]|[2]|[0]|[1]|[2]|[0]|[1]|[2]|[0]|...|...

*/
	uint8_t scale = 2; 

	data[0] = malloc(nb_acq*3*sizeof(int16_t));
	for(i = 1; i < nb_acq; i++)
		data[i] = data[0] + 3*i;

	if(!bcm2835_i2c_begin())
		pthread_exit((void *) 0);

	printf("Mark 1");
	// Setup the cleanup handlers
	struct acq_cleanup_args args;
	args.alive = arg;
	*args.alive = 1;
	args.buffer = message_queue;
	args.data_to_free = data;
	args.print_thread = &print_thread;
	args.stats_thread = &stats_thread;
	pthread_cleanup_push(acq_cleanup, &args);

	struct stats_struct stats_args = {
		.message_queue = message_queue,
		.change_scale = 0
	};

	printf("Mark 2");
	// Setup i2c, then setup ctrl registers of the lsm9ds0, create thread
	if(!setup_all() || pthread_create(&print_thread, NULL,	print_to_file,
				(void *) message_queue)
			|| pthread_create(&stats_thread, NULL, stats,
			       	(void *) &stats_args))
	{
		printf("Exiting acq_Gyr_Acc thread\n");
		pthread_exit((void *) 0);
	}

	sleep(10);

	get_float_data(data, message_queue[pos].data, 1);

	uint8_t tmp;
	readReg(GYR_ADDRESS, CTRL_REG2_G, &tmp, 1);
	tmp &= 0b01111111;

	printf("Mark 3");
	// main thread loop
	while(!end_program)
	{
		nanosleep(&tt, NULL);

		message_queue[(pos + 1) % QUEUE_SIZE].read_allowed = 0;

		if (stats_args.change_scale != scale)
		{
			scale = stats_args.change_scale;
			printf("\n\tChange scale in main loop : new scale : %i\n", scale);
			acc_scale.reg_config = (scale - 1) << 3;
			acc_scale.value = 0.061 * scale;
			if(scale == 5)
				acc_scale.value = 0.488;
			setup_accelerometer(&acc_scale);
			get_float_data(data, message_queue[pos].data, scale);
			nanosleep(&tt, NULL);
		}
		
		read_all(data);

		gettimeofday(&message_queue[pos].acq_time, NULL);

		get_float_data(data, message_queue[pos].data, 0);
		//ADD to queue
		message_queue[pos].read_allowed = 1;
		pos = (pos + 1) % QUEUE_SIZE;
	}

	pthread_cleanup_pop(1);
	pthread_exit((void *) 1);
}

static uint8_t open_new_file(FILE **fp)
{
	printf("Mark 4");
	if(*fp != NULL)
		fclose(*fp);

	static DIR* FD;
	struct dirent* in_file;

	static int file_count = 0;
	static char *path_base = NULL;
	char *path = malloc(100);

	// Path specified in the config, add a suffixe to it
	if(path_base == NULL)
	{
		path_base = malloc(100);
		char *cfg = "PATH_LSM9DS0_DATA";
		get_cfg(&path_base, &cfg, 1);
		if(NULL == (FD = opendir(path_base)))
		{
			printf("Can't open directory, try creating it");
			mkdir(path_base, S_IRWXU | S_IRWXG | S_IRWXO);
			if(NULL == (FD = opendir(path_base)))
			{
				printf("Error opening output directory");
				return 0;
			}
		}
		while ((in_file = readdir(FD))) 
		{
			sprintf(path, "data_%i", file_count);
			if (!strcmp(in_file->d_name, path))
			{
				printf("in file : %s\t d_name : %s\n", in_file->d_name, path);
				file_count++;
				rewinddir(FD);
			}
		}
		closedir(FD);
	}

	sprintf(path, "%sdata_%i", path_base, file_count++);

	if(!(*fp = fopen(path, "w+")))
	{
		printf("Can't open file %s\n", path);
		return 0;
	}

	printf("\tSaving accelerometer data to file : %s\n", path);
	printf("Mark 5");

	free(path);
	return 1;
}

static uint8_t add_header(FILE *fp)
{
	fprintf(fp, "TIME, ACC_X, ACC_Y, ACC_Z, GYR_X, GYR_Y, GYR_Z");
	if(LSM9DS0_MAG_ENABLE)
		fprintf(fp, ", MAG_X, MAG_Y, MAG_Z");
	fprintf(fp, "\n");
	return 1;
}

void *print_to_file(void * arg)
{
	printf("Thread saving to file created, id : %li\n",
			syscall(__NR_gettid));

	struct timespec tt;
	tt.tv_sec = 0;
	tt.tv_nsec = (long) 18000000.0;

	char *time_string = malloc(100*sizeof(char));
	int a=0;
	FILE *fp = NULL;
	//transfer from i2c thread to print thread is done through a message
	//queue which contains up to QUEUE_SIZE records. 
	// This thread reads data when message_queue[i].read_allowed var is set to 1.
	struct data_acq *message_queue = (struct data_acq*) arg;
	uint8_t pos = 0;

	open_new_file(&fp);
	add_header(fp);
	printf("Mark 6");

	sleep(1);
	while(!end_program)
	{
		if(ftell(fp) > SIZE_MAX_FILE*1024)
		{
			open_new_file(&fp);
			add_header(fp);
		}


		//Try to print at the same rate data is send by i2c
		nanosleep(&tt, NULL);

		if(message_queue[pos].read_allowed == 0)
			continue;

		strcpy(time_string, ctime(&message_queue[pos].acq_time.tv_sec));
		a = strlen(time_string);
		sprintf(time_string, "%s %uus", 
				ctime(&message_queue[pos].acq_time.tv_sec),
				(unsigned int) message_queue[pos].acq_time.tv_usec/1000);
		time_string[a-1] = ' ';
		fprintf(fp, "%s, ", time_string);
		fprintf(fp, "%g, %g, %g, %g, %g, %g", 
				message_queue[pos].data[ACC][X],
				message_queue[pos].data[ACC][Y],
				message_queue[pos].data[ACC][Z],
				message_queue[pos].data[GYR][X],
				message_queue[pos].data[GYR][Y],
				message_queue[pos].data[GYR][Z]);


		if(LSM9DS0_MAG_ENABLE)
			fprintf(fp, ",%g,%g,%g,",
					message_queue[pos].data[MAG][X],
					message_queue[pos].data[MAG][Y],
					message_queue[pos].data[MAG][Z]);
		fprintf(fp, "\n");
		fflush(fp);
		
		//Garder pos entre 0 et QUEUE_SZE
		pos = (pos + 1) % QUEUE_SIZE;
	}
	free(time_string);
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
