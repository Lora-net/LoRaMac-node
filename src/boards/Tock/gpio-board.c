/*!
 * \file      gpio-board.c
 *
 * \brief     Target board GPIO driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \author    Alistair Francis <alistair@alistair23.me>
 */
#include "utilities.h"
#include "board-config.h"
#include "rtc-board.h"
#include "gpio-board.h"
#if defined( BOARD_IOE_EXT )
#include "gpio-ioe.h"
#endif

#include "libtock/lora_phy.h"

static Gpio_t *GpioIrq[16];

static void lora_phy_gpio_Callback (int gpioPin,
                           __attribute__ ((unused)) int arg2,
                           __attribute__ ((unused)) int arg3,
                           __attribute__ ((unused)) void* userdata)
{
    uint8_t callbackIndex = 1;

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

void GpioMcuInit( Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value )
{
    obj->pin = pin;
    obj->pinIndex = pin;

    if (mode == PIN_OUTPUT) {
        lora_phy_gpio_enable_output(pin);
    } else {
        if (type == PIN_NO_PULL) {
            lora_phy_gpio_enable_input(pin, PullNone);
        } else if (type == PIN_PULL_UP) {
            lora_phy_gpio_enable_input(pin, PullUp);
        } else if (type == PIN_PULL_DOWN) {
            lora_phy_gpio_enable_input(pin, PullDown);
        }
    }
}

void GpioMcuSetContext( Gpio_t *obj, void* context )
{
    obj->Context = context;
}

void GpioMcuSetInterrupt( Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler )
{
    if (obj->pinIndex == RADIO_NSS) {
        // Skip the chips select as Tock handles this for us
        return;
    }

    obj->IrqHandler = irqHandler;

    GpioIrq[( obj->pin ) & 0x0F] = obj;

    lora_phy_gpio_interrupt_callback(lora_phy_gpio_Callback, NULL);

    // set GPIO as input and enable interrupts on it
    lora_phy_gpio_enable_input(obj->pinIndex, PullDown);
    lora_phy_gpio_enable_interrupt(obj->pinIndex, Change);

}

void GpioMcuRemoveInterrupt( Gpio_t *obj )
{
    if (obj->pinIndex == RADIO_NSS) {
        // Skip the chips select as Tock handles this for us
        return;
    }

    lora_phy_gpio_disable_interrupt(obj->pinIndex);
}

void GpioMcuWrite( Gpio_t *obj, uint32_t value )
{
    if (obj->pinIndex == RADIO_NSS) {
        // Skip the chips select as Tock handles this for us
        return;
    }

    if (value) {
        lora_phy_gpio_set(obj->pinIndex);
    } else {
        lora_phy_gpio_clear(obj->pinIndex);
    }
}

void GpioMcuToggle( Gpio_t *obj )
{
    if (obj->pinIndex == RADIO_NSS) {
        // Skip the chips select as Tock handles this for us
        return;
    }

    lora_phy_gpio_toggle(obj->pinIndex);
}

uint32_t GpioMcuRead( Gpio_t *obj )
{
    int value;

    if (obj->pinIndex == RADIO_NSS) {
        // Skip the chips select as Tock handles this for us
        return 0;
    }

    lora_phy_gpio_read(obj->pinIndex, &value);

    return value;
}
