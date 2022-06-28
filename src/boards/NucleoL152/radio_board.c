/*!
 * \file  radio_board.c
 *
 * \brief Radio board Hardware Abstraction Layer
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */
#include <stdlib.h>
#include "board-config.h"
#include "delay.h"
#include "radio_board.h"
#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )
#include "sx126x_lr_fhss.h"
#include "ral_sx126x.h"
#elif defined( LR1110MB1XXS ) || defined( LR1120MB1XXS )
#include "ral_lr11xx.h"
#elif defined( SX1272MB2DAS ) || defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
#include "ral_sx127x.h"
#else
#error "Please define a radio shield to be used"
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE MACROS-----------------------------------------------------------
 */

#if defined( SX1261MBXBAS ) || defined( SX1262MBXDAS )
#define RADIO_SHIELD_HAS_TCXO 0
#elif defined( SX1262MBXCAS ) || defined( LR1110MB1XXS ) || defined( LR1120MB1XXS )
#define RADIO_SHIELD_HAS_TCXO 1
#elif defined( SX1272MB2DAS ) || defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
#define RADIO_SHIELD_HAS_TCXO 0
#else
#error "Please define a radio shield to be used"
#endif

#if( RADIO_SHIELD_HAS_TCXO == 1 )
#undef RADIO_BOARD_TCXO_WAKEUP_TIME
#define RADIO_BOARD_TCXO_WAKEUP_TIME 5  // 5 milliseconds
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE TYPES -----------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE CONSTANTS -------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE VARIABLES -------------------------------------------------------
 */

/*!
 * Board context
 */
static radio_context_t radio_context_reference;

#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )

#if( LORAMAC_LR_FHSS_IS_ON == 1 )
/*!
 * LR-FHSS state storage
 */
static sx126x_lr_fhss_state_t sx126x_lr_fhss_state;
#endif

#elif defined( LR1110MB1XXS ) || defined( LR1120MB1XXS )

#elif defined( SX1272MB2DAS ) || defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
/*!
 * Board context
 */
static radio_context_t radio_context_reference;

/*!
 * SX127X context
 */
static sx127x_t sx127x_context_reference = {
    .hal_context = &radio_context_reference,
};
#endif

/*!
 * RAL context
 */
#if defined( SX126X )
static ral_t ral_context_reference = RAL_SX126X_INSTANTIATE( &radio_context_reference );
#elif defined( LR11XX )
static ral_t ral_context_reference = RAL_LR11XX_INSTANTIATE( &radio_context_reference );
#elif defined( SX127X )
static ral_t ral_context_reference = RAL_SX127X_INSTANTIATE( &sx127x_context_reference );
#endif

#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )
/*!
 * Antenna switch GPIO pins objects
 */
Gpio_t ant_power;

/*!
 * Device selection GPIO pins objects
 */
Gpio_t device_sel;
#elif defined( LR1110MB1XXS ) || defined( LR1120MB1XXS )
#elif defined( SX1272MB2DAS ) || defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
/*!
 * Antenna switch GPIO pins objects
 */
Gpio_t ant_sw;
#endif

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DECLARATION -------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS DEFINITION ---------------------------------------------
 */

radio_context_t* radio_board_get_radio_context_reference( void )
{
    return &radio_context_reference;
}

#if( LORAMAC_LR_FHSS_IS_ON == 1 )
void* radio_board_get_lr_fhss_state_reference( void )
{
#if defined( SX126X )
    return ( void* ) &sx126x_lr_fhss_state;
#else
    return NULL;
#endif
}
#endif

#if defined( SX1272MB2DAS ) || defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
sx127x_t* radio_board_get_sx127x_context_reference( void )
{
    return &sx127x_context_reference;
}
#endif

ral_t* radio_board_get_ral_context_reference( void )
{
    return &ral_context_reference;
}

ral_status_t radio_board_init( const ral_t* ral_context, radio_board_dio_irq_handler dio_irq )
{
#if defined( SX126X )
    return ral_sx126x_bsp_init( ral_context, dio_irq );
#elif defined( LR11XX )
    return ral_lr11xx_bsp_init( ral_context, dio_irq );
#elif defined( SX127X )
    return ral_sx127x_bsp_init( ral_context, dio_irq );
#endif
}

