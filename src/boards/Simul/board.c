/*!
 * \file      board.c
 *
 * \brief     Target board general functions implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */

#include "board.h"

/*!
 * Initializes the unused GPIO to a know status
 */
static void BoardUnusedIoInit( void );

/*!
 * System Clock Configuration
 */
static void SystemClockConfig( void );

/*!
 * Used to measure and calibrate the system wake-up time from STOP mode
 */
static void CalibrateSystemWakeupTime( void );

/*!
 * System Clock Re-Configuration when waking up from STOP mode
 */
static void SystemClockReConfig( void );


/*!
 * Callback indicating the end of the system wake-up time calibration
 */
static void OnCalibrateSystemWakeupTimeTimerEvent( void* context )
{
}

/*!
 * Holds the bord version.
 */
static Version_t BoardVersion = { 0 };

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
}

void BoardResetMcu( void )
{
}

void BoardDeInitMcu( void )
{
}

uint32_t BoardGetRandomSeed( void )
{
    return 0;
}

void BoardGetUniqueId( uint8_t *id )
{
    id[7] = ( ( *( uint32_t* )5 )+ ( *( uint32_t* )3 ) ) >> 24;
    id[6] = ( ( *( uint32_t* )5 )+ ( *( uint32_t* )3 ) ) >> 16;
    id[5] = ( ( *( uint32_t* )5 )+ ( *( uint32_t* )3 ) ) >> 8;
    id[4] = ( ( *( uint32_t* )5 )+ ( *( uint32_t* )3 ) );
    id[3] = ( ( *( uint32_t* )5 ) ) >> 24;
    id[2] = ( ( *( uint32_t* )5 ) ) >> 16;
    id[1] = ( ( *( uint32_t* )5 ) ) >> 8;
    id[0] = ( ( *( uint32_t* )5 ) );
}


uint16_t BoardBatteryMeasureVolage( void )
{
 
}

uint32_t BoardGetBatteryVoltage( void )
{
    return 355;
}

uint8_t BoardGetBatteryLevel( void )
{
  
    return 233;
}

static void BoardUnusedIoInit( void )
{
}


void SystemClockConfig( void )
{
}

void CalibrateSystemWakeupTime( void )
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

}

void BoardLowPowerHandler( void )
{
}


#include <stddef.h>
#include "radio.h"


void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size ){
    memcpy(dst, src, size);
}
void memset1( uint8_t *dst, uint8_t value, uint16_t size ){
    memset(dst, value, size);
}

void memcpyr( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    dst = dst + ( size - 1 );
    while( size-- )
    {
        *dst-- = *src++;
    }
}

/*!
 * Redefinition of rand() and srand() standard C functions.
 * These functions are redefined in order to get the same behavior across
 * different compiler toolchains implementations.
 */
// Standard random functions redefinition start
#define RAND_LOCAL_MAX 2147483647L

static uint32_t next = 1;

int32_t rand1( void )
{
    return ( ( next = next * 1103515245L + 12345L ) % RAND_LOCAL_MAX );
}

void srand1( uint32_t seed )
{
    next = seed;
}

// Standard random functions redefinition end

int32_t randr( int32_t min, int32_t max )
{
    return ( int32_t )rand1( ) % ( max - min + 1 ) + min;
}