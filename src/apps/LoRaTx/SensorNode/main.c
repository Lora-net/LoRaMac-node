/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Ping-Pong implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <string.h>
#include "board.h"
#include "radio.h"

typedef enum
{
    LOWPOWER,
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    TX,
    TX_TIMEOUT,
}States_t;


#define FREQUENCY								470000000
#define POWER									14
#define BANDWIDTH								0
#define DATARATE								7


#define RX_TIMEOUT_VALUE                            1000000
#define BUFFER_SIZE                                 9 // Define the payload size here

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

States_t State = LOWPOWER;

double RssiValue = 0.0;
double SnrValue = 0.0;

uint32_t count=0;

static TimerEvent_t Led2Timer;
volatile bool Led2TimerEvent = false;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
void OnTxDone( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
void OnRxDone( uint8_t *payload, uint16_t *size, double rssi, double snr, uint8_t rawSnr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
void OnRxTimeout( void );

/*!
 * \brief Function executed on Radio Rx Error event
 */
void OnRxError( void );

/*!
 * \brief Function executed on Led 1 Timeout event
 */
void OnLed2TimerEvent( void )
{
    Led2TimerEvent = true;
}

/**
 * Main application entry point.
 */
int main( void )
{
    bool isMaster = true;
    uint8_t i;

    // Target board initialisation
    BoardInitMcu( );
    BoardInitPeriph( );

    // Radio initialization
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    Radio.Init( &RadioEvents );

    Radio.SetChannel( SYS_FREQUENCY );
    Radio.SetTxConfig( MODEM_LORA, POWER, 0, BANDWIDTH, DATARATE, 1, 8, false, true, false, 3000000 );

    TimerInit( &Led2Timer, OnLed2TimerEvent );
    TimerSetValue( &Led2Timer, 1000000 );

    Led2TimerEvent = true;

    while( 1 )
    {
    	if(Led2TimerEvent){
    		Led2TimerEvent = false;
    		GpioWrite( &Led2, 1);
    		Buffer[0] = 'P';
			Buffer[1] = 'I';
			Buffer[2] = 'N';
			Buffer[3] = 'G';
    		Radio.Send( Buffer, 9 );
    	}
        switch( State )
        {
        case RX:
            State = LOWPOWER;
            break;
        case TX:
        	// Switch LED2 ON
			GpioWrite( &Led2, 0 );
			TimerStart( &Led2Timer );
            State = LOWPOWER;
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            State = LOWPOWER;
            break;
        case TX_TIMEOUT:
        	// Switch LED2 ON
			GpioWrite( &Led2, 0 );
			TimerStart( &Led2Timer );
            State = LOWPOWER;
            break;
        case LOWPOWER:
        default:
            // Set low power
            break;
        }
    
        TimerLowPowerHandler( );
    
    }
}

void OnTxDone( void )
{
    State = TX;
}

void OnRxDone( uint8_t *payload, uint16_t *size, double rssi, double snr, uint8_t rawSnr )
{
    Radio.Sleep( );
    State = RX;
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    State = TX_TIMEOUT;
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    State = RX_TIMEOUT;
}

void OnRxError( void )
{
    Radio.Sleep( );
    State = RX_ERROR;
}

