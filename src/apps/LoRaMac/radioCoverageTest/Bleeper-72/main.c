/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Radio coverage tester implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <string.h>
#include <math.h>
#include "board.h"
#include "radio.h"

#include "LoRaMac.h"

/*!
 * Number of packets sent by channel
 */
#define NB_PACKETS                                  15

/*!
 * Enables/Disables the downlink test
 */
#define DOWNLINK_TEST_ON                            0

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
#define APP_DATA_SIZE                               6

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

static bool AppLedStateOn = false;

static LoRaMacEvent_t LoRaMacEvents;
static LoRaMacEventInfo_t LoRaMacLastRxEvent;

static TimerEvent_t Led1Timer;
static TimerEvent_t Led2Timer;
static TimerEvent_t StopTimer;

static uint8_t ChannelNb;
static uint16_t DownLinkCounter = 0;

#if( OVER_THE_AIR_ACTIVATION != 0 )

/*!
 * \brief Function executed on JoinReq Timeout event
 */
static void OnJoinReqTimerEvent( void )
{
    TxNextPacket = true;
}

#endif

/*!
 * \brief Function executed on TxNextPacket Timeout event
 */
static void OnTxNextPacketTimerEvent( void )
{
    TxNextPacket = true;
}

/*!
 * \brief Function executed on Led 1 Timeout event
 */
static void OnLed1TimerEvent( void )
{
    // Switch LED 1 OFF
    GpioWrite( &Led1, 1 );
}

/*!
 * \brief Function executed on Led 2 Timeout event
 */
static void OnLed2TimerEvent( void )
{
    // Switch LED 2 OFF
    GpioWrite( &Led2, 1 );
}

/*!
 * \brief Function executed on Stop Timeout event
 */
static void OnStopTimerEvent( void )
{
    TimerStart( &StopTimer );
}

/*!
 * \brief Function to be executed on MAC layer event
 */
static void OnMacEvent( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info )
{
    int32_t snr = 0;

    LoRaMacLastRxEvent = *info;
    
    if( flags->Bits.JoinAccept == 1 )
    {
#if( OVER_THE_AIR_ACTIVATION != 0 )
        // Once joined disable reception windows opening
        LoRaMacTestRxWindowsOn( false );

        TimerStop( &JoinReqTimer );
#endif
        IsNetworkJoined = true;
    }
    else
    {
        // Schedule a new transmission
        TxDone = true;
    }
    
    if( flags->Bits.Tx == 1 )
    {
    }

    if( flags->Bits.Rx == 1 )
    {
        if( (flags->Bits.RxData == true ) && ( ( info->RxPort == 1 ) || ( info->RxPort == 2 ) ) )
        {
            AppLedStateOn = info->RxBuffer[0];
        
            if( AppLedStateOn == true )
            {
                // Switch LED 3 ON
                GpioWrite( &Led3, 0 );
            }
            else
            {
                // Switch LED 3 OFF
                GpioWrite( &Led3, 1 );
            }
        }

        if( info->RxSnr & 0x80 ) // The SNR sign bit is 1
        {
            // Invert and divide by 4
            snr = ( ( ~info->RxSnr + 1 ) & 0xFF ) >> 2;
            snr = -snr;
        }
        else
        {
            // Divide by 4
            snr = ( info->RxSnr & 0xFF ) >> 2;
        }
        DownLinkCounter++;

        // Switch LED 2 ON for each received downlink
        GpioWrite( &Led2, 0 );
        TimerStart( &Led2Timer );
    }
}

/**
 * Main application entry point.
 */
