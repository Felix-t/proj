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

typedef struct scale_cfg
{
	float value;
	uint8_t reg_config;
} scale_config;

// Linear Acceleration: mg per LSB
#define SCALE_ACC_2G  (scale_config) {(0.061F), 0b00000000};
#define SCALE_ACC_4G  (scale_config) {(0.122F), 0b00001000};
#define SCALE_ACC_6G  (scale_config) {(0.183F), 0b00010000};
#define SCALE_ACC_8G  (scale_config) {(0.244F), 0b00011000};
#define SCALE_ACC_16G (scale_config) {(0.732F), 0b00100000} ;

/// Magnetic Field Strength: gauss range
#define SCALE_MAG_2GAUSS  (scale_config) {(0.08F), 0b00000000};
#define SCALE_MAG_4GAUSS  (scale_config) {(0.16F), 0b00100000};
#define SCALE_MAG_8GAUSS  (scale_config) {(0.32F), 0b01000000};
#define SCALE_MAG_12GAUSS (scale_config) {(0.48F), 0b01100000};

// Angular Rate: dps per L SB
#define SCALE_GYR_245DPS  (scale_config) {(0.00875F), 0b00000000};
#define SCALE_GYR_500DPS  (scale_config) {(0.01750F), 0b00010000};
#define SCALE_GYR_2000DPS (scale_config) {(0.07000F), 0b00100000};

// Slave address for the i2c communication
#define MAG_ADDRESS            0x1D
#define ACC_ADDRESS            0x1D
#define GYR_ADDRESS            0x6B

#define INPUT_DATA_RATE		50 //Hz	
#define QUEUE_SIZE 		200


struct data_acq{
	int16_t x_acc;
	int16_t y_acc;
	int16_t z_acc;
	int16_t x_gyr;
	int16_t y_gyr;
	int16_t z_gyr;
	int16_t x_mag;
	int16_t y_mag;
	int16_t z_mag;
	uint8_t write;
};

typedef enum {OK, READ_FAIL, WRITE_FAIL, ACC_FAIL, GYR_FAIL, MAG_FAIL, FAIL}  error_code;

enum instrument {ACC,GYR,MAG};

/* Function : This thread saves the data send by the LSM9DO in the file 
 * whose path is specified in config. When end_program is set, finishes 
 * writing last records before quitting
 * Params : arg is a message queue : type struct data_acq[]
*/
void *print_to_file(void * arg);


/* Function : Thread managing the acquisition from the accelerometer,
 * 		gyrometer and (if defined as asuch) magnetometer
 * Setup and config the board, then request and read the data through i2c
 * The data is sent to be saved through a message queue
*/
void *acq_GYR_ACC(void * arg);


/* Function : Set the scale for the specified LSM9D0 instrument
 * 	The hardware needs to be reconfigured with setup() afterwards	
 * Params :  * 	uint8_t instrument : from enum instrument
 * 	scale_config new_scale : new scale struct for the specified instrument
 * 	Possible choices in #define (ex SCALE_ACC...)
 * Return : 0 if instruments (@TODO : or scale) does not exist
 */
uint8_t set_scale(enum instrument inst, scale_config new_scale);


/* Function : Get the accelerometer, gyrometer and magnetometer data
 * Params : int16_t buffer[3][3] : [gyro/magn/acc] [x/y/z]
 * Return : error_code
*/
uint8_t read_all(int16_t *buffer[3]);


/* Function : get the gyrometer data
 * 	See datasheet LSM9DS0 p46 to get info about read registers
 * Params : -int16_t buffer[3] : x,y,z data points.
 * 	    -double scale : 0 or any SCALE_GYR_xxxxDPS : see p.13
 * Return : error_code
*/
uint8_t read_gyrometer(int16_t *buffer);


/* Function : get the magnetometer data
 * 	See datasheet LSM9DS0 p61 to get info about read registers
 * Params : -int16_t buffer[3] : x,y,z data points.
 * 	    -double scale : 0 or any SCALE_MAG_xGAUSS : see p.13
 * Return : error_code
*/
uint8_t read_magnetometer(int16_t *buffer);


/* Function : get the accelerometer data
 * 	See datasheet LSM9DS0 p61 to get info about read registers
 * Params : -int16_t buffer[3] : x,y,z data points.
 * 	    -double scale : 0 or any SCALE_ACC_xG : see p.13
 * Return : error_code
*/
uint8_t read_accelerometer(int16_t *buffer);


/* Function : Configure the LSM9DS0 to start acquisition 
 * Return : error_code
*/
uint8_t setup_all();


/* Function : configure the gyrometer for acquisition
 * 	See datasheet LSM9DS0 p41 42 for register configuration
 * Return : error_code
*/
uint8_t setup_gyrometer();


/* Function : configure the gyrometer for acquisition
 * 	See datasheet LSM9DS0 p 59 60 for register configuration
 * 	This function also enable temperature sensor
 * Return : error_code
*/
uint8_t setup_magnetometer();


/* Function : configure the gyrometer for acquisition
 * 	See datasheet LSM9DS0 p55 56 for register configuration
 * Return : error_code
*/
uint8_t setup_accelerometer();

#endif
