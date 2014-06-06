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

/*!
 * MCU I2C peripherals enumeration
 */
typedef enum {
    I2C_1 = ( uint32_t )I2C1_BASE,
    I2C_2 = ( uint32_t )I2C2_BASE,
} I2cName;

void I2cMcuInit( I2c_t *obj, PinNames scl, PinNames sda )
{
    obj->I2c = ( I2C_TypeDef * )I2C1_BASE;

    RCC_APB1PeriphClockCmd( RCC_APB1Periph_I2C1, ENABLE );

    I2C_DeInit( obj->I2c );   

    GpioInit( &obj->Scl, scl, PIN_ALTERNATE_FCT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );
    GpioInit( &obj->Sda, sda, PIN_ALTERNATE_FCT, PIN_OPEN_DRAIN, PIN_NO_PULL, 0 );

    GPIO_PinAFConfig( obj->Scl.port, ( obj->Scl.pin & 0x0F ), GPIO_AF_I2C1 );
    GPIO_PinAFConfig( obj->Sda.port, ( obj->Sda.pin & 0x0F ), GPIO_AF_I2C1 ); 
}

void I2cMcuFormat( I2c_t *obj, I2cMode mode, I2cDutyCycle dutyCycle, bool I2cAckEnable, I2cAckAddrMode AckAddrMode, uint32_t I2cFrequency )
{
    I2C_InitTypeDef I2C_InitStructure;

    if( mode == MODE_I2C )
    {
        I2C_InitStructure.I2C_Mode = 0x0000;
    }
    else if( mode == MODE_SMBUS_DEVICE )
    {
        I2C_InitStructure.I2C_Mode = 0x0002;
    }
    else 
    {
        I2C_InitStructure.I2C_Mode = 0x000A; // MODE_SMBUS_HOST
    }

    if( dutyCycle == I2C_DUTY_CYCLE_2 )
    {
        I2C_InitStructure.I2C_DutyCycle = 0xBFFF;
    }
    else 
    {
        I2C_InitStructure.I2C_DutyCycle = 0x4000; //I2C_DUTY_CYCLE_16_9
    }

    if( I2cAckEnable == true )
    {
        I2C_InitStructure.I2C_Ack = 0x0400;
    }
    else 
    {
        I2C_InitStructure.I2C_Ack = 0x0000;
    }

    if( AckAddrMode == I2C_ACK_ADD_7_BIT )
    {
        I2C_InitStructure.I2C_AcknowledgedAddress = 0x4000;
    }
    else 
    {
        I2C_InitStructure.I2C_AcknowledgedAddress = 0xC000; // I2C_ACK_ADD_10_BIT
    }

    if( I2cFrequency > 400000 )
    {
        I2C_InitStructure.I2C_ClockSpeed = 400000;
    }
    else
    {
        I2C_InitStructure.I2C_ClockSpeed = I2cFrequency;
    }

    I2C_Init( obj->I2c, &I2C_InitStructure );

    /* I2C Peripheral Enable */
    I2C_Cmd( obj->I2c, ENABLE );
}

