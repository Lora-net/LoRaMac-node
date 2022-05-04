/*!
 * \file      gpio-board.c
 *
 * \brief     Target board GPIO driver implementation
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
#include "msp430.h"
#include "driverlib.h"
#include "utilities.h"
#include "sysIrqHandlers.h"
#include "board-config.h"
#include "rtc-board.h"
#include "gpio-board.h"

static Gpio_t *GpioIrq[16];

void GpioMcuInit( Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value )
{

    if( pin == NC )
    {
        return;
    }

    obj->pin = pin;
    obj->pinIndex = ( 0x01 << ( obj->pin & 0x0F ) );
    obj->portIndex = pin/16 + 1;
    obj->pull = type;

    if( mode == PIN_INPUT )
    {
        GPIO_setAsInputPin(obj->portIndex, obj->pinIndex);
    }
    else if( mode == PIN_ALTERNATE_FCT )
    {
        //TODO: distinguish type of alternate function (primary/secondary) --> this solution works for UART only and is not generic!
        if(value)
            GPIO_setAsPeripheralModuleFunctionInputPin(obj->portIndex, obj->pinIndex, GPIO_SECONDARY_MODULE_FUNCTION);
        else
            GPIO_setAsPeripheralModuleFunctionOutputPin(obj->portIndex, obj->pinIndex, GPIO_SECONDARY_MODULE_FUNCTION);
    }
    else // mode output
    {
        GPIO_setAsOutputPin(obj->portIndex, obj->pinIndex);
    }

    // Sets initial output value
    if( mode == PIN_OUTPUT )
    {
        GpioMcuWrite( obj, value );
    }

}

void GpioMcuSetContext( Gpio_t *obj, void* context )
{
    obj->Context = context;
}

void GpioMcuSetInterrupt( Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler )
{
    // uint32_t priority = 0;

    // IRQn_Type IRQnb = EXTI0_IRQn;
    // GPIO_InitTypeDef   GPIO_InitStructure;

    // if( irqHandler == NULL )
    // {
    //     return;
    // }

    // obj->IrqHandler = irqHandler;

    // GPIO_InitStructure.Pin =  obj->pinIndex;

    // if( irqMode == IRQ_RISING_EDGE )
    // {
    //     GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING;
    // }
    // else if( irqMode == IRQ_FALLING_EDGE )
    // {
    //     GPIO_InitStructure.Mode = GPIO_MODE_IT_FALLING;
    // }
    // else
    // {
    //     GPIO_InitStructure.Mode = GPIO_MODE_IT_RISING_FALLING;
    // }

    // GPIO_InitStructure.Pull = obj->pull;
    // GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;

    // HAL_GPIO_Init( obj->port, &GPIO_InitStructure );

    // switch( irqPriority )
    // {
    // case IRQ_VERY_LOW_PRIORITY:
    // case IRQ_LOW_PRIORITY:
    //     priority = 3;
    //     break;
    // case IRQ_MEDIUM_PRIORITY:
    //     priority = 2;
    //     break;
    // case IRQ_HIGH_PRIORITY:
    //     priority = 1;
    //     break;
    // case IRQ_VERY_HIGH_PRIORITY:
    // default:
    //     priority = 0;
    //     break;
    // }

    // switch( obj->pinIndex )
    // {
    // case GPIO_PIN_0:
    //     IRQnb = EXTI0_IRQn;
    //     break;
    // case GPIO_PIN_1:
    //     IRQnb = EXTI1_IRQn;
    //     break;
    // case GPIO_PIN_2:
    //     IRQnb = EXTI2_IRQn;
    //     break;
    // case GPIO_PIN_3:
    //     IRQnb = EXTI3_IRQn;
    //     break;
    // case GPIO_PIN_4:
    //     IRQnb = EXTI4_IRQn;
    //     break;
    // case GPIO_PIN_5:
    // case GPIO_PIN_6:
    // case GPIO_PIN_7:
    // case GPIO_PIN_8:
    // case GPIO_PIN_9:
    //     IRQnb = EXTI9_5_IRQn;
    //     break;
    // case GPIO_PIN_10:
    // case GPIO_PIN_11:
    // case GPIO_PIN_12:
    // case GPIO_PIN_13:
    // case GPIO_PIN_14:
    // case GPIO_PIN_15:
    //     IRQnb = EXTI15_10_IRQn;
    //     break;
    // default:
    //     break;
    // }

    // GpioIrq[( obj->pin ) & 0x0F] = obj;

    // HAL_NVIC_SetPriority( IRQnb , priority, 0 );
    // HAL_NVIC_EnableIRQ( IRQnb );
   
}

void GpioMcuRemoveInterrupt( Gpio_t *obj )
{
    // Clear callback before changing pin mode
    GpioIrq[( obj->pin ) & 0x0F] = NULL;

   // GPIO_InitTypeDef   GPIO_InitStructure;

  //  GPIO_InitStructure.Pin =  obj->pinIndex ;
 //   GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
 //   HAL_GPIO_Init( obj->port, &GPIO_InitStructure );

}

void GpioMcuWrite( Gpio_t *obj, uint32_t value )
{
    if( obj == NULL )
    {
        //assert( LMN_STATUS_ERROR );
    }
    // Check if pin is not connected
    if( obj->pin == NC )
    {
        return;
    }
    if(value)
        GPIO_setOutputHighOnPin(obj->portIndex, obj->pinIndex);
    else
        GPIO_setOutputLowOnPin(obj->portIndex, obj->pinIndex);
}

void GpioMcuToggle( Gpio_t *obj )
{
    if( obj == NULL )
    {
        //assert_param( LMN_STATUS_ERROR );
    }

    // Check if pin is not connected
    if( obj->pin == NC )
    {
        return;
    }
    GPIO_toggleOutputOnPin( obj->portIndex, obj->pinIndex );
}

uint32_t GpioMcuRead( Gpio_t *obj )
{
    if( obj == NULL )
    {
        //assert_param( LMN_STATUS_ERROR );
    }
    // Check if pin is not connected
    if( obj->pin == NC )
    {
        return 0;
    }
    return GPIO_getInputPinValue( obj->portIndex, obj->pinIndex );
}

void EXTI0_IRQHandler( void )
{
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_0 );
}

void EXTI1_IRQHandler( void )
{
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_1 );
}

void EXTI2_IRQHandler( void )
{
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_2 );
}

void EXTI3_IRQHandler( void )
{
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_3 );
}

void EXTI4_IRQHandler( void )
{
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_4 );
}

void EXTI9_5_IRQHandler( void )
{
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_5 );
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_6 );
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_7 );
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_8 );
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_9 );
}

void EXTI15_10_IRQHandler( void )
{
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_10 );
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_11 );
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_12 );
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_13 );
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_14 );
    //HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_15 );
}

void HAL_GPIO_EXTI_Callback( uint16_t gpioPin )
{
    uint8_t callbackIndex = 0;

    if( gpioPin > 0 )
    {
        while( gpioPin != 0x01 )
        {
            gpioPin = gpioPin >> 1;
            callbackIndex++;
        }
    }

    if( ( GpioIrq[callbackIndex] != NULL ) && ( GpioIrq[callbackIndex]->IrqHandler != NULL ) )
    {
        GpioIrq[callbackIndex]->IrqHandler( GpioIrq[callbackIndex]->Context );
    }
}
