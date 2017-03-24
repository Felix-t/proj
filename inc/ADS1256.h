#ifndef ADS1256_H
#define ADS1256_H


#include "headers.h"
#include "util.h"
/*
 * ad_converter.h:
 *	Program launched on startup 
 *	 - Check remaining battery power
 *	 - Schedule next startup and shutdown 
 *	 - Start acquisitions subroutines
 */

//CS      -----   SPICS  
//DIN     -----   MOSI
//DOUT  -----   MISO
//SCLK   -----   SCLK
//DRDY  -----   ctl_IO     data  starting
//RST     -----   ctl_IO     reset



#define  DRDY  RPI_GPIO_P1_11         //P0
#define  RST  RPI_GPIO_P1_12     //P1
#define	SPICS	RPI_GPIO_P1_15	//P3

#define CS_1() bcm2835_gpio_write(SPICS,HIGH)
#define CS_0()  bcm2835_gpio_write(SPICS,LOW)

#define DRDY_IS_LOW()	((bcm2835_gpio_lev(DRDY)==0))

#define RST_1() 	bcm2835_gpio_write(RST,HIGH);
#define RST_0() 	bcm2835_gpio_write(RST,LOW);





typedef enum {FALSE = 0, TRUE = !FALSE} bool;

typedef enum {SINGLE= 0, DIFFERENTIAL = 1} input_type;

/* gain channelî */
typedef enum
{
	ADS1256_GAIN_1			= (0),	/* GAIN   1 */
	ADS1256_GAIN_2			= (1),	/*GAIN   2 */
	ADS1256_GAIN_4			= (2),	/*GAIN   4 */
	ADS1256_GAIN_8			= (3),	/*GAIN   8 */
	ADS1256_GAIN_16			= (4),	/* GAIN  16 */
	ADS1256_GAIN_32			= (5),	/*GAIN    32 */
	ADS1256_GAIN_64			= (6),	/*GAIN    64 */
}ADS1256_GAIN_E;

/* Sampling speed choice*/
/* 
	11110000 = 30,000SPS (default)
	11100000 = 15,000SPS
	11010000 = 7,500SPS
	11000000 = 3,750SPS
	10110000 = 2,000SPS
	10100001 = 1,000SPS
	10010010 = 500SPS
	10000010 = 100SPS
	01110010 = 60SPS
	01100011 = 50SPS
	01010011 = 30SPS
	01000011 = 25SPS
	00110011 = 15SPS
	00100011 = 10SPS
	00010011 = 5SPS
	00000011 = 2.5SPS
*/
typedef enum
{
	ADS1256_30000SPS = 0,
	ADS1256_15000SPS,
	ADS1256_7500SPS,
	ADS1256_3750SPS,
	ADS1256_2000SPS,
	ADS1256_1000SPS,
	ADS1256_500SPS,
	ADS1256_100SPS,
	ADS1256_60SPS,
	ADS1256_50SPS,
	ADS1256_30SPS,
	ADS1256_25SPS,
	ADS1256_15SPS,
	ADS1256_10SPS,
	ADS1256_5SPS,
	ADS1256_2d5SPS,

	ADS1256_DRATE_MAX
}ADS1256_DRATE_E;

#define ADS1256_DRAE_COUNT = 15;

typedef struct
{
        ADS1256_GAIN_E Gain;            /* GAIN  */
        ADS1256_DRATE_E DataRate;       /* DATA output  speed*/
        int32_t AdcNow[8];                      /* ADC  Conversion value */
        uint8_t Channel;                        /* The current channel*/
        input_type ScanMode;       /*Scanning mode,   0  Single-ended input  8 channel£¬ 1 Differential input  4 channel*/
}ADS1256_VAR_T;


ADS1256_VAR_T g_tADS1256;

void  bsp_DelayUS(uint64_t micros);
void ADS1256_StartScan(input_type _ucScanMode, uint8_t ch_num);
void ADS1256_CfgADC(ADS1256_GAIN_E _gain, ADS1256_DRATE_E _drate);
uint8_t ADS1256_ReadChipID(void);

int32_t ADS1256_GetAdc(uint8_t _ch);
void ADS1256_ISR(void);
uint8_t ADS1256_Scan(void);



#endif
