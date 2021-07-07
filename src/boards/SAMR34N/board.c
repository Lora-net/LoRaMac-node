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
 */
#include "asf.h"

#include "utilities.h"

#include "board-config.h"
#include "delay-board.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "timer.h"
#include "gps.h"
#include "rtc-board.h"
#include "sx1276-board.h"
#include "../board.h"

/*
 * MCU objects
 */
Gpio_t Led0;
Gpio_t Led1;
Gpio_t Button;
Uart_t Uart;
I2c_t  I2c;

/*!
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

void BoardCriticalSectionBegin( uint32_t* mask )
{
    *mask = cpu_irq_save( );
}

void BoardCriticalSectionEnd( uint32_t* mask )
{
    cpu_irq_restore( *mask );
}

void BoardInitPeriph( void )
{
}

#include <stdio.h>
#include "delay.h"

void BoardInitMcu( void )
{
    /* Initialize clock system */
    system_init( );

    delay_init( );

    GpioInit( &Led0, LED_0_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, LED_0_INACTIVE );
    GpioInit( &Led1, LED_1_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, LED_1_INACTIVE );
    GpioInit( &Button, BUTTON_0_PIN, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, BUTTON_0_INACTIVE );

    cpu_irq_enable( );
    // RtcInit( );

    // Uart is fully initialized inside UartInit function
    UartInit( &Uart, UART_1, NC, NC );
    // This function call has no effect on UART configuration. See \ref UartInit implmentation
    UartConfig( &Uart, RX_TX, 921600, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );

    // SPI is fully initialized inside SpiInit function
    // SpiInit( &SX1276.Spi, SPI_1, NC, NC, NC, NC );

    // Radio IO initialization
    SX1276IoInit( );
    SX1276IoDbgInit( );
    SX1276IoTcxoInit( );

    // I2C is fully initialized inside I2cInit function
    // I2cInit( &I2c, I2C_1, NC, NC );

    // Setup EEPROM emulator service
    enum status_code error_code = rww_eeprom_emulator_init( );

    if( error_code == STATUS_ERR_NO_MEMORY )
    {
        while( true )
        {
            /* No EEPROM section has been set in the device's fuses */
        }
    }
    else if( error_code != STATUS_OK )
    {
        /* Erase the emulated EEPROM memory (assume it is unformatted or
         * irrecoverably corrupt) */
        rww_eeprom_emulator_erase_memory( );
        rww_eeprom_emulator_init( );
    }

    McuInitialized = true;

    uint8_t  buffer[256];
    uint16_t nb_bytes_read = 0;

    printf( "Hello world\n" );
    while( 1 )
    {
        if( UartGetBuffer( &Uart, buffer, 256, &nb_bytes_read ) == 0 )
        {
            GpioToggle( &Led0 );
            while( UartPutBuffer( &Uart, buffer, nb_bytes_read ) != 0 )
            {
            };
        }
    }
}

void BoardResetMcu( void )
{
    rww_eeprom_emulator_commit_page_buffer( );

    CRITICAL_SECTION_BEGIN( );

    // Restart system
    NVIC_SystemReset( );
}

void BoardDeInitMcu( void )
{
    SpiDeInit( &SX1276.Spi );
}

uint32_t BoardGetRandomSeed( void )
{
    return 0;
}

void BoardGetUniqueId( uint8_t* id )
{
    // We don't have an ID, so use the one from Commissioning.h
}

uint8_t BoardGetBatteryLevel( void )
{
    return 0;  //  Battery level [0: node is connected to an external power source ...
}

uint8_t GetBoardPowerSource( void )
{
    return USB_POWER;
}

void BoardLowPowerHandler( void )
{
}

#if !defined( __CC_ARM )

/*
 * Function to be used by stdout for printf etc
 */
int _write( int fd, const void* buf, size_t count )
{
    while( UartPutBuffer( &Uart, ( uint8_t* ) buf, ( uint16_t ) count ) != 0 )
    {
    };
    return count;
}

/*
 * Function to be used by stdin for scanf etc
 */
int _read( int fd, const void* buf, size_t count )
{
    size_t bytesRead = 0;
    while( UartGetBuffer( &Uart, ( uint8_t* ) buf, count, ( uint16_t* ) &bytesRead ) != 0 )
    {
    };
    // Echo back the character
    while( UartPutBuffer( &Uart, ( uint8_t* ) buf, ( uint16_t ) bytesRead ) != 0 )
    {
    };
    return bytesRead;
}

#else

#include <stdio.h>

// Keil compiler
int fputc( int c, FILE* stream )
{
    while( UartPutChar( &Uart, ( uint8_t ) c ) != 0 )
        ;
    return c;
}

int fgetc( FILE* stream )
{
    uint8_t c = 0;
    while( UartGetChar( &Uart, &c ) != 0 )
        ;
    // Echo back the character
    while( UartPutChar( &Uart, c ) != 0 )
        ;
    return ( int ) c;
}

#endif

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
     ex: printf("Wrong parameters value: file %s on line %u\n", file, line) */

    printf( "Wrong parameters value: file %s on line %u\n", ( const char* ) file, line );
    /* Infinite loop */
    while( 1 )
    {
    }
}
#endif
