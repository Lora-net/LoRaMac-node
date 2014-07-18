/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: LoRaMac classA device implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Andreas Pella (IMST GmbH), Miguel Luis and Gregory Cristian
*/
#include <string.h>
#include <math.h>
#include "board.h"
#include "radio.h"

#include "LoRaMac.h"

/*!
 * When set to 1 the application uses the Over-the-Air activation procedure
 * When set to 0 the application uses the Personalization activation procedure
 */
#define OVER_THE_AIR_ACTIVATION                     0

/*!
 * Mote device IEEE EUI
 */
static uint8_t DevEui[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#if( OVER_THE_AIR_ACTIVATION != 0 )

#define OVER_THE_AIR_ACTIVATION_DUTYCYCLE          10000000  // 10 [s] value in us

/*!
 * Application IEEE EUI
 */
static uint8_t AppEui[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*!
 * AES encryption/decryption cipher application key
 */
static uint8_t AppKey[] = 
{ 
    0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

#else

/*!
 * AES encryption/decryption cipher network session key
 */
static uint8_t NwkSKey[] = 
{ 
    0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

/*!
 * AES encryption/decryption cipher application session key
 */
static uint8_t AppSKey[] = 
{ 
    0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
    0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

/*!
 * Device address
 */
static uint32_t DevAddr;

#endif

/*!
 * Indicates if the MAC layer has already joined a network.
 */
static bool IsNetworkJoined = false;

/*!
 * Defines the application data transmission duty cycle
 */
#define APP_TX_DUTYCYCLE                            5000000  // 5 [s] value in us
#define APP_TX_DUTYCYCLE_RND                        1000000  // 1 [s] value in us

/*!
 * User application data buffer size
 */
#define APP_DATA_SIZE                               4

/*!
 * User application data
 */
static uint8_t AppData[APP_DATA_SIZE];

/*!
 * Defines the application data transmission duty cycle
 */
static uint32_t TxDutyCycleTime;

static TimerEvent_t TxNextPacketTimer;

#if( OVER_THE_AIR_ACTIVATION != 0 )

/*!
 * Defines the join request timer
 */
static TimerEvent_t JoinReqTimer;

#endif

/*!
 * Indicates if a new packet can be sent
 */
static bool TxNextPacket = true;
static bool TxDone = false;
static bool RxDone = false;
static bool TxAckReceived = false;

static bool AppLedStateOn = false;

static LoRaMacEvent_t LoRaMacEvents;

static TimerEvent_t Led4Timer;
volatile bool Led4TimerEvent = false;

static TimerEvent_t Led2Timer;
volatile bool Led2TimerEvent = false;

#if( OVER_THE_AIR_ACTIVATION != 0 )

/*!
 * \brief Function executed on Led 4 Timeout event
 */
void OnJoinReqTimerEvent( void )
{
    TxNextPacket = true;
}

#endif

/*!
 * \brief Function executed on Led 4 Timeout event
 */
void OnTxNextPacketTimerEvent( void )
{
    TxNextPacket = true;
}

/*!
 * \brief Function executed on Led 4 Timeout event
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
 * \brief Function to be executed on MAC layer event
 */
void OnMacEvent( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info )
{
    if( info->Status == LORAMAC_EVENT_INFO_STATUS_ERROR )
    {
        // Schedule a new transmission
        TxDone = true;
        return;
    }

    if( flags->Bits.JoinAccept == 1 )
    {
#if( OVER_THE_AIR_ACTIVATION != 0 )
        TimerStop( &JoinReqTimer );
#endif
        IsNetworkJoined = true;
    }
    
    if( flags->Bits.Tx == 1 )
    {
        if( info->TxAckReceived == true )
        {
            TxAckReceived = true;
        }
        // Schedule a new transmission
        TxDone = true;
    }

    if( flags->Bits.Rx == 1 )
    {
        if( ( ( info->RxPort == 1 ) || ( info->RxPort == 3 ) ) && ( info->RxBufferSize > 0 ) )
        {
            AppLedStateOn = info->RxBuffer[0];
        }
        RxDone = true;
    }
}

/**
 * Main application entry point.
 */
int main( void )
{
    uint8_t sendFrameStatus = 0;
    uint8_t potiPercentage = 0;
    uint16_t vdd = 0;

    BoardInitMcu( );
    BoardInitPeriph( );

    // Initialize LoRaMac device unique ID
    BoardGetUniqueId( DevEui );

    LoRaMacEvents.MacEvent = OnMacEvent;
    LoRaMacInit( &LoRaMacEvents );

    IsNetworkJoined = false;

#if( OVER_THE_AIR_ACTIVATION == 0 )
    // Random seed initialization
    srand( RAND_SEED );
    // Choose a random device address
    // NwkID = 0
    // NwkAddr rand [0, 33554431]
    DevAddr = randr( 0, 0x01FFFFFF );

    LoRaMacInitNwkIds( 0x000000, DevAddr, NwkSKey, AppSKey );
    IsNetworkJoined = true;
#else
    // Sends a JoinReq Command every 5 seconds until the network is joined
    TimerInit( &JoinReqTimer, OnJoinReqTimerEvent ); 
    TimerSetValue( &JoinReqTimer, OVER_THE_AIR_ACTIVATION_DUTYCYCLE );
#endif

    TxNextPacket = true;
    TimerInit( &TxNextPacketTimer, OnTxNextPacketTimerEvent );
    
    TimerInit( &Led4Timer, OnLed4TimerEvent ); 
    TimerSetValue( &Led4Timer, 25000 );

    TimerInit( &Led2Timer, OnLed2TimerEvent ); 
    TimerSetValue( &Led2Timer, 25000 );

    LoRaMacSetAdrOn( true );

    while( 1 )
    {
        while( IsNetworkJoined == false )
        {
#if( OVER_THE_AIR_ACTIVATION != 0 )
            if( TxNextPacket == true )
            {
                TxNextPacket = false;
                
                LoRaMacJoinReq( DevEui, AppEui, AppKey );

                // Relaunch timer for next trial
                TimerStart( &JoinReqTimer );
            }
            TimerLowPowerHandler( );
#endif
        }
        if( Led4TimerEvent == true )
        {
            Led4TimerEvent = false;
            
            // Switch LED 4 OFF
            GpioWrite( &Led4, 0 );
        }

        if( Led2TimerEvent == true )
        {
            Led2TimerEvent = false;
            
            // Switch LED 2 OFF
            GpioWrite( &Led2, 0 );
        }

        if( TxAckReceived == true )
        {
            TxAckReceived = false;
            // Switch LED 2 ON
            GpioWrite( &Led2, 1 );
            TimerStart( &Led2Timer );
        }
        
        if( RxDone == true )
        {
            RxDone = false;
            
            // Switch LED 2 ON
            GpioWrite( &Led2, 1 );
            TimerStart( &Led2Timer );

            if( AppLedStateOn == true )
            {
                // Switch LED 3 ON
                GpioWrite( &Led3, 1 );
            }
            else
            {
                // Switch LED 3 OFF
                GpioWrite( &Led3, 0 );
            }
        }
        
        if( TxDone == true )
        {
            TxDone = false;
            
            // Schedule next packet transmission
            TxDutyCycleTime = APP_TX_DUTYCYCLE + randr( -APP_TX_DUTYCYCLE_RND, APP_TX_DUTYCYCLE_RND );
            TimerSetValue( &TxNextPacketTimer, TxDutyCycleTime );
            TimerStart( &TxNextPacketTimer );
        }

        if( TxNextPacket == true )
        {
            TxNextPacket = false;

            // Read the current potentiometer setting in percent
            potiPercentage = BoardMeasurePotiLevel( );

            // Read the current voltage level
            vdd = BoardMeasureVdd( );
        
            // Switch LED 4 ON
            GpioWrite( &Led4, 1 );
            TimerStart( &Led4Timer );
        
            AppData[0] = AppLedStateOn;
            AppData[1] = potiPercentage;
            AppData[2] = ( vdd >> 8 ) & 0xFF;
            AppData[3] = vdd & 0xFF;
            
            sendFrameStatus = LoRaMacSendFrame( 3, AppData, APP_DATA_SIZE );
            //sendFrameStatus = LoRaMacSendConfirmedFrame( 3, AppData, APP_DATA_SIZE, 8 );
            switch( sendFrameStatus )
            {
            case 3: // LENGTH_PORT_ERROR
            case 4: // MAC_CMD_ERROR
            case 5: // NO_FREE_CHANNEL
                // Schedule a new transmission
                TxDone = true;
                break;
            default:
                break;
            }
        }

        TimerLowPowerHandler( );
    }
}

