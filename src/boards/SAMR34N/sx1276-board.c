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
 */
#include <stddef.h>

#include "asf.h"

#include "board-config.h"
#include "delay-board.h"
#include "radio.h"
#include "sx1276-board.h"

static void Dio0IrqHandler( void );
static void Dio1IrqHandler( void );
static void Dio2IrqHandler( void );
static void Dio3IrqHandler( void );

/*!
 * \brief Gets the board PA selection configuration
 *
 * \param [IN] channel Channel frequency in Hz
 * \retval PaSelect RegPaConfig PaSelect value
 */
static uint8_t SX1276GetPaSelect( uint32_t channel );

/*!
 * Flag used to set the RF switch control pins in low power mode when the radio is not active.
 */
static bool RadioIsActive = false;

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio = {
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
    NULL,  // void ( *IrqProcess )( void )
    NULL,  // void ( *RxBoosted )( uint32_t timeout ) - SX126x Only
    NULL,  // void ( *SetRxDutyCycle )( uint32_t rxTime, uint32_t sleepTime ) - SX126x Only
};

/*!
 * TCXO power control pin
 */
Gpio_t tcxo_pin;

/*!
 * Antenna switch GPIO pins objects
 */
Gpio_t rfswitch_pin;

/*!
 * Debug GPIO pins objects
 */
#if defined( USE_RADIO_DEBUG )
Gpio_t DbgPinTx;
Gpio_t DbgPinRx;
#endif

typedef struct radio_dio_pin_s
{
    Gpio_t*                 pin_obj;
    uint8_t                 pin;
    struct extint_chan_conf config_extint_chan;
    uint8_t                 channel;
    extint_callback_t       irq_handler;
} radio_dio_pin_t;

static radio_dio_pin_t dio_pins[] =
{
    {
        .pin_obj = &SX1276.DIO0,
        .pin = DIO0_PIN,
        .config_extint_chan =
        {
            .gpio_pin           = DIO0_EIC_PIN,
            .gpio_pin_mux       = DIO0_EIC_MUX,
            .gpio_pin_pull      = EXTINT_PULL_NONE,
            .detection_criteria = EXTINT_DETECT_RISING,
        },
        .channel = DIO0_EIC_LINE,
        .irq_handler = Dio0IrqHandler,
    },
    {
        .pin_obj = &SX1276.DIO1,
        .pin = DIO1_PIN,
        .config_extint_chan =
        {
            .gpio_pin           = DIO1_EIC_PIN,
            .gpio_pin_mux       = DIO1_EIC_MUX,
            .gpio_pin_pull      = EXTINT_PULL_NONE,
            .detection_criteria = EXTINT_DETECT_BOTH, // Must be setup to be trigged on both edges.
        },
        .channel = DIO1_EIC_LINE,
            .irq_handler = Dio1IrqHandler,
    },
    {
        .pin_obj = &SX1276.DIO2,
        .pin = DIO2_PIN,
        .config_extint_chan =
        {
            .gpio_pin           = DIO2_EIC_PIN,
            .gpio_pin_mux       = DIO2_EIC_MUX,
            .gpio_pin_pull      = EXTINT_PULL_NONE,
            .detection_criteria = EXTINT_DETECT_RISING,
        },
        .channel = DIO2_EIC_LINE,
            .irq_handler = Dio2IrqHandler,
    },
    {
        .pin_obj = &SX1276.DIO3,
        .pin = DIO3_PIN,
        .config_extint_chan =
        {
            .gpio_pin           = DIO3_EIC_PIN,
            .gpio_pin_mux       = DIO3_EIC_MUX,
            .gpio_pin_pull      = EXTINT_PULL_NONE,
            .detection_criteria = EXTINT_DETECT_RISING,
        },
        .channel = DIO3_EIC_LINE,
        .irq_handler = Dio3IrqHandler,

    },
};

