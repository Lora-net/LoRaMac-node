/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: LoRaMac class1 device implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
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
#define APP_DATA_SIZE                               16

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

static LoRaMacEvent_t LoRaMacEvents;

static TimerEvent_t Led1Timer;
volatile bool Led1TimerEvent = false;

#if( OVER_THE_AIR_ACTIVATION != 0 )

/*!
 * \brief Function executed on Led 1 Timeout event
 */
void OnJoinReqTimerEvent( void )
{
    TxNextPacket = true;
}

#endif

/*!
 * \brief Function executed on Led 1 Timeout event
 */
void OnTxNextPacketTimerEvent( void )
{
    TxNextPacket = true;
}

/*!
 * \brief Function executed on Led 1 Timeout event
 */
void OnLed1TimerEvent( void )
{
    Led1TimerEvent = true;
}

/*!
 * \brief Function to be executed on MAC layer event
 */
void OnMacEvent( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info )
{
    if( info->Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT )
    {
        TxDone = true;
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
        TxDone = true;
    }
}

/**
 * Main application entry point.
 */
int main( void )
{
    uint16_t pressure = 0;
    uint16_t altitudeBar = 0;
    int16_t temperature = 0;
    int32_t latitude, longitude = 0;
    uint16_t altitudeGps = 0xFFFF;
    uint8_t batteryLevel = 0;   

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
    DevAddr = randr( 0, 0x0FFFFFFF );

    LoRaMacInitNwkIds( DevAddr, NwkSKey, AppSKey );
    
    IsNetworkJoined = true;
#else
    // Sends a JoinReq Command every 5 seconds until the network is joined
    TimerInit( &JoinReqTimer, OnJoinReqTimerEvent ); 
    TimerSetValue( &JoinReqTimer, OVER_THE_AIR_ACTIVATION_DUTYCYCLE );
    TimerStart( &JoinReqTimer );
#endif

    TxNextPacket = true;
    TimerInit( &TxNextPacketTimer, OnTxNextPacketTimerEvent );
    
    TimerInit( &Led1Timer, OnLed1TimerEvent ); 
    TimerSetValue( &Led1Timer, 25000 );

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
        if( Led1TimerEvent == true )
        {
            Led1TimerEvent = false;
            
            // Switch LED 1 OFF
            GpioWrite( &Led1, 1 );
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
        
            pressure = ( uint16_t )( mpl3115ReadPressure( ) / 10 );             // in hPa / 10
            temperature = ( int16_t )( mpl3115ReadTemperature( ) * 100 );       // in �C * 100
            altitudeBar = ( uint16_t )( mpl3115ReadAltitude( ) * 10 );          // in m * 10
            batteryLevel = BoardMeasureBatterieLevel( );                        // 1 (very low) to 254 (fully charged)
            SIM39EAGetLatestGpsPositionBinary( &latitude, &longitude );
            altitudeGps = SIM39EAGetLatestGpsAltitude( );                         // in m
        
            // Switch LED 1 ON
            GpioWrite( &Led1, 0 );
            TimerStart( &Led1Timer );
        
            AppData[0] = 0x00;
            AppData[1] = ( pressure >> 8 ) & 0xFF;
            AppData[2] = pressure & 0xFF;
            AppData[3] = ( temperature >> 8 ) & 0xFF;
            AppData[4] = temperature & 0xFF;
            AppData[5] = ( altitudeBar >> 8 ) & 0xFF;
            AppData[6] = altitudeBar & 0xFF;
            AppData[7] = batteryLevel;
            AppData[8] = ( latitude >> 16 ) & 0xFF;
            AppData[9] = ( latitude >> 8 ) & 0xFF;
            AppData[10] = latitude & 0xFF;
            AppData[11] = ( longitude >> 16 ) & 0xFF;
            AppData[12] = ( longitude >> 8 ) & 0xFF;
            AppData[13] = longitude & 0xFF;
            AppData[14] = ( altitudeGps >> 8 ) & 0xFF;
            AppData[15] = altitudeGps & 0xFF;
            
            LoRaMacSendFrame( 2, AppData, APP_DATA_SIZE );
        }

        TimerLowPowerHandler( );
    }
}