void radio_board_init_io( void )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )
    GpioInit( &radio_context->reset, RADIO_RESET, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &radio_context->spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &radio_context->busy, RADIO_BUSY, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &device_sel, RADIO_DEVICE_SEL, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#elif defined( LR1110MB1XXS ) || defined( LR1120MB1XXS )
    GpioInit( &radio_context->reset, RADIO_RESET, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &radio_context->spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &radio_context->busy, RADIO_BUSY, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#elif defined( SX1272MB2DAS ) || defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
    GpioInit( &radio_context->spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &radio_context->dio_0, RADIO_DIO_0, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_2, RADIO_DIO_2, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ant_sw, RADIO_ANT_SWITCH, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
}

void radio_board_deinit_io( void )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )
    GpioInit( &radio_context->reset, RADIO_RESET, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &radio_context->spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &radio_context->busy, RADIO_BUSY, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#elif defined( LR1110MB1XXS ) || defined( LR1120MB1XXS )
    GpioInit( &radio_context->reset, RADIO_RESET, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &radio_context->spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &radio_context->busy, RADIO_BUSY, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#elif defined( SX1272MB2DAS ) || defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
    GpioInit( &radio_context->spi.Nss, RADIO_NSS, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    GpioInit( &radio_context->dio_0, RADIO_DIO_0, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_1, RADIO_DIO_1, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dio_2, RADIO_DIO_2, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &ant_sw, RADIO_ANT_SWITCH, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
}

void radio_board_init_dbg_io( void )
{
#if defined( USE_RADIO_DEBUG )
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    GpioInit( &radio_context->dbg_tx, RADIO_DBG_PIN_TX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    GpioInit( &radio_context->dbg_rx, RADIO_DBG_PIN_RX, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
#endif
}

void radio_board_set_ant_switch( bool is_tx_on )
{
#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS ) || defined( LR1110MB1XXS ) || \
    defined( LR1120MB1XXS )
    // Antenna switch is controlled by the radio on this board design. Nothing to do.
#elif defined( SX1272MB2DAS ) || defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
    GpioWrite( &ant_sw, ( is_tx_on == true ) ? 1 : 0 );
#endif
}

void radio_board_start_radio_tcxo( void )
{
#if defined( SX1261MBXBAS ) || defined( SX1262MBXDAS )
    // No TCXO component available on this board design. Nothing to do.
#elif defined( SX1262MBXCAS ) || defined( LR1110MB1XXS ) || defined( LR1120MB1XXS )
    // TCXO is controlled by the radio on this board design. Nothing to do.
#elif defined( SX1272MB2DAS ) || defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
    // No TCXO component available on this board design. Nothing to do.
#endif
}

void radio_board_stop_radio_tcxo( void )
{
#if defined( SX1261MBXBAS ) || defined( SX1262MBXDAS )
    // No TCXO component available on this board design. Nothing to do.
#elif defined( SX1262MBXCAS ) || defined( LR1110MB1XXS ) || defined( LR1120MB1XXS )
    // TCXO is controlled by the radio on this board design. Nothing to do.
#elif defined( SX1272MB2DAS ) || defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
    // No TCXO component available on this board design. Nothing to do.
#endif
}

uint32_t radio_board_get_tcxo_wakeup_time_in_ms( void )
{
#if defined( SX1261MBXBAS ) || defined( SX1262MBXDAS )
    // No TCXO component available on this board design.
    // return 0 ms delay
    return 0;
#elif defined( SX1262MBXCAS ) || defined( LR1110MB1XXS ) || defined( LR1120MB1XXS )
    // TCXO component available on this board design has RADIO_BOARD_TCXO_WAKEUP_TIME ms wake up time.
    // return RADIO_BOARD_TCXO_WAKEUP_TIME ms delay
    return RADIO_BOARD_TCXO_WAKEUP_TIME;
#elif defined( SX1272MB2DAS ) || defined( SX1276MB1LAS ) || defined( SX1276MB1MAS )
    // No TCXO component available on this board design.
    // return 0 ms delay
    return 0;
#endif
}

radio_board_operating_mode_t radio_board_get_operating_mode( void )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    return radio_context->op_mode;
}

void radio_board_set_operating_mode( radio_board_operating_mode_t op_mode )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    radio_context->op_mode = op_mode;
#if defined( SX1261MBXBAS ) || defined( SX1262MBXCAS ) || defined( SX1262MBXDAS )
    if( op_mode == RADIO_BOARD_OP_MODE_SLEEP )
    {
        // Antenna switch is turned off
        GpioInit( &ant_power, RADIO_ANT_SWITCH_POWER, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
    }
    else
    {
        // Antenna switch is turned on
        GpioInit( &ant_power, RADIO_ANT_SWITCH_POWER, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 1 );
    }
#endif
#if defined( USE_RADIO_DEBUG )
    switch( op_mode )
    {
    case RADIO_BOARD_OP_MODE_TX:
        GpioWrite( &radio_context->dbg_tx, 1 );
        GpioWrite( &radio_context->dbg_rx, 0 );
        break;
    case RADIO_BOARD_OP_MODE_RX:
    case RADIO_BOARD_OP_MODE_RX_C:
    case RADIO_BOARD_OP_MODE_RX_DC:
        GpioWrite( &radio_context->dbg_tx, 0 );
        GpioWrite( &radio_context->dbg_rx, 1 );
        break;
    default:
        GpioWrite( &radio_context->dbg_tx, 0 );
        GpioWrite( &radio_context->dbg_rx, 0 );
        break;
    }
#endif
}

uint32_t radio_board_get_dio_1_pin_state( void )
{
    radio_context_t* radio_context = radio_board_get_radio_context_reference( );

    return GpioRead( &radio_context->dio_1 );
}

/*
 * -----------------------------------------------------------------------------
 * --- PRIVATE FUNCTIONS DEFINITION --------------------------------------------
 */

/* --- EOF ------------------------------------------------------------------ */
