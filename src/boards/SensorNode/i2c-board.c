/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Bleeper board I2C driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "i2c-board.h"

/*!
 *  The value of the maximal timeout for I2C waiting loops
 */
#define TIMEOUT_MAX                                 0x8000

I2cAddrSize I2cInternalAddrSize = I2C_ADDR_SIZE_8;

/*!
 * MCU I2C peripherals enumeration
 */
typedef enum {
    I2C_1 = ( uint32_t )I2C1_BASE,
    I2C_2 = ( uint32_t )I2C2_BASE,
} I2cName;

void I2cMcuInit( I2c_t *obj, PinNames scl, PinNames sda )
{
    __HAL_RCC_I2C1_FORCE_RESET( );
    __HAL_RCC_I2C1_RELEASE_RESET( );

    obj->I2c.Instance  = ( I2C_TypeDef * )I2C1_BASE;

    GpioInit( &obj->Scl, scl, PIN_ALTERNATE_FCT, PIN_OPEN_DRAIN, PIN_NO_PULL, GPIO_AF4_I2C1 );
    GpioInit( &obj->Sda, sda, PIN_ALTERNATE_FCT, PIN_OPEN_DRAIN, PIN_NO_PULL, GPIO_AF4_I2C1 );
}

void I2cMcuFormat( I2c_t *obj, I2cMode mode, I2cDutyCycle dutyCycle, bool I2cAckEnable, I2cAckAddrMode AckAddrMode, uint32_t I2cFrequency )
{
    I2C_HandleTypeDef *i2c;

    __HAL_RCC_I2C1_CLK_ENABLE( );
    obj->I2c.Init.ClockSpeed = I2cFrequency;

    if( dutyCycle == I2C_DUTY_CYCLE_2 )
    {
        obj->I2c.Init.DutyCycle = I2C_DUTYCYCLE_2;
    }
    else
    {
        obj->I2c.Init.DutyCycle = I2C_DUTYCYCLE_16_9;
    }

    obj->I2c.Init.OwnAddress1 = 0;
    obj->I2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    obj->I2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    obj->I2c.Init.OwnAddress2 = 0;
    obj->I2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    obj->I2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;

    i2c = &obj->I2c;
    HAL_I2C_Init( i2c );
}

void I2cMcuDeInit( I2c_t *obj )
{
    I2C_HandleTypeDef *i2c;
    i2c = &obj->I2c;

    HAL_I2C_DeInit( i2c );

    if( obj->I2c.Instance == ( I2C_TypeDef * ) I2C1_BASE )
    {
        __HAL_RCC_I2C1_FORCE_RESET();
        __HAL_RCC_I2C1_RELEASE_RESET();
        __HAL_RCC_I2C1_CLK_DISABLE( );
    }
    else
    {
        __HAL_RCC_I2C2_FORCE_RESET();
        __HAL_RCC_I2C2_RELEASE_RESET();
        __HAL_RCC_I2C2_CLK_DISABLE( );
    }

    GpioInit( &obj->Scl, obj->Scl.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Sda, obj->Sda.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void I2cSetAddrSize( I2c_t *obj, I2cAddrSize addrSize )
{
    I2cInternalAddrSize = addrSize;
}

uint8_t I2cMcuWriteBuffer( I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t *buffer, uint16_t size )
{
    uint8_t status = FAIL;
    uint16_t memAddSize = 0;

    I2C_HandleTypeDef *i2c;
    i2c = &obj->I2c;

    if( I2cInternalAddrSize == I2C_ADDR_SIZE_8 )
    {
        memAddSize = I2C_MEMADD_SIZE_8BIT;
    }
    else
    {
        memAddSize = I2C_MEMADD_SIZE_16BIT;
    }
    status = ( HAL_I2C_Mem_Write( i2c, deviceAddr, addr, memAddSize, buffer, size, 2000 ) == HAL_OK ) ? SUCCESS : FAIL;
    return status;
}

uint8_t I2cMcuReadBuffer( I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t *buffer, uint16_t size )
{
    uint8_t status = FAIL;
    uint16_t memAddSize = 0;

    I2C_HandleTypeDef *i2c;
    i2c = &obj->I2c;
    if( I2cInternalAddrSize == I2C_ADDR_SIZE_8 )
    {
        memAddSize = I2C_MEMADD_SIZE_8BIT;
    }
    else
    {
        memAddSize = I2C_MEMADD_SIZE_16BIT;
    }
    status = ( HAL_I2C_Mem_Read( i2c, deviceAddr, addr, memAddSize, buffer, size, 2000 ) == HAL_OK ) ? SUCCESS : FAIL;
    return status;
}

uint8_t I2cMcuWaitStandbyState( I2c_t *obj, uint8_t deviceAddr )
{
    uint8_t status = FAIL;
    I2C_HandleTypeDef *i2c;
    i2c = &obj->I2c;
    status = ( HAL_I2C_IsDeviceReady( i2c, deviceAddr, 300, 4096 ) == HAL_OK ) ? SUCCESS : FAIL;;
    return status;
}