void I2cMcuDeInit( I2c_t *obj )
{
    I2C_DeInit( obj->I2c );

    GpioInit( &obj->Scl, obj->Scl.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &obj->Sda, obj->Sda.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

uint8_t I2cMcuWriteBuffer( I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t *buffer, uint16_t size )
{
    uint32_t timeOut;

    __disable_irq( );

    /* Test on BUSY Flag */
    timeOut = TIMEOUT_MAX;
    while( I2C_GetFlagStatus( obj->I2c, I2C_FLAG_BUSY) ) 
    {
        if( ( timeOut-- ) == 0 )
        {
            I2cResetBus( obj );

            __enable_irq( );
            return( FAIL );
        }
    }

    /* Send START condition */
    I2C_GenerateSTART( obj->I2c, ENABLE );

    /* Test on EV5 and clear it */
    timeOut = TIMEOUT_MAX;
    while( !I2C_CheckEvent( obj->I2c, I2C_EVENT_MASTER_MODE_SELECT ) )
    {
        if( ( timeOut-- ) == 0 )
        {
            I2cResetBus( obj );

            __enable_irq( );
            return( FAIL );
        }
    }

    /* Send device's address for write */
    I2C_Send7bitAddress( obj->I2c, deviceAddr, I2C_Direction_Transmitter );

    /* Test on EV6 and clear it */
    timeOut = TIMEOUT_MAX;
    while( !I2C_CheckEvent( obj->I2c, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) )
    {
        if( ( timeOut-- ) == 0 )
        {
            I2cResetBus( obj );

            __enable_irq( );
            return( FAIL );
        }
    }

    if( ( addr & 0xFF00 ) != 0x0000 ) 
    {
        /* Send the device's internal address MSB to write to */
        I2C_SendData( obj->I2c, ( uint8_t )( ( addr & 0xFF00 ) >> 8 )  );

        /* Test on EV8 and clear it */
        timeOut = TIMEOUT_MAX;
        while( !I2C_CheckEvent( obj->I2c, I2C_EVENT_MASTER_BYTE_TRANSMITTING ) )
        {
            if( ( timeOut-- ) == 0 )
            {
                I2cResetBus( obj );

                __enable_irq( );
                return( FAIL );
            }
        }
    }

    /* Send the device's internal address LSB to write to */
    I2C_SendData( obj->I2c, ( uint8_t )( addr & 0x00FF ) );

    /* Test on EV8 and clear it */
    timeOut = TIMEOUT_MAX;
    while( !I2C_CheckEvent( obj->I2c, I2C_EVENT_MASTER_BYTE_TRANSMITTING ) )
    {
        if( ( timeOut-- ) == 0 )
        {
            I2cResetBus( obj );

            __enable_irq( );
            return( FAIL );
        }
    }

    while( size )
    {
        /* Send the byte to be written */
        I2C_SendData( obj->I2c, *buffer );

        /* Test on EV8 and clear it */
        timeOut = TIMEOUT_MAX;
        while( !I2C_CheckEvent( obj->I2c, I2C_EVENT_MASTER_BYTE_TRANSMITTING ) )
        {
            if( ( timeOut-- ) == 0 )
            {
                I2cResetBus( obj );

                __enable_irq( );
                return( FAIL );
            }
        }

        if( size == 1 )
        { 
            I2C_GenerateSTOP( obj->I2c, ENABLE ); 
        
            /* Wait to make sure that STOP control bit has been cleared */
            timeOut = TIMEOUT_MAX;
            while(obj->I2c->CR1 & I2C_CR1_STOP)
            {
                if( ( timeOut-- ) == 0 )
                {
                    I2cResetBus( obj );

                    __enable_irq( );
                    return( FAIL );
                }
            }
        } /* STOP */

        buffer++;
        size--;
    }

    __enable_irq( );
    return( SUCCESS );
}

uint8_t I2cMcuReadBuffer( I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t *buffer, uint16_t size )
{
    uint32_t timeOut;

    __disable_irq( );

    /* Test on BUSY Flag */
    timeOut = TIMEOUT_MAX;
    while( I2C_GetFlagStatus( obj->I2c, I2C_FLAG_BUSY ) ) 
    {
        if( ( timeOut-- ) == 0 )
        {
            I2cResetBus( obj );

            __enable_irq( );
            return( FAIL );
        }
    }

    /* Send START condition */
    I2C_GenerateSTART( obj->I2c, ENABLE );

    /* Test on EV5 and clear it */
    timeOut = TIMEOUT_MAX;
    while( !I2C_CheckEvent( obj->I2c, I2C_EVENT_MASTER_MODE_SELECT ) )
    {
        if( ( timeOut-- ) == 0 )
        {
            I2cResetBus( obj );

            __enable_irq( );
            return( FAIL );
        }
    }

    /* Send device's address for write */
    I2C_Send7bitAddress( obj->I2c, deviceAddr, I2C_Direction_Transmitter );

    /* Test on EV6 and clear it */
    timeOut = TIMEOUT_MAX;
    while( !I2C_CheckEvent( obj->I2c, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) )
    {
        if( ( timeOut-- ) == 0 )
        {
            I2cResetBus( obj );

            __enable_irq( );
            return( FAIL );
        }
    }

    if( ( addr & 0xFF00 ) != 0x0000 )
    {
        /* Send the device's internal address MSB to write to */
        I2C_SendData( obj->I2c, ( uint8_t )( ( addr & 0xFF00 ) >> 8 )  );

        /* Test on EV8 and clear it */
        timeOut = TIMEOUT_MAX;
        while( !I2C_CheckEvent( obj->I2c, I2C_EVENT_MASTER_BYTE_TRANSMITTING ) )
        {
            if( ( timeOut-- ) == 0 )
            {
                I2cResetBus( obj );

                __enable_irq( );
                return( FAIL );
            }
        }
    }

    /* Send the device's internal address LSB to write to */
    I2C_SendData( obj->I2c, ( uint8_t )( addr & 0x00FF ) );

    /* Test on EV8 and clear it */
    timeOut = TIMEOUT_MAX;
    while( !I2C_CheckEvent( obj->I2c, I2C_EVENT_MASTER_BYTE_TRANSMITTING ) )
    {
        if( ( timeOut-- ) == 0 )
        {
            I2cResetBus( obj );

            __enable_irq( );
            return( FAIL );
        }
    }

    /* Test on EV8 and clear it */
    timeOut = TIMEOUT_MAX;
    while( I2C_GetFlagStatus( obj->I2c, I2C_FLAG_BTF ) == RESET )
    {
        if( ( timeOut-- ) == 0 )
        {
            I2cResetBus( obj );

            __enable_irq( );
            return( FAIL );
        }
    }

    /* Send START condition a second time */
    I2C_GenerateSTART( obj->I2c, ENABLE );

    /* Test on EV5 and clear it */
    timeOut = TIMEOUT_MAX;
    while( !I2C_CheckEvent( obj->I2c, I2C_EVENT_MASTER_MODE_SELECT ) )
    {
        if( ( timeOut-- ) == 0 )
        {
            I2cResetBus( obj );

            __enable_irq( );
            return( FAIL );
        }
    }

    /* Send device's address for read */
    I2C_Send7bitAddress( obj->I2c, deviceAddr, I2C_Direction_Receiver );

    if( size < 2 )
    {
        /* Wait on ADDR flag to be set (ADDR is still not cleared at this level */
        timeOut = TIMEOUT_MAX;
        while( I2C_GetFlagStatus( obj->I2c, I2C_FLAG_ADDR ) == RESET )
        {
            if( ( timeOut-- ) == 0 )
            {
                I2cResetBus( obj );

                __enable_irq( );
                return( FAIL );
            }
        }
    
        /* Disable Acknowledgement */
        I2C_AcknowledgeConfig( obj->I2c, DISABLE );  
    
        /* Clear ADDR register by reading SR1 then SR2 register (SR1 has already been read) */
        (void)obj->I2c->SR2;
    
        /* Send STOP Condition */
        I2C_GenerateSTOP( obj->I2c, ENABLE );
    
        /* Wait for the byte to be received */
        timeOut = TIMEOUT_MAX;
        while( I2C_GetFlagStatus( obj->I2c, I2C_FLAG_RXNE ) == RESET )
        {
            if( ( timeOut-- ) == 0 )
            {
                I2cResetBus( obj );

                __enable_irq( );
                return( FAIL );
            }
        }
    
        /* Read a byte from the device */
        *buffer = I2C_ReceiveData( obj->I2c );

        /* Decrement the read bytes counter */
        size--;
    }
    else
    {
        while( size )
        {
             /* Wait for the byte to be received */
            timeOut = TIMEOUT_MAX;
            while( I2C_GetFlagStatus( obj->I2c, I2C_FLAG_RXNE ) == RESET )
            {
                if( ( timeOut-- ) == 0 )
                {
                    I2cResetBus( obj );

                    __enable_irq( );
                    return( FAIL );
                }
            } 
        
            if( size == 1 )
            {            
                /* Disable Acknowledgement */
                I2C_AcknowledgeConfig( obj->I2c, DISABLE );

                /* Send STOP Condition */
                I2C_GenerateSTOP( obj->I2c, ENABLE );
            }
        
            /* Read a byte from the device */
            *buffer = I2C_ReceiveData( obj->I2c );

            /* Point to the next location where the byte read will be saved */
            buffer++;

            /* Decrement the read bytes counter */
            size--;
        }
        
        /* Wait to make sure that STOP control bit has been cleared */
        timeOut = TIMEOUT_MAX;
        while(obj->I2c->CR1 & I2C_CR1_STOP)
        {
            if( ( timeOut-- ) == 0 )
            {
                I2cResetBus( obj );

                __enable_irq( );
                return( FAIL );
            }
        }
        /*!< Re-Enable Acknowledgement to be ready for another reception */
        I2C_AcknowledgeConfig( obj->I2c, ENABLE );   
    }

    __enable_irq( );
    return( SUCCESS );
}