void SX1276IoInit( void )
{
    GpioInit( &rfswitch_pin, RF_SWITCH_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    for( int i = 0; i < sizeof( dio_pins ) / sizeof( radio_dio_pin_t ); i++ )
    {
        GpioInit( dio_pins[i].pin_obj, dio_pins[i].pin, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
        extint_chan_get_config_defaults( &dio_pins[3].config_extint_chan );
        extint_chan_set_config( dio_pins[i].channel, &dio_pins[3].config_extint_chan );
    }
}

static void Dio0IrqHandler( void )
{
    if( ( dio_pins[0].pin_obj != NULL ) && ( dio_pins[0].pin_obj->IrqHandler != NULL ) )
    {
        dio_pins[0].pin_obj->IrqHandler( dio_pins[0].pin_obj->Context );
    }
}

static void Dio1IrqHandler( void )
{
    if( ( dio_pins[1].pin_obj != NULL ) && ( dio_pins[1].pin_obj->IrqHandler != NULL ) )
    {
        dio_pins[1].pin_obj->IrqHandler( dio_pins[1].pin_obj->Context );
    }
}

static void Dio2IrqHandler( void )
{
    if( ( dio_pins[2].pin_obj != NULL ) && ( dio_pins[2].pin_obj->IrqHandler != NULL ) )
    {
        dio_pins[2].pin_obj->IrqHandler( dio_pins[2].pin_obj->Context );
    }
}

static void Dio3IrqHandler( void )
{
    if( ( dio_pins[3].pin_obj != NULL ) && ( dio_pins[3].pin_obj->IrqHandler != NULL ) )
    {
        dio_pins[3].pin_obj->IrqHandler( dio_pins[3].pin_obj->Context );
    }
}

static void IoIrqInit( uint8_t index, DioIrqHandler* irqHandler )
{
    dio_pins[index].pin_obj->IrqHandler = irqHandler;
    extint_register_callback( dio_pins[index].irq_handler, dio_pins[index].channel, EXTINT_CALLBACK_TYPE_DETECT );
}

void SX1276IoIrqInit( DioIrqHandler** irqHandlers )
{
    for( int8_t i = 0; i < 4; i++ )
    {
        IoIrqInit( i, irqHandlers[i] );
    }
}

void SX1276IoDeInit( void )
{
    GpioInit( &rfswitch_pin, RF_SWITCH_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void SX1276IoDbgInit( void )
{
#if defined( USE_RADIO_DEBUG )
    GpioInit( &DbgPinTx, RADIO_DBG_PIN_TX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &DbgPinRx, RADIO_DBG_PIN_RX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
}

void SX1276IoTcxoInit( void )
{
    GpioInit( &tcxo_pin, TCXO_PWR_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void SX1276SetBoardTcxo( uint8_t state )
{
    GpioWrite( &tcxo_pin, state );
    DelayMsMcu( BOARD_TCXO_WAKEUP_TIME );
}

uint32_t SX1276GetBoardTcxoWakeupTime( void )
{
    return BOARD_TCXO_WAKEUP_TIME;
}

void SX1276Reset( void )
{
    // Enables the TCXO if available on the board design
    SX1276SetBoardTcxo( true );

    // Set RESET pin to 0
    GpioInit( &SX1276.Reset, SX_RF_RESET_PIN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );

    // Wait 1 ms
    DelayMsMcu( 1 );

    // Configure RESET as input
    GpioInit( &SX1276.Reset, SX_RF_RESET_PIN, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );

    // Wait 6 ms
    DelayMsMcu( 6 );
}

void SX1276SetRfTxPower( int8_t power )
{
    uint8_t paConfig = 0;
    uint8_t paDac    = 0;

    paConfig = SX1276Read( REG_PACONFIG );
    paDac    = SX1276Read( REG_PADAC );

    paConfig = ( paConfig & RF_PACONFIG_PASELECT_MASK ) | SX1276GetPaSelect( SX1276.Settings.Channel );

    if( ( paConfig & RF_PACONFIG_PASELECT_PABOOST ) == RF_PACONFIG_PASELECT_PABOOST )
    {
        if( power > 17 )
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_ON;
        }
        else
        {
            paDac = ( paDac & RF_PADAC_20DBM_MASK ) | RF_PADAC_20DBM_OFF;
        }
        if( ( paDac & RF_PADAC_20DBM_ON ) == RF_PADAC_20DBM_ON )
        {
            if( power < 5 )
            {
                power = 5;
            }
            if( power > 20 )
            {
                power = 20;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 5 ) & 0x0F );
        }
        else
        {
            if( power < 2 )
            {
                power = 2;
            }
            if( power > 17 )
            {
                power = 17;
            }
            paConfig = ( paConfig & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( uint8_t )( ( uint16_t )( power - 2 ) & 0x0F );
        }
    }
    else
    {
        if( power > 0 )
        {
            if( power > 15 )
            {
                power = 15;
            }
            paConfig =
                ( paConfig & RF_PACONFIG_MAX_POWER_MASK & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( 7 << 4 ) | ( power );
        }
        else
        {
            if( power < -4 )
            {
                power = -4;
            }
            paConfig =
                ( paConfig & RF_PACONFIG_MAX_POWER_MASK & RF_PACONFIG_OUTPUTPOWER_MASK ) | ( 0 << 4 ) | ( power + 4 );
        }
    }
    SX1276Write( REG_PACONFIG, paConfig );
    SX1276Write( REG_PADAC, paDac );
}

static uint8_t SX1276GetPaSelect( uint32_t channel )
{
    return RF_PACONFIG_PASELECT_PABOOST;
}

void SX1276SetAntSwLowPower( bool status )
{
    // Control the TCXO and Antenna switch
    if( RadioIsActive != status )
    {
        RadioIsActive = status;
    }
}

void SX1276SetAntSw( uint8_t opMode )
{
}

bool SX1276CheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}

uint32_t SX1276GetDio1PinState( void )
{
    return GpioRead( &SX1276.DIO1 );
}

#if defined( USE_RADIO_DEBUG )
void SX1276DbgPinTxWrite( uint8_t state )
{
    GpioWrite( &DbgPinTx, state );
}

void SX1276DbgPinRxWrite( uint8_t state )
{
    GpioWrite( &DbgPinRx, state );
}
#endif
