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
#include "stm32l1xx_hal_spi.h"

/*!
 * MCU SPI peripherals enumeration
 */
typedef enum
{
    SPI_1 = ( uint32_t )SPI1_BASE,
    SPI_2 = ( uint32_t )SPI2_BASE,
}SPIName;

void SpiInit( Spi_t *obj, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss )
{
    BoardDisableIrq( );

    // Choose SPI interface according to the given pins
    if( mosi == PA_7 )
    {
        __HAL_RCC_SPI1_FORCE_RESET( );
        __HAL_RCC_SPI1_RELEASE_RESET( );

        __HAL_RCC_SPI1_CLK_ENABLE( );

        obj->Spi.Instance = ( SPI_TypeDef* )SPI1_BASE;

        GpioInit( &obj->Mosi, mosi, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, GPIO_AF5_SPI1 );
        GpioInit( &obj->Miso, miso, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, GPIO_AF5_SPI1 );
        GpioInit( &obj->Sclk, sclk, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, GPIO_AF5_SPI1 );
        GpioInit( &obj->Nss, nss, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF5_SPI1 );

        if( nss == NC )
        {
            obj->Spi.Init.NSS = SPI_NSS_SOFT;
            SpiFormat( obj, SPI_DATASIZE_8BIT, SPI_POLARITY_LOW, SPI_PHASE_1EDGE, 0 );
        }
        else
        {
            SpiFormat( obj, SPI_DATASIZE_8BIT, SPI_POLARITY_LOW, SPI_PHASE_1EDGE, 1 );
        }
    }
    else if( mosi == PB_15 )
    {
        __HAL_RCC_SPI2_FORCE_RESET( );
        __HAL_RCC_SPI2_RELEASE_RESET( );

        __HAL_RCC_SPI2_CLK_ENABLE( );

        obj->Spi.Instance = ( SPI_TypeDef* )SPI2_BASE;

        GpioInit( &obj->Mosi, mosi, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, GPIO_AF5_SPI2 );
        GpioInit( &obj->Miso, miso, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, GPIO_AF5_SPI2 );
        GpioInit( &obj->Sclk, sclk, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, GPIO_AF5_SPI2 );
        GpioInit( &obj->Nss, nss, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF5_SPI2 );

        if( nss == NC )
        {
            obj->Spi.Init.NSS = SPI_NSS_SOFT;
            SpiFormat( obj, SPI_DATASIZE_8BIT, SPI_POLARITY_LOW, SPI_PHASE_1EDGE, 0 );
        }
        else
        {
            SpiFormat( obj, SPI_DATASIZE_8BIT, SPI_POLARITY_LOW, SPI_PHASE_1EDGE, 1 );
        }
    }
    SpiFrequency( obj, 10000000 );

    HAL_SPI_Init( &obj->Spi );

    BoardEnableIrq( );
}

void SpiDeInit( Spi_t *obj )
{
    HAL_SPI_DeInit( &obj->Spi );

    GpioInit( &obj->Mosi, obj->Mosi.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Miso, obj->Miso.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
    GpioInit( &obj->Sclk, obj->Sclk.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Nss, obj->Nss.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
}

void SpiFormat( Spi_t *obj, int8_t bits, int8_t cpol, int8_t cpha, int8_t slave )
{
    obj->Spi.Init.Direction = SPI_DIRECTION_2LINES;
    if( bits == SPI_DATASIZE_8BIT )
    {
        obj->Spi.Init.DataSize = SPI_DATASIZE_8BIT;
    }
    else
    {
        obj->Spi.Init.DataSize = SPI_DATASIZE_16BIT;
    }
    obj->Spi.Init.CLKPolarity = cpol;
    obj->Spi.Init.CLKPhase = cpha;
    obj->Spi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    obj->Spi.Init.TIMode = SPI_TIMODE_DISABLE;
    obj->Spi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    obj->Spi.Init.CRCPolynomial = 7;

    if( slave == 0 )
    {
        obj->Spi.Init.Mode = SPI_MODE_MASTER;
    }
    else
    {
        obj->Spi.Init.Mode = SPI_MODE_SLAVE;
    }
}

void SpiFrequency( Spi_t *obj, uint32_t hz )
{
    uint32_t divisor = 0;
    uint32_t sysClkTmp = SystemCoreClock;
    uint32_t baudRate;

    while( sysClkTmp > hz )
    {
        divisor++;
        sysClkTmp = ( sysClkTmp >> 1 );

        if( divisor >= 7 )
        {
            break;
        }
    }

    baudRate =( ( ( divisor & 0x4 ) == 0 ) ? 0x0 : SPI_CR1_BR_2 ) |
              ( ( ( divisor & 0x2 ) == 0 ) ? 0x0 : SPI_CR1_BR_1 ) |
              ( ( ( divisor & 0x1 ) == 0 ) ? 0x0 : SPI_CR1_BR_0 );

    obj->Spi.Init.BaudRatePrescaler = baudRate;
}

uint16_t SpiInOut( Spi_t *obj, uint16_t outData )
{
    uint8_t rxData = 0;

    if( ( obj == NULL ) || ( obj->Spi.Instance ) == NULL )
    {
        assert_param( FAIL );
    }

    __HAL_SPI_ENABLE( &obj->Spi );

    BoardDisableIrq( );

    while( __HAL_SPI_GET_FLAG( &obj->Spi, SPI_FLAG_TXE ) == RESET );
    obj->Spi.Instance->DR = ( uint16_t ) ( outData & 0xFF );

    while( __HAL_SPI_GET_FLAG( &obj->Spi, SPI_FLAG_RXNE ) == RESET );
    rxData = ( uint16_t ) obj->Spi.Instance->DR;

    BoardEnableIrq( );

    return( rxData );
}

