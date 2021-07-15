/**
  ******************************************************************************
  * @file           : i2c_middleware.c
  * @brief          : I2C middleware for this specific project, using HAL
	*										libraries
  ******************************************************************************
  * Imperial College Space Society
	* Medad Newman, Richard Ibbotson
  *
  *
  ******************************************************************************
  */

/* ==================================================================== */
/* ========================== include files =========================== */
/* ==================================================================== */

/* Inclusion of system and local header files goes here */

#include "i2c_middleware.h"
#include "config.h"
#include "stdio.h"
#include "delay.h"
// #include "iwdg.h"
#include "board.h"
#include "i2c.h"

extern I2c_t I2c;
extern Gpio_t Load_enable;
extern Gpio_t i2c_scl;
extern Gpio_t i2c_sda;

/* ==================================================================== */
/* ============================ constants ============================= */
/* ==================================================================== */

/* #define and enum statements go here */

/* ==================================================================== */
/* ======================== global variables ========================== */
/* ==================================================================== */

/* Global variables definitions go here */

/* ==================================================================== */
/* ========================== private data ============================ */
/* ==================================================================== */

/* Definition of private datatypes go here */

/* ==================================================================== */
/* ====================== private functions =========================== */
/* ==================================================================== */

/* Function prototypes for private (static) functions go here */

/* ==================================================================== */
/* ===================== All functions by section ===================== */
/* ==================================================================== */

/* Functions definitions go here, organised into sections */

/* rapidly toggle the i2c lines to get it unstuck
 * Workaround to solve this mysterious problem where the sda line
 * appears to get stuck low.
 */
I2C_MIDDLEWARE_STATUS_t reinit_i2c()
{
	/* Deinit i2c bus */
	I2cDeInit(&I2c);

	/* disable power to GPS */
	GpioWrite(&Load_enable, 1);
	DelayMs(100);

	/* Make I2C bus pins GPIO */
	GpioInit(&i2c_scl, PB_8, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 1);
	GpioInit(&i2c_sda, PB_9, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 1);

	/* set i2c pins low to ensure it cannot power up the core of the GPS */
	GpioWrite(&i2c_scl, 0);
	GpioWrite(&i2c_sda, 0);

	DelayMs(100);

	/* Enable power to GPS */
	GpioWrite(&Load_enable, 0);
	DelayMs(1000);

	/* send 9 clock pulses to the GPS ref: https://www.microchip.com/forums/FindPost/175578 */
	for (uint8_t i = 0; i < 9; i++)
	{
		GpioWrite(&i2c_scl, 0);
		DelayMs(1);
		GpioWrite(&i2c_scl, 1);
		DelayMs(1);
	}

	DelayMs(100);

	I2cInit(&I2c, I2C_1, PB_8, PB_9);

	return I2C_SUCCSS;
}