int main( void )
{
    const ChannelParams_t channels[] = 
    { 
        { 868100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 1 },
        { 868300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 1 },
        { 868500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 1 },
        { 867100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 1 },
        { 867300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 1 },
        { 867500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 1 },
        { 867700000, { ( ( DR_5 << 4 ) | DR_0 ) }, 1 },
        { 867900000, { ( ( DR_5 << 4 ) | DR_0 ) }, 1 },
     };
    const uint8_t  channelsDatarate[] = { DR_5, DR_2, DR_0 };

    uint8_t channelNb = ( sizeof( channels ) / sizeof( ChannelParams_t ) );

#if( OVER_THE_AIR_ACTIVATION != 0 )
    uint8_t sendFrameStatus = 0;
#endif
    uint8_t tstState = 0;
    int16_t pktCnt = NB_PACKETS;
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

    TimerInit( &Led2Timer, OnLed2TimerEvent );
    TimerSetValue( &Led2Timer, 25000 );
    
    // Low power timer to be run when tests are finished.
    TimerInit( &StopTimer, OnStopTimerEvent );
    TimerSetValue( &StopTimer, 3.6e9 ); // wakes up the microcontroller every hour

    DownLinkCounter = 0;

    // Initialize MAC frame
    macHdr.Value = 0;
#if ( DOWNLINK_TEST_ON == 1 )
    macHdr.Bits.MType = FRAME_TYPE_DATA_CONFIRMED_UP;
#else
    // Disable reception windows opening
    LoRaMacTestRxWindowsOn( false );
    macHdr.Bits.MType = FRAME_TYPE_DATA_UNCONFIRMED_UP;
#endif
    fCtrl.Value = 0;
    fCtrl.Bits.FOptsLen      = 0;
    fCtrl.Bits.FPending      = 0;
    fCtrl.Bits.Ack           = false;
    fCtrl.Bits.AdrAckReq     = false;
    fCtrl.Bits.Adr           = false;

    LoRaMacSetChannelsTxPower( TX_POWER_14_DBM );
    
    while( 1 )
    {
        while( IsNetworkJoined == false )
        {
#if( OVER_THE_AIR_ACTIVATION != 0 )
            if( TxNextPacket == true )
            {
                TxNextPacket = false;
                
                sendFrameStatus = LoRaMacJoinReq( DevEui, AppEui, AppKey );
                switch( sendFrameStatus )
                {
                case 1: // BUSY
                    break;
                case 0: // OK
                case 2: // NO_NETWORK_JOINED
                case 3: // LENGTH_PORT_ERROR
                case 4: // MAC_CMD_ERROR
                case 6: // DEVICE_OFF
                default:
                    // Relaunch timer for next trial
                    TimerStart( &JoinReqTimer );
                    break;
                }
            }
            TimerLowPowerHandler( );
#endif
        }
        for( datarateIndex = 0; datarateIndex < 3; datarateIndex++ )
        {
            pktCnt = NB_PACKETS * channelNb;
            while( pktCnt > 0 )
            {
                switch( tstState )
                {
                    case 0: // Init
                        AppData[0] = SelectorGetValue( );
                        AppData[1] = LoRaMacLastRxEvent.RxRssi >> 8;
                        AppData[2] = LoRaMacLastRxEvent.RxRssi & 0xFF;
                        AppData[3] = LoRaMacLastRxEvent.RxSnr;
                        AppData[4] = ( ( DownLinkCounter >> 8 ) & 0xFF );
                        AppData[5] = ( ( DownLinkCounter & 0xFF ) );
                        
                        LoRaMacSetChannelsDatarate( channelsDatarate[datarateIndex] );
                        LoRaMacSendOnChannel( channels[channelsIndex], &macHdr, &fCtrl, NULL, 15, AppData, APP_DATA_SIZE );
                        
                        // Switch LED 1 ON
                        GpioWrite( &Led1, 0 );
                        TimerStart( &Led1Timer );

                        channelsIndex = ( channelsIndex + 1 ) % ChannelNb;

                        tstState = 1;
                        break;
                    case 1: // Wait for end of transmission
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

        // Switch LED OFF
        GpioWrite( &Led1, 1 );
        GpioWrite( &Led2, 1 );
        GpioWrite( &Led3, 1 );
    
        TimerStart( &StopTimer );
        while( 1 ) // Reset device to restart
        {
            TimerLowPowerHandler( );
        }
    }
}
