/*!
 * \file      sx1261mbxbas-board.c
 *
 * \brief     Target board SX1261MBXBAS shield driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Alistair Francis <alistair@alistair23.me>
 */
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "utilities.h"
#include "board-config.h"
#include "board.h"
#include "delay.h"
#include "radio.h"
#include "sx126x-board.h"
#include "gpio-board.h"

#include "libtock/gpio.h"
#include "libtock/lora_phy.h"

/*!
 * \brief Holds the internal operating mode of the radio
 */
static RadioOperatingModes_t OperatingMode;

void SX126xIoInit( void )
{
    GpioInit( &SX126x.Spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &SX126x.BUSY, RADIO_BUSY, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &SX126x.DIO1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void SX126xIoIrqInit( DioIrqHandler dioIrq )
{
    GpioSetInterrupt( &SX126x.DIO1, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, dioIrq );
}

void SX126xIoDeInit( void )
{
    GpioInit( &SX126x.Spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &SX126x.BUSY, RADIO_BUSY, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &SX126x.DIO1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void SX126xIoDbgInit( void )
{
}

void SX126xIoTcxoInit( void )
{
}

uint32_t SX126xGetBoardTcxoWakeupTime( void )
{
    return BOARD_TCXO_WAKEUP_TIME;
}

void SX126xIoRfSwitchInit( void )
{
    SX126xSetDio2AsRfSwitchCtrl( true );
}

RadioOperatingModes_t SX126xGetOperatingMode( void )
{
    return OperatingMode;
}

void SX126xSetOperatingMode( RadioOperatingModes_t mode )
{
}

void SX126xReset( void )
{
    DelayMs( 10 );
    GpioInit( &SX126x.Reset, RADIO_RESET, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioMcuWrite(&SX126x.Reset, 0);
    DelayMs( 20 );
    GpioMcuWrite(&SX126x.Reset, 1);
    DelayMs( 10 );
}

void SX126xWaitOnBusy( void )
{
    while( GpioRead( &SX126x.BUSY ) == 1 );
}

void SX126xWakeup( void )
{
    char wbuf[2] = {RADIO_GET_STATUS, 0};
    char rbuf[2];

    lora_phy_read_write_sync(wbuf, rbuf, 2);

    // Wait for chip to be ready.
    SX126xWaitOnBusy( );

    // Update operating mode context variable
    SX126xSetOperatingMode( MODE_STDBY_RC );
}

void SX126xWriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    char wbuf[size + 1];

    SX126xCheckDeviceReady( );

    bzero(wbuf, size + 1);
    wbuf[0] = command;

    for( uint16_t i = 0; i < size; i++ )
    {
        wbuf[1 + i] = buffer[i];
    }

    lora_phy_write_sync(wbuf, size + 1);

    if( command != RADIO_SET_SLEEP )
    {
        SX126xWaitOnBusy( );
    }
}

uint8_t SX126xReadCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    char wbuf[size + 2];
    char rbuf[size + 2];
    uint8_t status = 0;

    SX126xCheckDeviceReady( );

    bzero(wbuf, size + 2);
    wbuf[0] = command;
    wbuf[1] = 0x00; // NOP

    lora_phy_read_write_sync(wbuf, rbuf, size + 2);

    status = rbuf[1];

    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = rbuf[i + 2];
    }

    SX126xWaitOnBusy( );

    return status;
}

void SX126xWriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    char wbuf[size + 3];

    SX126xCheckDeviceReady( );

    bzero(wbuf, size + 3);
    wbuf[0] = RADIO_WRITE_REGISTER;
    wbuf[1] = ( address & 0xFF00 ) >> 8 ;
    wbuf[2] = address & 0x00FF;

    for( uint16_t i = 0; i < size; i++ )
    {
        wbuf[3 + i] = buffer[i];
    }

    lora_phy_write_sync(wbuf, size + 3);

    SX126xWaitOnBusy( );
}

void SX126xWriteRegister( uint16_t address, uint8_t value )
{
    SX126xWriteRegisters( address, &value, 1 );
}

void SX126xReadRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
    char wbuf[size + 4];
    char rbuf[size + 4];

    SX126xCheckDeviceReady( );

    bzero(wbuf, size + 4);
    wbuf[0] = RADIO_READ_REGISTER;
    wbuf[1] = ( address & 0xFF00 ) >> 8 ;
    wbuf[2] = address & 0x00FF;
    wbuf[3] = 0x00; // NOP

    lora_phy_read_write_sync(wbuf, rbuf, size + 4);

    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = rbuf[i + 4];
    }

    SX126xWaitOnBusy( );
}

uint8_t SX126xReadRegister( uint16_t address )
{
    uint8_t data;
    SX126xReadRegisters( address, &data, 1 );
    return data;
}

void SX126xWriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    char wbuf[size + 2];

    SX126xCheckDeviceReady( );

    bzero(wbuf, size + 2);
    wbuf[0] = RADIO_WRITE_BUFFER;
    wbuf[1] = offset;

    for( uint16_t i = 0; i < size; i++ )
    {
        wbuf[2 + i] = buffer[i];
    }

    lora_phy_write_sync(wbuf, size + 2);


    SX126xWaitOnBusy( );
}

void SX126xReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
    char wbuf[size + 3];
    char rbuf[size + 3];

    SX126xCheckDeviceReady( );

    bzero(wbuf, size + 3);
    wbuf[0] = RADIO_READ_BUFFER;
    wbuf[1] = offset;
    wbuf[2] = 0x00; // NOP

    lora_phy_read_write_sync(wbuf, rbuf, size + 3);

    for( uint16_t i = 0; i < size; i++ )
    {
        buffer[i] = rbuf[i + 3];
    }

    SX126xWaitOnBusy( );
}

void SX126xSetRfTxPower( int8_t power )
{
    SX126xSetTxParams( power, RADIO_RAMP_40_US );
}

uint8_t SX126xGetDeviceId( void )
{
    return SX1262;
}

void SX126xAntSwOn( void )
{
}

void SX126xAntSwOff( void )
{
}

bool SX126xCheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}

uint32_t SX126xGetDio1PinState( void )
{
    return GpioRead( &SX126x.DIO1 );
}
