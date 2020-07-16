/*!
 * \file      lr1110mb1xxs-board.c
 *
 * \brief     Target board LR1110MB1XXS shield driver implementation
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
 *              (C)2019-2019 Semtech
 *
 * \endcode
 *
 */
#include <stdlib.h>
#include "utilities.h"
#include "board-config.h"
#include "delay.h"
#include "rtc-board.h"
#include "radio.h"

#include "lr1110_hal.h"
#include "lr1110_radio.h"
#include "lr1110_system.h"
#include "lr1110_regmem.h"

#include "lr1110-board.h"

#define LR1110_SHIELD_HAS_TCXO                      1

#if( LR1110_SHIELD_HAS_TCXO == 1 )
    #undef BOARD_TCXO_WAKEUP_TIME
    #define BOARD_TCXO_WAKEUP_TIME                  5 // 5 milliseconds
#endif

/*!
 * Debug GPIO pins objects
 */
#if defined( USE_RADIO_DEBUG )
Gpio_t DbgPinTx;
Gpio_t DbgPinRx;
#endif

static void lr1110_board_init_tcxo_io( const void* context );

void lr1110_board_init_io( const void* context )
{
    GpioInit( &( ( lr1110_t* ) context )->reset, RADIO_RESET, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &( ( lr1110_t* ) context )->spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &( ( lr1110_t* ) context )->dio_1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &( ( lr1110_t* ) context )->busy, RADIO_BUSY, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void lr1110_board_deinit_io( const void* context )
{
    GpioInit( &( ( lr1110_t* ) context )->reset, RADIO_RESET, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &( ( lr1110_t* ) context )->spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &( ( lr1110_t* ) context )->dio_1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &( ( lr1110_t* ) context )->busy, RADIO_BUSY, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

void lr1110_board_init_dbg_io( const void* context )
{
#if defined( USE_RADIO_DEBUG )
    GpioInit( &DbgPinTx, RADIO_DBG_PIN_TX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &DbgPinRx, RADIO_DBG_PIN_RX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
}

void lr1110_board_set_rf_tx_power( const void* context, int8_t power )
{
    // TODO: Add PA Config check
    if( power > 0 )
    {
        if( power > 22 )
        {
            power = 22;
        }
    }
    else
    {
        if( power < -9 )
        {
            power = -9;
        }
    }
    lr1110_radio_set_tx_params( context, power, LR1110_RADIO_RAMP_TIME_40U );
}

uint32_t lr1110_board_get_tcxo_wakeup_time( const void* context )
{
    return BOARD_TCXO_WAKEUP_TIME;
}

void lr1110_board_init( const void* context, lr1110_dio_irq_handler dio_irq )
{
    lr1110_system_reset( context );
    lr1110_hal_set_operating_mode( context, LR1110_HAL_OP_MODE_STDBY_RC );

    // Attach interrupt handler to radio irq pin
    GpioSetInterrupt( &( ( lr1110_t* ) context )->dio_1, IRQ_RISING_EDGE, IRQ_HIGH_PRIORITY, dio_irq );

    lr1110_system_stat1_t stat1;
    lr1110_system_stat2_t stat2;
    uint32_t              irq = 0;
    lr1110_system_get_status( context, &stat1, &stat2, &irq );
    lr1110_system_version_t version;
    lr1110_system_get_version( context, &version );
    lr1110_system_errors_t errors = { 0 };
    lr1110_system_get_errors( context, &errors );
    lr1110_system_clear_errors( context );

    // Initialize TCXO control
    lr1110_board_init_tcxo_io( context );

    // Initialize RF switch control
    lr1110_system_rfswitch_config_t rf_switch_configuration;
    rf_switch_configuration.enable  = LR1110_SYSTEM_RFSW0_HIGH | LR1110_SYSTEM_RFSW1_HIGH;
    rf_switch_configuration.standby = 0;
    rf_switch_configuration.rx      = LR1110_SYSTEM_RFSW0_HIGH;
    rf_switch_configuration.tx      = LR1110_SYSTEM_RFSW0_HIGH | LR1110_SYSTEM_RFSW1_HIGH;
    rf_switch_configuration.wifi    = 0;
    rf_switch_configuration.gnss    = 0;

    lr1110_system_set_dio_as_rf_switch( context, &rf_switch_configuration );

    lr1110_radio_pa_config_t paConfig = {
        .pa_sel        = LR1110_RADIO_PA_SEL_LP,
        .pa_reg_supply = LR1110_RADIO_PA_REG_SUPPLY_DCDC,
        .pa_dutycycle  = 0x04,
        .pa_hp_sel     = 0x00,
    };
    lr1110_radio_set_pa_config( context, &paConfig );

    // Set packet type
    lr1110_radio_packet_types_t packet_type = LR1110_RADIO_PACKET_LORA;
    lr1110_radio_set_packet_type( context, packet_type );
}

static void lr1110_board_init_tcxo_io( const void* context )
{
#if( LR1110_SHIELD_HAS_TCXO == 1 )
    lr1110_system_set_tcxo_mode( context, LR1110_SYSTEM_TCXO_SUPPLY_VOLTAGE_1_8V,
                                 ( lr1110_board_get_tcxo_wakeup_time( context ) * 1000 ) / 30.52 );

    uint8_t calib_params = LR1110_SYSTEM_CALIBRATE_LF_RC_MASK | LR1110_SYSTEM_CALIBRATE_HF_RC_MASK |
                  LR1110_SYSTEM_CALIBRATE_PLL_MASK | LR1110_SYSTEM_CALIBRATE_ADC_MASK |
                  LR1110_SYSTEM_CALIBRATE_IMG_MASK | LR1110_SYSTEM_CALIBRATE_PLL_TX_MASK;
    lr1110_system_calibrate( context, calib_params );
#endif
}

//
// lr1110_hal.h API implementation
//

static lr1110_hal_status_t lr1110_hal_wait_on_busy( const void* context );

lr1110_hal_status_t lr1110_hal_write( const void* context, const uint8_t* command, const uint16_t command_length,
                                      const uint8_t* data, const uint16_t data_length )

{
    if( lr1110_hal_wakeup( context ) == LR1110_HAL_STATUS_OK )
    {
        GpioWrite( &( ( lr1110_t* ) context )->spi.Nss, 0 );
        for( uint16_t i = 0; i < command_length; i++ )
        {
            SpiInOut( &( ( lr1110_t* ) context )->spi, command[i] );
        }
        for( uint16_t i = 0; i < data_length; i++ )
        {
            SpiInOut( &( ( lr1110_t* ) context )->spi, data[i] );
        }
        GpioWrite( &( ( lr1110_t* ) context )->spi.Nss, 1 );

        // 0x011B - LR1110_SYSTEM_SET_SLEEP_OC
        if( ( ( command[0] << 8 ) | command[1] ) != 0x011B )
        {
            return lr1110_hal_wait_on_busy( context );
        }
        else
        {
            return LR1110_HAL_STATUS_OK;
        }
    }
    return LR1110_HAL_STATUS_ERROR;
}

lr1110_hal_status_t lr1110_hal_read( const void* context, const uint8_t* command, const uint16_t command_length,
                                     uint8_t* data, const uint16_t data_length )
{
    if( lr1110_hal_wakeup( context ) == LR1110_HAL_STATUS_OK )
    {
        GpioWrite( &( ( lr1110_t* ) context )->spi.Nss, 0 );

        for( uint16_t i = 0; i < command_length; i++ )
        {
            SpiInOut( &( ( lr1110_t* ) context )->spi, command[i] );
        }

        GpioWrite( &( ( lr1110_t* ) context )->spi.Nss, 1 );

        lr1110_hal_wait_on_busy( context );

        // Send dummy byte
        GpioWrite( &( ( lr1110_t* ) context )->spi.Nss, 0 );

        SpiInOut( &( ( lr1110_t* ) context )->spi, 0 );

        for( uint16_t i = 0; i < data_length; i++ )
        {
            data[i] = SpiInOut( &( ( lr1110_t* ) context )->spi, 0 );
        }

        GpioWrite( &( ( lr1110_t* ) context )->spi.Nss, 1 );

        return lr1110_hal_wait_on_busy( context );
    }
    return LR1110_HAL_STATUS_ERROR;
}

lr1110_hal_status_t lr1110_hal_write_read( const void* context, const uint8_t* command, uint8_t* data,
                                           const uint16_t data_length )
{
    if( lr1110_hal_wakeup( context ) == LR1110_HAL_STATUS_OK )
    {
        GpioWrite( &( ( lr1110_t* ) context )->spi.Nss, 0 );

        for( uint16_t i = 0; i < data_length; i++ )
        {
            data[i] = SpiInOut( &( ( lr1110_t* ) context )->spi, command[i] );
        }

        GpioWrite( &( ( lr1110_t* ) context )->spi.Nss, 1 );

        // 0x011B - LR1110_SYSTEM_SET_SLEEP_OC
        if( ( ( command[0] << 8 ) | command[1] ) != 0x011B )
        {
            return lr1110_hal_wait_on_busy( context );
        }
        else
        {
            return LR1110_HAL_STATUS_OK;
        }
    }
    return LR1110_HAL_STATUS_ERROR;
}

void lr1110_hal_reset( const void* context )
{
    GpioWrite( &( ( lr1110_t* ) context )->reset, 0 );
    DelayMs( 1 );
    GpioWrite( &( ( lr1110_t* ) context )->reset, 1 );
}

lr1110_hal_status_t lr1110_hal_wakeup( const void* context )
{
    if( ( lr1110_hal_get_operating_mode( context ) == LR1110_HAL_OP_MODE_SLEEP ) ||
        ( lr1110_hal_get_operating_mode( context ) == LR1110_HAL_OP_MODE_RX_DC ) )
    {
        // Wakeup radio
        GpioWrite( &( ( lr1110_t* ) context )->spi.Nss, 0 );
        GpioWrite( &( ( lr1110_t* ) context )->spi.Nss, 1 );

        // Radio is awake in STDBY_RC mode
        ( ( lr1110_t* ) context )->op_mode = LR1110_HAL_OP_MODE_STDBY_RC;
    }

    // Wait on busy pin for 100 ms
    return lr1110_hal_wait_on_busy( context );
}

static lr1110_hal_status_t lr1110_hal_wait_on_busy( const void* context )
{
    while( GpioRead( &( ( lr1110_t* ) context )->busy ) == 1 )
    {
        ;
    }
    return LR1110_HAL_STATUS_OK;
}

lr1110_hal_operating_mode_t lr1110_hal_get_operating_mode( const void* context )
{
    return ( ( lr1110_t* ) context )->op_mode;
}

void lr1110_hal_set_operating_mode( const void* context, lr1110_hal_operating_mode_t op_mode )
{
    ( ( lr1110_t* ) context )->op_mode = op_mode;

#if defined( USE_RADIO_DEBUG )
    switch( op_mode )
    {
    case LR1110_HAL_OP_MODE_TX:
        GpioWrite( &DbgPinTx, 1 );
        GpioWrite( &DbgPinRx, 0 );
        break;
    case LR1110_HAL_OP_MODE_RX:
    case LR1110_HAL_OP_MODE_RX_C:
    case LR1110_HAL_OP_MODE_RX_DC:
        GpioWrite( &DbgPinTx, 0 );
        GpioWrite( &DbgPinRx, 1 );
        break;
    default:
        GpioWrite( &DbgPinTx, 0 );
        GpioWrite( &DbgPinRx, 0 );
        break;
    }
#endif
}
