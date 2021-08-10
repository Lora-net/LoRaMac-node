/*!
 * \file      sx1276-board.c
 *
 * \brief     Target board SX1276 driver implementation
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
#include <stdlib.h>
#include "utilities.h"
#include "board-config.h"
#include "delay.h"
#include "radio.h"
#include "sx1276-board.h"

/*!
 * \brief Gets the board PA selection configuration
 *
 * \param [IN] power Selects the right PA according to the wanted power.
 * \retval PaSelect RegPaConfig PaSelect value
 */
static uint8_t SX1276GetPaSelect(int8_t power);

/*!
 * Flag used to set the RF switch control pins in low power mode when the radio is not active.
 */
static bool RadioIsActive = false;

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
    {
        SX1276Init,
        SX1276GetStatus,
        SX1276SetModem,
        SX1276SetChannel,
        SX1276IsChannelFree,
        SX1276Random,
        SX1276SetRxConfig,
        SX1276SetTxConfig,
        SX1276CheckRfFrequency,
        SX1276GetTimeOnAir,
        SX1276Send,
        SX1276SetSleep,
        SX1276SetStby,
        SX1276SetRx,
        SX1276StartCad,
        SX1276SetTxContinuousWave,
        SX1276ReadRssi,
        SX1276Write,
        SX1276Read,
        SX1276WriteBuffer,
        SX1276ReadBuffer,
        SX1276SetMaxPayloadLength,
        SX1276SetPublicNetwork,
        SX1276GetWakeupTime,
        NULL, // void ( *IrqProcess )( void )
        NULL, // void ( *RxBoosted )( uint32_t timeout ) - SX126x Only
        NULL, // void ( *SetRxDutyCycle )( uint32_t rxTime, uint32_t sleepTime ) - SX126x Only
};

/*!
 * TCXO power control pin
 */
Gpio_t TcxoPower;

/*!
 * Antenna switch GPIO pins objects
 */
Gpio_t AntSwitchRx;
Gpio_t AntSwitchTxBoost;
Gpio_t AntSwitchTxRfo;

/*!
 * Debug GPIO pins objects
 */
#if defined(USE_RADIO_DEBUG)
Gpio_t DbgPinTx;
Gpio_t DbgPinRx;
#endif

void SX1276IoInit(void)
{
}

void SX1276IoIrqInit(DioIrqHandler **irqHandlers)
{
}

void SX1276IoDeInit(void)
{
}

void SX1276IoDbgInit(void)
{
#if defined(USE_RADIO_DEBUG)
    GpioInit(&DbgPinTx, RADIO_DBG_PIN_TX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
    GpioInit(&DbgPinRx, RADIO_DBG_PIN_RX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0);
#endif
}

void SX1276IoTcxoInit(void)
{
}

void SX1276SetBoardTcxo(uint8_t state)
{
    if (state == true)
    {
    }
    else
    {
    }
}

uint32_t SX1276GetBoardTcxoWakeupTime(void)
{
    return BOARD_TCXO_WAKEUP_TIME;
}

void SX1276Reset(void)
{
}

void SX1276SetRfTxPower(int8_t power)
{
    uint8_t paConfig = 0;
    uint8_t paDac = 0;

    paConfig = (paConfig & RF_PACONFIG_PASELECT_MASK);

    if ((paConfig & RF_PACONFIG_PASELECT_PABOOST) == RF_PACONFIG_PASELECT_PABOOST)
    {
        if (power > 17)
        {
            paDac = (paDac & RF_PADAC_20DBM_MASK) | RF_PADAC_20DBM_ON;
        }
        else
        {
            paDac = (paDac & RF_PADAC_20DBM_MASK) | RF_PADAC_20DBM_OFF;
        }
        if ((paDac & RF_PADAC_20DBM_ON) == RF_PADAC_20DBM_ON)
        {
            if (power < 5)
            {
                power = 5;
            }
            if (power > 20)
            {
                power = 20;
            }
            paConfig = (paConfig & RF_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t)((uint16_t)(power - 5) & 0x0F);
        }
        else
        {
            if (power < 2)
            {
                power = 2;
            }
            if (power > 17)
            {
                power = 17;
            }
            paConfig = (paConfig & RF_PACONFIG_OUTPUTPOWER_MASK) | (uint8_t)((uint16_t)(power - 2) & 0x0F);
        }
    }
    else
    {
        if (power > 0)
        {
            if (power > 15)
            {
                power = 15;
            }
            paConfig = (paConfig & RF_PACONFIG_MAX_POWER_MASK & RF_PACONFIG_OUTPUTPOWER_MASK) | (7 << 4) | (power);
        }
        else
        {
            if (power < -4)
            {
                power = -4;
            }
            paConfig = (paConfig & RF_PACONFIG_MAX_POWER_MASK & RF_PACONFIG_OUTPUTPOWER_MASK) | (0 << 4) | (power + 4);
        }
    }
}

static uint8_t SX1276GetPaSelect(int8_t power)
{
    if (power > 14)
    {
        return RF_PACONFIG_PASELECT_PABOOST;
    }
    else
    {
        return RF_PACONFIG_PASELECT_RFO;
    }
}

void SX1276SetAntSwLowPower(bool status)
{
    if (RadioIsActive != status)
    {
        RadioIsActive = status;

        if (status == false)
        {
            SX1276AntSwInit();
        }
        else
        {
            SX1276AntSwDeInit();
        }
    }
}

void SX1276AntSwInit(void)
{
}

void SX1276AntSwDeInit(void)
{
}

void SX1276SetAntSw(uint8_t opMode)
{
    switch (opMode)
    {
    case RFLR_OPMODE_TRANSMITTER:

        break;
    case RFLR_OPMODE_RECEIVER:
    case RFLR_OPMODE_RECEIVER_SINGLE:
    case RFLR_OPMODE_CAD:
    default:
        break;
    }
}

bool SX1276CheckRfFrequency(uint32_t frequency)
{
    // Implement check. Currently all frequencies are supported
    return true;
}

uint32_t SX1276GetDio1PinState(void)
{
}

#if defined(USE_RADIO_DEBUG)
void SX1276DbgPinTxWrite(uint8_t state)
{
    GpioWrite(&DbgPinTx, state);
}

void SX1276DbgPinRxWrite(uint8_t state)
{
    GpioWrite(&DbgPinRx, state);
}
#endif
