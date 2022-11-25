/*!
 * \file      board.c
 *
 * \brief     Target board general functions implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Alistair Francis <alistair@alistair23.me>
 */
#include "utilities.h"
#include "delay.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "timer.h"
#include "gps.h"
#include "mpl3115.h"
#include "mag3110.h"
#include "mma8451.h"
#include "sx9500.h"
#include "board-config.h"
#include "lpm-board.h"
#include "sx126x-board.h"
#include "board.h"

#include "libtock/rng.h"
#include "libtock/console.h"
#include <stdio.h>
#include <stdlib.h>

void BoardCriticalSectionBegin( uint32_t *mask )
{
}

void BoardCriticalSectionEnd( uint32_t *mask )
{
}

void BoardInitPeriph( void )
{
}

void BoardInitMcu( void )
{
    SX126xIoInit( );
    SX126xIoDbgInit( );
    SX126xIoTcxoInit( );
}

void BoardResetMcu( void )
{
}

void BoardDeInitMcu( void )
{
}

uint32_t BoardGetRandomSeed( void )
{
    uint32_t procid;
    int num_bytes;

    int r = rng_sync((uint8_t *) &procid, 4, 4, &num_bytes);

    if (r != RETURNCODE_SUCCESS || num_bytes != 4) {
        printf("RNG failure\n");
        exit(1);
    }

    return procid;
}

void BoardGetUniqueId( uint8_t *id )
{
    *id = 0xAB;
}

uint16_t BoardBatteryMeasureVoltage( void )
{
    return 0;
}

uint32_t BoardGetBatteryVoltage( void )
{
    return 0;
}

uint8_t BoardGetBatteryLevel( void )
{
    return 0;
}

Version_t BoardGetVersion( void )
{
    Version_t boardVersion = { 0 };

    boardVersion.Fields.Major = 2;
    boardVersion.Fields.Major = 1;

    return boardVersion;
}

void SystemClockConfig( void )
{
}

void SystemClockReConfig( void )
{
}

void SysTick_Handler( void )
{
}

uint8_t GetBoardPowerSource( void )
{
    return USB_POWER;
}

void LpmEnterStopMode( void )
{
    yield();
}

void LpmExitStopMode( void )
{
}

void LpmEnterSleepMode( void )
{
    yield();
}

void BoardLowPowerHandler( void )
{
    yield();
}

int _swiwrite(int fd, const void *buf, uint32_t count)
{
  putnstr((const char*)buf, count);
  return count;
}

#ifdef USE_FULL_ASSERT

#include <stdio.h>

/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
void assert_failed( uint8_t* file, uint32_t line )
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %lu\n", file, line) */

    printf( "Wrong parameters value: file %s on line %lu\n", ( const char* )file, line );
    /* Infinite loop */
    while( 1 )
    {
        yield();
    }
}
#endif
