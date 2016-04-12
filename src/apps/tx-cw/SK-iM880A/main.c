/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Tx Continuous Wave implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <string.h>
#include "board.h"
#include "radio.h"

#define RF_FREQUENCY                                868000000 // Hz
#define TX_OUTPUT_POWER                             20        // 20 dBm

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       9         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false
#define LORA_IQ_INVERSION_ON                        false

static TimerEvent_t Led4Timer;
volatile bool Led4TimerEvent = false;

static TimerEvent_t Led2Timer;
volatile bool Led2TimerEvent = false;

static TimerEvent_t Led3Timer;
volatile bool Led3TimerEvent = false;

/*!
 * \brief Function executed on Led 1 Timeout event
 */
void OnLed4TimerEvent( void )
{
    Led4TimerEvent = true;
}

/*!
 * \brief Function executed on Led 2 Timeout event
 */
void OnLed2TimerEvent( void )
{
    Led2TimerEvent = true;
}

/*!
 * \brief Function executed on Led 3 Timeout event
 */
void OnLed3TimerEvent( void )
{
    Led3TimerEvent = true;
}

/**
 * Main application entry point.
 */
int main( void )
{
    // Target board initialisation
    BoardInitMcu( );
    BoardInitPeriph( );

    // Radio initialization
    Radio.Init( NULL );

    Radio.SetChannel( RF_FREQUENCY );

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
    
    /**********************************************/
    /*                  WARNING                   */
    /* The below settings can damage the chipset  */
    /* if wrongly used. DO NOT CHANGE THE VALUES! */
    /*                                            */
    /**********************************************/

    Radio.Write( 0x4B, 0x7B );
    Radio.Write( 0x3D, 0xAF );
    Radio.Write( 0x1e, 0x08 );
    Radio.Write( 0x4C, 0xC0 );
    Radio.Write( 0x4D, 0x03 );
    Radio.Write( 0x5A, 0x87 );
    Radio.Write( 0x63, 0x60 );
    Radio.Write( 0x01, 0x83 );

    TimerInit( &Led4Timer, OnLed4TimerEvent ); 
    TimerSetValue( &Led4Timer, 90 );

    TimerInit( &Led2Timer, OnLed2TimerEvent ); 
    TimerSetValue( &Led2Timer, 90 );

    TimerInit( &Led3Timer, OnLed3TimerEvent ); 
    TimerSetValue( &Led3Timer, 90 );
               
    // Switch LED 1 ON
    GpioWrite( &Led4, 0 );
    TimerStart( &Led4Timer );
    
    // Sets the radio in Tx mode
    Radio.Send( NULL, 0 );

    // Blink LEDs just to show some activity
    while( 1 )
    {
        if( Led4TimerEvent == true )
        {
            Led4TimerEvent = false;
            
            // Switch LED 4 OFF
            GpioWrite( &Led4, 4 );
            // Switch LED 2 ON
            GpioWrite( &Led2, 0 );
            TimerStart( &Led2Timer );
        }

        if( Led2TimerEvent == true )
        {
            Led2TimerEvent = false;
            
            // Switch LED 2 OFF
            GpioWrite( &Led2, 1 );
            // Switch LED 3 ON
            GpioWrite( &Led3, 0 );
            TimerStart( &Led3Timer );
        }
    
        if( Led3TimerEvent == true )
        {
            Led3TimerEvent = false;
            
            // Switch LED 3 OFF
            GpioWrite( &Led3, 1 );
            // Switch LED 1 ON
            GpioWrite( &Led4, 0 );
            TimerStart( &Led4Timer );
        }    
    }
}
