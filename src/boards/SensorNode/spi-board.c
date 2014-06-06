/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Bleeper board SPI driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "spi-board.h"
#include "stm32l1xx_spi.h"
#include "stm32l1xx_gpio.h"

/*!
 * MCU SPI peripherals enumeration
 */
typedef enum {
    SPI_1 = ( uint32_t )SPI1_BASE,
    SPI_2 = ( uint32_t )SPI2_BASE,
    SPI_3 = ( uint32_t )SPI3_BASE,
} SPIName;

SPI_InitTypeDef SPI_InitStructure;

void SpiInit( Spi_t *obj, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss )
{
    GpioInit( &obj->Mosi, mosi, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
    GpioInit( &obj->Miso, miso, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
    GpioInit( &obj->Sclk, sclk, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );

    // TODO: Make independent of stm32l1xx_gpio.h
    GPIO_PinAFConfig( obj->Mosi.port, ( obj->Mosi.pin & 0x0F ), GPIO_AF_SPI1 );
    GPIO_PinAFConfig( obj->Miso.port, ( obj->Miso.pin & 0x0F ), GPIO_AF_SPI1 );
    GPIO_PinAFConfig( obj->Sclk.port, ( obj->Sclk.pin & 0x0F ), GPIO_AF_SPI1 );

    if( nss != NC )
    {
        GpioInit( &obj->Nss, nss, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
        // TODO: Make independent of stm32l1xx_gpio.h
        GPIO_PinAFConfig( obj->Nss.port, ( obj->Nss.pin & 0x0F ), GPIO_AF_SPI1 );
    }
    else
    {
        SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    }

    // Choose SPI interface according to the given pins
    obj->Spi = ( SPI_TypeDef* )SPI1_BASE;
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1, ENABLE );

    if( nss == NC )
    {
        // 8 bits, CPOL = 0, CPHA = 0, MASTER
        SpiFormat( obj, 8, 0, 0, 0 );
    }
    else
    {
        // 8 bits, CPOL = 0, CPHA = 0, SLAVE
        SpiFormat( obj, 8, 0, 0, 1 );
    }
    SpiFrequency( obj, 10000000 );

    SPI_Cmd( obj->Spi, ENABLE );
}

void SpiDeInit( Spi_t *obj )
{
    SPI_Cmd( obj->Spi, DISABLE );
    SPI_I2S_DeInit( obj->Spi );

    GpioInit( &obj->Mosi, obj->Mosi.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Miso, obj->Miso.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
    GpioInit( &obj->Sclk, obj->Sclk.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Nss, obj->Nss.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

}

void SpiFormat( Spi_t *obj, int8_t bits, int8_t cpol, int8_t cpha, int8_t slave )
{
    SPI_Cmd( obj->Spi, DISABLE );
    
    if( ( ( ( bits == 8 ) || ( bits == 16 ) ) == false ) ||
        ( ( ( cpol >= 0 ) && ( cpol <= 1 ) ) == false ) ||
        ( ( ( cpha >= 0 ) && ( cpha <= 1 ) ) == false ) )
    {
        // SPI error
        while( 1 );
    }

    SPI_InitStructure.SPI_Mode = ( slave == 0x01 ) ? SPI_Mode_Slave : SPI_Mode_Master;
    SPI_InitStructure.SPI_CPOL = ( cpol == 0x01 ) ? SPI_CPOL_High : SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = ( cpha == 0x01 ) ? SPI_CPHA_2Edge : SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_DataSize = ( bits == 8 ) ? SPI_DataSize_8b : SPI_DataSize_16b;
    SPI_Init( obj->Spi, &SPI_InitStructure );

    SPI_Cmd( obj->Spi, ENABLE );
}

void SpiFrequency( Spi_t *obj, uint32_t hz )
{
    uint32_t divisor;

    SPI_Cmd( obj->Spi, DISABLE );

    divisor = SystemCoreClock / hz;
    
    // Find the nearest power-of-2
    divisor = divisor > 0 ? divisor-1 : 0;
    divisor |= divisor >> 1;
    divisor |= divisor >> 2;
    divisor |= divisor >> 4;
    divisor |= divisor >> 8;
    divisor |= divisor >> 16;
    divisor++;

    divisor = __ffs( divisor ) - 1;

    divisor = ( divisor > 0x07 ) ? 0x07 : divisor;

    SPI_InitStructure.SPI_BaudRatePrescaler = divisor << 3;
    SPI_Init( obj->Spi, &SPI_InitStructure );

    SPI_Cmd( obj->Spi, ENABLE );
}

uint16_t SpiInOut( Spi_t *obj, uint16_t outData )
{
    if( ( obj == NULL ) || ( obj->Spi ) == NULL )
    {
        while( 1 );
    }
    
    while( SPI_I2S_GetFlagStatus( obj->Spi, SPI_I2S_FLAG_TXE ) == RESET );
    SPI_I2S_SendData( obj->Spi, outData );
    while( SPI_I2S_GetFlagStatus( obj->Spi, SPI_I2S_FLAG_RXNE ) == RESET );
    return SPI_I2S_ReceiveData( obj->Spi );
}

