#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H
/*
 * accelerometer.h
 *  Interface for the LSM9DS0 
 *  Give access to register address, scale factors and setup/read functions
 *  bcm2835_init() and bcm2835_i2c_begin() must have been called before
 *  executing anything
 */
#include "headers.h"
#include "cfg.h" 
#include "util.h"
#include "sigfox.h"
#include <math.h>

// LSM9DS0 Gyro Registers 
#define WHO_AM_I_G			0x0F
#define CTRL_REG1_G			0x20
#define CTRL_REG2_G			0x21
#define CTRL_REG3_G			0x22
#define CTRL_REG4_G			0x23
#define CTRL_REG5_G			0x24
#define REFERENCE_G			0x25
#define STATUS_REG_G			0x27
#define OUT_X_L_G			0x28
#define OUT_X_H_G			0x29
#define OUT_Y_L_G			0x2A
#define OUT_Y_H_G			0x2B
#define OUT_Z_L_G			0x2C
#define OUT_Z_H_G			0x2D
#define FIFO_CTRL_REG_G			0x2E
#define FIFO_SRC_REG_G			0x2F
#define INT1_CFG_G			0x30
#define INT1_SRC_G			0x31
#define INT1_THS_XH_G			0x32
#define INT1_THS_XL_G			0x33
#define INT1_THS_YH_G			0x34
#define INT1_THS_YL_G			0x35
#define INT1_THS_ZH_G			0x36
#define INT1_THS_ZL_G			0x37
#define INT1_DURATION_G			0x38

// LSM9DS0 Accel/Magneto (XM) Registers
#define OUT_TEMP_L_XM			0x05
#define OUT_TEMP_H_XM			0x06
#define STATUS_REG_M			0x07
#define OUT_X_L_M			0x08
#define OUT_X_H_M			0x09
#define OUT_Y_L_M			0x0A
#define OUT_Y_H_M			0x0B
#define OUT_Z_L_M			0x0C
#define OUT_Z_H_M			0x0D
#define WHO_AM_I_XM			0x0F
#define INT_CTRL_REG_M			0x12
#define INT_SRC_REG_M			0x13
#define INT_THS_L_M			0x14
#define INT_THS_H_M			0x15
#define OFFSET_X_L_M			0x16
#define OFFSET_X_H_M			0x17
#define OFFSET_Y_L_M			0x18
#define OFFSET_Y_H_M			0x19
#define OFFSET_Z_L_M			0x1A
#define OFFSET_Z_H_M			0x1B
#define REFERENCE_X			0x1C
#define REFERENCE_Y			0x1D
#define REFERENCE_Z			0x1E
#define CTRL_REG0_XM			0x1F
#define CTRL_REG1_XM			0x20
#define CTRL_REG2_XM			0x21
#define CTRL_REG3_XM			0x22
#define CTRL_REG4_XM			0x23
#define CTRL_REG5_XM			0x24
#define CTRL_REG6_XM			0x25
#define CTRL_REG7_XM			0x26
#define STATUS_REG_A			0x27
#define OUT_X_L_A			0x28
#define OUT_X_H_A			0x29
#define OUT_Y_L_A			0x2A
#define OUT_Y_H_A			0x2B
#define OUT_Z_L_A			0x2C
#define OUT_Z_H_A			0x2D
#define FIFO_CTRL_REG			0x2E
#define FIFO_SRC_REG			0x2F
#define INT_GEN_1_REG			0x30
#define INT_GEN_1_SRC			0x31
#define INT_GEN_1_THS			0x32
#define INT_GEN_1_DURATION		0x33
#define INT_GEN_2_REG			0x34
#define INT_GEN_2_SRC			0x35
#define INT_GEN_2_THS			0x36
#define INT_GEN_2_DURATION		0x37
#define CLICK_CFG			0x38
#define CLICK_SRC			0x39
#define CLICK_THS			0x3A
#define TIME_LIMIT			0x3B
#define TIME_LATENCY			0x3C
#define TIME_WINDOW			0x3D


// Linear Acceleration: mg per LSB
#define SCALE_ACC_2G  (scale_config) {(0.061F), 0b00000000};
#define SCALE_ACC_4G  (scale_config) {(0.122F), 0b00001000};
#define SCALE_ACC_6G  (scale_config) {(0.183F), 0b00010000};
#define SCALE_ACC_8G  (scale_config) {(0.244F), 0b00011000};
#define SCALE_ACC_16G (scale_config) {(0.488F), 0b00100000}; // is 0.732F in doc
							     // why

/// Magnetic Field Strength: gauss range
#define SCALE_MAG_2GAUSS  (scale_config) {(0.08F), 0b00000000};
#define SCALE_MAG_4GAUSS  (scale_config) {(0.16F), 0b00010000};
#define SCALE_MAG_8GAUSS  (scale_config) {(0.32F), 0b00100000};
#define SCALE_MAG_12GAUSS (scale_config) {(0.48F), 0b00110000};

// Angular Rate: dps per L SB
#define SCALE_GYR_245DPS  (scale_config) {(0.00875F), 0b00000000};
#define SCALE_GYR_500DPS  (scale_config) {(0.01750F), 0b00010000};
#define SCALE_GYR_2000DPS (scale_config) {(0.07000F), 0b00100000};

// Slave address for the i2c communication
#define MAG_ADDRESS            0x1D
#define ACC_ADDRESS            0x1D
#define GYR_ADDRESS            0x6B

//TTS between each measure : change this depending on the output datarate
//setup in the hardware
#define INPUT_DATA_RATE		100 //Hz	
#define WRITE_DATA_NSEC		8000000.0

