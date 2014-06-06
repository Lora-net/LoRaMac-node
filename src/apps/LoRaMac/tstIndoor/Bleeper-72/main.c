/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: LoRaMac indoor test implementation

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
 * User application data buffer size
 */
#define APP_DATA_SIZE                               1

/*!
 * User application data
 */
static uint8_t AppData[APP_DATA_SIZE];

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

static TimerEvent_t StopTimer;
volatile bool StopTimerEvent = false;

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
 * \brief Function executed on Stop Timeout event
 */
void OnStopTimerEvent( void )
{
    StopTimerEvent = true;
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
    //                                   LC1        LC2        LC3        LC4        LC5        LC6
    const uint32_t channelsFreq[] = { 868100000, 868300000, 868500000, 868200000, 868600000, 869300000 };
    const uint8_t  channelsDatarate[] = { DR_SF7, DR_SF10, DR_SF12 };
    uint8_t tstState = 0;
    int16_t pktCnt = 15;
    ChannelParams_t channel;
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;
    uint8_t channelsIndex = 0;
    uint8_t datarateIndex = 0;
    
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
    
    TimerInit( &Led1Timer, OnLed1TimerEvent ); 
    TimerSetValue( &Led1Timer, 25000 );

    // Low power timer to be run when tests are finished.
    TimerInit( &StopTimer, OnStopTimerEvent ); 
    TimerSetValue( &StopTimer, 3.6e9 ); // wakes up the microcontroller every hour

    // Initialize MAC frame
    macHdr.Value = 0;
    macHdr.Bits.MType = FRAME_TYPE_DATA_UNCONFIRMED;

    fCtrl.Value = 0;
    fCtrl.Bits.OptionsLength = 0;
    fCtrl.Bits.FPending      = 0;
    fCtrl.Bits.Ack           = false;
    fCtrl.Bits.AdrAckReq     = false;
    fCtrl.Bits.Adr           = false;

    // Initialize channel
    channel.DrRange.Fields.Min = DR_SF12;
    channel.DrRange.Fields.Min = DR_SF7;
    channel.DutyCycle = 0;
    LoRaMacSetChannelsTxPower( TX_POWER_14_DBM );
    
    // Disable reception windows opening
    LoRaMacTestRxWindowsOn( false );
    
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

        for( datarateIndex = 0; datarateIndex < 3; datarateIndex++ )
        {
            //for( channelsIndex = 0; channelsIndex < 3; channelsIndex++ )
            {
                pktCnt = 15 * ( sizeof( channelsFreq ) / sizeof( uint32_t ) );
                while( pktCnt > 0 )
                {
                    switch( tstState )
                    {
                        case 0: // Init
                            AppData[0] = SelectorGetValue( );
                            
                            channel.Frequency = channelsFreq[channelsIndex];
                            LoRaMacSetChannelsDatarate( channelsDatarate[datarateIndex] );
                            LoRaMacSendOnChannel( channel, &macHdr, &fCtrl, NULL, 15, AppData, 1 );

                            // Switch LED 1 ON
                            GpioWrite( &Led1, 0 );
                            TimerStart( &Led1Timer );

                            channelsIndex = ( channelsIndex + 1 ) % 6;
                            tstState = 1;
                            break;
                        case 1: // Wait for end of transmission
                            if( Led1TimerEvent == true )
                            {
                                Led1TimerEvent = false;
                                
                                // Switch LED 1 OFF
                                GpioWrite( &Led1, 1 );
                            }
                            if( TxDone == true )
                            {
                                TxDone = false;
                                pktCnt--;
                                // Schedule next packet transmission after 100 ms
                                TimerSetValue( &TxNextPacketTimer, 100000 );
                                TimerStart( &TxNextPacketTimer );
                                tstState = 2;
                            }
                            break;
                        case 2: // Wait for next packet timer to expire
                            if( TxNextPacket == true )
                            {
                                TxNextPacket = false;
                                tstState = 0;
                            }
                            break;
                    }
                    
                    TimerLowPowerHandler( );
                }
            }
        }
        
        TimerStart( &StopTimer );
        while( 1 ) // Reset device to restart
        {
            if( StopTimerEvent == true )
            {
                StopTimerEvent = false;
                TimerStart( &StopTimer );
            }
            TimerLowPowerHandler( );
        }
    }
}

