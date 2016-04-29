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
 * \brief  Find First Set
 *         This function identifies the least significant index or position of the
 *         bits set to one in the word
 *
 * \param [in]  value  Value to find least significant index
 * \retval bitIndex    Index of least significat bit at one
 */
__STATIC_INLINE uint8_t __ffs( uint32_t value )
{
    return( uint32_t )( 32 - __CLZ( value & ( -value ) ) );
}

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
    __HAL_RCC_SPI1_FORCE_RESET( );
    __HAL_RCC_SPI1_RELEASE_RESET( );

    __HAL_RCC_SPI1_CLK_ENABLE( );

    obj->Spi.Instance = ( SPI_TypeDef *) SPI1_BASE;

    GpioInit( &obj->Mosi, mosi, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, GPIO_AF5_SPI1 );
    GpioInit( &obj->Miso, miso, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, GPIO_AF5_SPI1 );
    GpioInit( &obj->Sclk, sclk, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, GPIO_AF5_SPI1 );

    if( nss != NC )
    {
        GpioInit( &obj->Nss, nss, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, GPIO_AF5_SPI1 );
    }
    else
    {
        obj->Spi.Init.NSS = SPI_NSS_SOFT;
    }

    if( nss == NC )
    {
        SpiFormat( obj, SPI_DATASIZE_8BIT, SPI_POLARITY_LOW, SPI_PHASE_1EDGE, 0 );
    }
    else
    {
        SpiFormat( obj, SPI_DATASIZE_8BIT, SPI_POLARITY_LOW, SPI_PHASE_1EDGE, 1 );
    }
    SpiFrequency( obj, 10000000 );

    HAL_SPI_Init( &obj->Spi );
}

void SpiDeInit( Spi_t *obj )
{
    HAL_SPI_DeInit( &obj->Spi );

    GpioInit( &obj->Mosi, obj->Mosi.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Miso, obj->Miso.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0 );
    GpioInit( &obj->Sclk, obj->Sclk.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Nss, obj->Nss.pin, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
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
    uint32_t divisor;

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

    obj->Spi.Init.BaudRatePrescaler = divisor << 3;
}

FlagStatus SpiGetFlag( Spi_t *obj, uint16_t flag )
{
    FlagStatus bitstatus = RESET;

    // Check the status of the specified SPI flag
    if( ( obj->Spi.Instance->SR & flag ) != ( uint16_t )RESET )
    {
        // SPI_I2S_FLAG is set
        bitstatus = SET;
    }
    else
    {
        // SPI_I2S_FLAG is reset
        bitstatus = RESET;
    }
    // Return the SPI_I2S_FLAG status
    return  bitstatus;
}

uint16_t SpiInOut( Spi_t *obj, uint16_t outData )
{
    uint8_t rxData = 0;

    if( ( obj == NULL ) || ( obj->Spi.Instance ) == NULL )
    {
        assert_param( FAIL );
    }

    __HAL_SPI_ENABLE( &obj->Spi );

    while( SpiGetFlag( obj, SPI_FLAG_TXE ) == RESET );
    obj->Spi.Instance->DR = ( uint16_t ) ( outData & 0xFF );

    while( SpiGetFlag( obj, SPI_FLAG_RXNE ) == RESET );
    rxData = ( uint16_t ) obj->Spi.Instance->DR;

    return( rxData );
}

