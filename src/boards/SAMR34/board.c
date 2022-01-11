/*!
 * \file  board.c
 *
 * \brief Target board general functions implementation
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 * Copyright TWTG on behalf of Microchip/Atmel (c)2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <peripheral_clk_config.h>
#include <hal_init.h>
#include <hal_delay.h>
#include <hal_timer.h>
#include <hal_spi_m_sync.h>
#include <hal_usart_sync.h>
#include <hpl_rtc_base.h>
#include "board-config.h"
#include "utilities.h"
#include "delay.h"
#include "gpio.h"
#include "adc.h"
#include "spi.h"
#include "i2c.h"
#include "uart.h"
#include "timer.h"
#include "gps.h"
#include "rtc-board.h"
#include "radio_board.h"
#include "board.h"

/*
 * MCU objects
 */
Gpio_t Led1;
Uart_t Uart1;
I2c_t  I2c;

/*!
 * Flag to indicate if the MCU is Initialized
 */
static bool McuInitialized = false;

void BoardCriticalSectionBegin( uint32_t* mask )
{
    *mask = __get_PRIMASK( );
    __disable_irq( );
}

void BoardCriticalSectionEnd( uint32_t* mask )
{
    __set_PRIMASK( *mask );
}

void BoardInitPeriph( void )
{
    GpioInit( &Led1, LED_1, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void BoardInitMcu( void )
{
    init_mcu( );
    delay_init( SysTick );

    hri_gclk_write_PCHCTRL_reg( GCLK, EIC_GCLK_ID, CONF_GCLK_EIC_SRC | ( 1 << GCLK_PCHCTRL_CHEN_Pos ) );
    hri_mclk_set_APBAMASK_EIC_bit( MCLK );

    RtcInit( );

    UartInit( &Uart1, UART_1, UART_TX, UART_RX );
    UartConfig( &Uart1, RX_TX, 921600, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL );

    radio_context_t* radio_context = radio_board_get_radio_context_reference( );
    SpiInit( &radio_context->spi, SPI_1, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );
    radio_board_init_io( );

    I2cInit( &I2c, I2C_1, I2C_SCL, I2C_SDA );

    McuInitialized = true;
    radio_board_init_dbg_io( );
}

void BoardResetMcu( void )
{
    CRITICAL_SECTION_BEGIN( );

    // Restart system
    NVIC_SystemReset( );
}

void BoardDeInitMcu( void )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );
    SpiDeInit( &radio_context->spi );
    radio_board_deinit_io( );
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
    __disable_irq( );
    /*!
     * If an interrupt has occurred after __disable_irq( ), it is kept pending
     * and cortex will not enter low power anyway
     */

    // Call low power handling function.

    __enable_irq( );
}

#if !defined( __CC_ARM )

/*
 * Function to be used by stdout for printf etc
 */
int _write( int fd, const void* buf, size_t count )
{
    while( UartPutBuffer( &Uart1, ( uint8_t* ) buf, ( uint16_t ) count ) != 0 )
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
    while( UartGetBuffer( &Uart1, ( uint8_t* ) buf, count, ( uint16_t* ) &bytesRead ) != 0 )
    {
    };
    // Echo back the character
    while( UartPutBuffer( &Uart1, ( uint8_t* ) buf, ( uint16_t ) bytesRead ) != 0 )
    {
    };
    return bytesRead;
}

#else

#include <stdio.h>

// Keil compiler
int fputc( int c, FILE* stream )
{
    while( UartPutChar( &Uart1, ( uint8_t ) c ) != 0 )
        ;
    return c;
}

int fgetc( FILE* stream )
{
    uint8_t c = 0;
    while( UartGetChar( &Uart1, &c ) != 0 )
        ;
    // Echo back the character
    while( UartPutChar( &Uart1, c ) != 0 )
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