// Percentage of range above which the scale needs to be changed
#define UP_SCALE 		0.90

// minimum time between a change of scale and the next possible downscale, in s
#define DOWNSCALE_TIME		60
#define QUEUE_SIZE 		50 // number max of data not printed
#define INTERVAL_CALC_SCALE 	10 //seconds
#define NB_MEASURES_INTERVAL	QUEUE_SIZE*INTERVAL_CALC_SCALE


enum {X, Y, Z};
enum instrument {ACC,GYR,MAG};

typedef struct scale_cfg
{
	float value;
	uint8_t reg_config;
} scale_config;

struct data_acq{
	struct timeval acq_time;
	float **data;
	uint16_t read_allowed;
};
/* message queue is an array of data_acq :  
    message_queue |_________________[0]_______________|____________[1]_______...
    .data         |___[ACC]___|___[GYR]___|___[MAG]___|___[ACC]___|...|...
    []            |[x]|[y]|[z]|[x]|[y]|[z]|[x]|[y]|[z]|[x]|[y]|[z]|...|...
*/

struct acq_cleanup_args{
	pthread_t *print_thread;
	pthread_t *stats_thread;
	struct data_acq *buffer;
	int16_t **data_to_free;
};

struct stats_struct{
	struct data_acq *message_queue;
	_Atomic uint8_t change_scale;
};



/* Function : Thread.
 * 	- Prints the data in the file specified in config (PATH_LSM9DS0_DATA).
 * 	   the file changes every SIZE_MAX_FILE kbytes
 * 	- Exit when end_program is set
 * Params : arg is struct data_acq[QUEUE_SIZE] where data is written and read
*/
void *print_to_file(void * arg);


/* Function : Thread.
 * 	- Manage the acquisition from the accelerometer, gyrometer and 
 * 	    (if defined as such) magnetometer
 * 	- Setup and config the board, then request and read the data through i2c
 * 	  Does not configure i2c communication (i2c is used by other threads or
 * 	    programs) : bcm2835_i2c_begin should be called before creating 
 * 	    the thread
 * 	- Create and fill the struct data_acq[QUEUE_SIZE] used to capture data
 * Params : arg is a _Atomic uint8_t pointer indicating whether the thread is 
 * 		running or not
*/
void *acq_GYR_ACC(void * arg);


/* Function : Thread. 
 * 	- Calculates the mean and standard deviation every INTERVAl_CALC_SCALE 
 * 	seconds, and decides which scale should be used depending
 * 	on these two values (for the accelerometer only).
 *	- Send the minimum, maximum, mean and standard deviation of the 
 *	different LSM9DS0 acquisition to the sigfox thread if it is running,
 *	every SGF_INTERVAL seconds.
 *	- Exit when end_program is set to 1.
 * Params : arg is a struct stats_struct with : 
 * 	- the message queue where the data is written and read
 *	- *change_scale indicates which ACC scale should be used, and is read by
 *	acq_GYR_ACC periodically
 */
void * stats(void * arg);

/* Function : Change the accelerometer, gyrometer or magnetometer sensitivity 
 * and maximum scale by setting the correct registers
 * Params : 
 * 	- inst is one of ACC, GYR or MAG
 * 	- new_scale is one of the struct scale_config defined 
 * 		l.100 to l.117 in accelerometer.h
 * Return : 0 in case of incorrect scale value or failure to write to registers
 * 	1 otherwise
 */
uint8_t set_scale(enum instrument inst, scale_config *new_scale);


/* Function : Get the accelerometer, gyrometer and magnetometer data
 * Params : int16_t buffer[2/3][3] : [ACC/GYR/MAG] [x/y/z]
 * Return : 0 if an error occured, 1 otherwise
*/
uint8_t read_all(int16_t **buffer);


/* Function : Query the data registers for instrument inst and fill buffer with
 * the raw int16_t x, y, z values
 * For info about registers see LSM9DS0 datasheet p.46(acc,mag) and 61(gyr)
 * Params :
 * 	- inst is one of ACC/GYR/MAG
 * 	- buffer is an array of 3 int16_t for x/y/z
 * Return : 0 if a read error occured
 */
uint8_t read_data(enum instrument inst, int16_t *buffer);


/* Function : Configure the LSM9DS0 with defaults scale values
 * 	- accelerometer : max range 4000.0mg, sensitivity 0.122mg
 * 	- gyrometer : max range 245.0dps, sensitivity 0.00875 dps
 * 	- magnetometer : max range 2 gauss, sensitivity 0.08 gauss
 * Return : 0 if an error while setting up the registers occured, 1 otherwise
 */
uint8_t setup_all();


/* Function : configure the gyrometer for acquisition
 * 	See datasheet LSM9DS0 p41 42 for register configuration
 * Params : scale is one of the struct scale_config defined l.115 to l.117 
 * Return : 0 if error when writing to registers, 1 otherwise
*/
uint8_t setup_gyrometer(scale_config *scale);


/* Function : configure the gyrometer for acquisition
 * 	See datasheet LSM9DS0 p 59 60 for register configuration
 * Params : scale is one of the struct scale_config defined l.108 to l.112
 * Return : 0 if error when writing to registers, 1 otherwise
*/
uint8_t setup_magnetometer(scale_config *scale);


/* Function : configure the gyrometer for acquisition
 * 	See datasheet LSM9DS0 p55 56 for register configuration
 * Params : scale is one of the struct scale_config defined l.100 to l.105 
 * Return : 0 if error when writing to registers, 1 otherwise
*/
uint8_t setup_accelerometer(scale_config *scale);

#endif
