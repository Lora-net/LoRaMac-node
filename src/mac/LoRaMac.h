/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: LoRa MAC layer implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __LORAMAC_H__
#define __LORAMAC_H__

// Includes board dependent definitions such as channels frequencies
#include "LoRaMac-board.h"

#if defined(__CC_ARM) || defined(__GNUC__)
#define PACKED                                      __attribute__( ( __packed__ ) )
#elif defined( __ICCARM__ )
#define PACKED                                      __packed
#else
    #warning Not supported compiler type
#endif
/*!
 * Beacon interval in ms
 */
#define BEACON_INTERVAL                             128000000

/*!
 * Class A&B receive delay in ms
 */
#define RECEIVE_DELAY1                              1000000
#define RECEIVE_DELAY2                              2000000

/*!
 * Join accept receive delay in ms
 */
#define JOIN_ACCEPT_DELAY1                          5000000
#define JOIN_ACCEPT_DELAY2                          6000000

/*!
 * Class A&B maximum receive window delay in ms
 */
#define MAX_RX_WINDOW                               3000000

/*!
 * Maximum allowed gap for the FCNT field
 */
#define MAX_FCNT_GAP                                16384

/*!
 * ADR acknowledgement counter limit
 */
#define ADR_ACK_LIMIT                               64

/*!
 * Number of ADR acknowledgement requests before returning to default datarate
 */
#define ADR_ACK_DELAY                               32

/*!
 * Number of seconds after the start of the second reception window without
 * receiving an acknowledge.
 * AckTimeout = ACK_TIMEOUT + Random( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND )
 */
#define ACK_TIMEOUT                                 2000000

/*!
 * Random number of seconds after the start of the second reception window without
 * receiving an acknowledge
 * AckTimeout = ACK_TIMEOUT + Random( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND )
 */
#define ACK_TIMEOUT_RND                             1000000

/*!
 * Check the Mac layer state every MAC_STATE_CHECK_TIMEOUT
 */
#define MAC_STATE_CHECK_TIMEOUT                     1000000

/*!
 * Maximum number of times the MAC layer tries to get an acknowledge.
 */
#define MAX_ACK_RETRIES                             8

/*!
 * RSSI free threshold
 */
#define RSSI_FREE_TH                                ( int8_t )( -90 ) // [dBm]

/*! 
 * Frame direction definition
 */
#define UP_LINK                                     0
#define DOWN_LINK                                   1

/*!
 * Sets the length of the LoRaMAC footer field.
 * Mainly indicates the MIC field length
 */
#define LORAMAC_MFR_LEN                             4

/*!
 * Syncword for Private LoRa networks
 */
#define LORA_MAC_PRIVATE_SYNCWORD                   0x12

/*!
 * Syncword for Public LoRa networks
 */
#define LORA_MAC_PUBLIC_SYNCWORD                    0x34

/*!
 * LoRaWAN devices classes definition
 */
typedef enum
{
    CLASS_A,
    CLASS_B,
    CLASS_C,
}DeviceClass_t;

/*!
 * LoRaMAC channels parameters definition
 */
typedef union
{
    int8_t Value;
    struct
    {
        int8_t Min : 4;
        int8_t Max : 4;
    }PACKED Fields;
}PACKED DrRange_t;

typedef struct
{
    uint16_t DCycle;
    int8_t TxMaxPower;
    uint64_t LastTxDoneTime;
    uint64_t TimeOff;
}PACKED Band_t;

typedef struct
{
    uint32_t Frequency; // Hz
    DrRange_t DrRange;  // Max datarate [0: SF12, 1: SF11, 2: SF10, 3: SF9, 4: SF8, 5: SF7, 6: SF7, 7: FSK]
                        // Min datarate [0: SF12, 1: SF11, 2: SF10, 3: SF9, 4: SF8, 5: SF7, 6: SF7, 7: FSK]
    uint8_t Band;       // Band index
}PACKED ChannelParams_t;

typedef struct
{
    uint32_t Frequency; // Hz
    uint8_t  Datarate;  // [0: SF12, 1: SF11, 2: SF10, 3: SF9, 4: SF8, 5: SF7, 6: SF7, 7: FSK]
}PACKED Rx2ChannelParams_t;

typedef struct MulticastParams_s
{
    uint32_t Address;
    uint8_t NwkSKey[16];
    uint8_t AppSKey[16];
    uint32_t DownLinkCounter;
    struct MulticastParams_s *Next;
}PACKED MulticastParams_t;

/*!
 * LoRaMAC frame types
 */
typedef enum
{
    FRAME_TYPE_JOIN_REQ              = 0x00,
    FRAME_TYPE_JOIN_ACCEPT           = 0x01,
    FRAME_TYPE_DATA_UNCONFIRMED_UP   = 0x02,
    FRAME_TYPE_DATA_UNCONFIRMED_DOWN = 0x03,
    FRAME_TYPE_DATA_CONFIRMED_UP     = 0x04,
    FRAME_TYPE_DATA_CONFIRMED_DOWN   = 0x05,
    FRAME_TYPE_RFU                   = 0x06,
    FRAME_TYPE_PROPRIETARY           = 0x07,
}PACKED LoRaMacFrameType_t;

/*!
 * LoRaMAC mote MAC commands
 */
typedef enum
{
    MOTE_MAC_LINK_CHECK_REQ          = 0x02,
    MOTE_MAC_LINK_ADR_ANS            = 0x03,
    MOTE_MAC_DUTY_CYCLE_ANS          = 0x04,
    MOTE_MAC_RX_PARAM_SETUP_ANS      = 0x05,
    MOTE_MAC_DEV_STATUS_ANS          = 0x06,
    MOTE_MAC_NEW_CHANNEL_ANS         = 0x07,
    MOTE_MAC_RX_TIMING_SETUP_ANS     = 0x08,
}PACKED LoRaMacMoteCmd_t;

/*!
 * LoRaMAC server MAC commands
 */
typedef enum
{
    SRV_MAC_LINK_CHECK_ANS           = 0x02,
    SRV_MAC_LINK_ADR_REQ             = 0x03,
    SRV_MAC_DUTY_CYCLE_REQ           = 0x04,
    SRV_MAC_RX_PARAM_SETUP_REQ       = 0x05,
    SRV_MAC_DEV_STATUS_REQ           = 0x06,
    SRV_MAC_NEW_CHANNEL_REQ          = 0x07,
    SRV_MAC_RX_TIMING_SETUP_REQ      = 0x08,
}PACKED LoRaMacSrvCmd_t;

/*!
 * LoRaMAC Battery level indicator
 */
typedef enum
{
    BAT_LEVEL_EXT_SRC                = 0x00,
    BAT_LEVEL_EMPTY                  = 0x01,
    BAT_LEVEL_FULL                   = 0xFE,
    BAT_LEVEL_NO_MEASURE             = 0xFF,
}PACKED LoRaMacBatteryLevel_t;

/*!
 * LoRaMAC header field definition
 */
typedef union
{
    uint8_t Value;
    struct
    {
        uint8_t Major           : 2;
        uint8_t RFU             : 3;
        uint8_t MType           : 3;
    }PACKED Bits;
}PACKED LoRaMacHeader_t;

/*!
 * LoRaMAC frame header field definition
 */
typedef union
{
    uint8_t Value;
    struct
    {
        uint8_t FOptsLen        : 4;
        uint8_t FPending        : 1;
        uint8_t Ack             : 1;
        uint8_t AdrAckReq       : 1;
        uint8_t Adr             : 1;
    }PACKED Bits;
}PACKED LoRaMacFrameCtrl_t;

/*!
 * LoRaMAC event flags
 */
typedef union
{
    uint8_t Value;
    struct
    {
        uint8_t Tx              : 1;
        uint8_t Rx              : 1;
        uint8_t RxData          : 1;
        uint8_t Multicast       : 1;
        uint8_t RxSlot          : 2;
        uint8_t LinkCheck       : 1;
        uint8_t JoinAccept      : 1;
    }PACKED Bits;
}PACKED LoRaMacEventFlags_t;

typedef enum
{
    LORAMAC_EVENT_INFO_STATUS_OK = 0,
    LORAMAC_EVENT_INFO_STATUS_ERROR,
    LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT,
    LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT,
    LORAMAC_EVENT_INFO_STATUS_RX2_ERROR,
    LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL,
    LORAMAC_EVENT_INFO_STATUS_DOWNLINK_FAIL,
    LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL,
    LORAMAC_EVENT_INFO_STATUS_MIC_FAIL,
}PACKED LoRaMacEventInfoStatus_t;

/*!
 * LoRaMAC event information
 */
typedef struct
{
    LoRaMacEventInfoStatus_t Status;
    bool TxAckReceived;
    uint8_t TxNbRetries;
    uint8_t TxDatarate;
    uint8_t RxPort;
    uint8_t *RxBuffer;
    uint8_t RxBufferSize;
    int16_t RxRssi;
    uint8_t RxSnr;
    uint16_t Energy;
    uint8_t DemodMargin;
    uint8_t NbGateways;
}PACKED LoRaMacEventInfo_t;

/*!
 * LoRaMAC events structure
 * Used to notify upper layers of MAC events
 */
typedef struct sLoRaMacEvent
{
    /*!
     * MAC layer event callback prototype.
     *
     * \param [IN] flags Bit field indicating the MAC events occurred
     * \param [IN] info  Details about MAC events occurred
     */
    void ( *MacEvent )( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info );
}PACKED LoRaMacEvent_t;

/*!
 * LoRaMAC layer initialization
 *
 * \param [IN] events        Pointer to a structure defining the LoRaMAC
 *                           callback functions.
 */
void LoRaMacInit( LoRaMacEvent_t *events );

/*!
 * Enables/Disables the ADR (Adaptive Data Rate)
 * 
 * \param [IN] enable [true: ADR ON, false: ADR OFF]
 */
void LoRaMacSetAdrOn( bool enable );

/*!
 * Initializes the network IDs. Device address, 
 * network session AES128 key and application session AES128 key.
 *
 * \remark To be only used when Over-the-Air activation isn't used.
 *
 * \param [IN] netID   24 bits network identifier 
 *                     ( provided by network operator )
 * \param [IN] devAddr 32 bits device address on the network 
 *                     (must be unique to the network)
 * \param [IN] nwkSKey Pointer to the network session AES128 key array
 *                     ( 16 bytes )
 * \param [IN] appSKey Pointer to the application session AES128 key array
 *                     ( 16 bytes )
 */
void LoRaMacInitNwkIds( uint32_t netID, uint32_t devAddr, uint8_t *nwkSKey, uint8_t *appSKey );

/*
 * TODO: Add documentation
 */
void LoRaMacMulticastChannelAdd( MulticastParams_t *channelParam );

/*
 * TODO: Add documentation
 */
void LoRaMacMulticastChannelRemove( MulticastParams_t *channelParam );

/*!
 * Initiates the Over-the-Air activation 
 * 
 * \param [IN] devEui Pointer to the device EUI array ( 8 bytes )
 * \param [IN] appEui Pointer to the application EUI array ( 8 bytes )
 * \param [IN] appKey Pointer to the application AES128 key array ( 16 bytes )
 *
 * \retval status [0: OK, 1: Tx error, 2: Already joined a network]
 */
uint8_t LoRaMacJoinReq( uint8_t *devEui, uint8_t *appEui, uint8_t *appKey );

/*!
 * Sends a LinkCheckReq MAC command on the next uplink frame
 *
 * \retval status Function status [0: OK, 1: Busy]
 */
uint8_t LoRaMacLinkCheckReq( void );

/*!
 * LoRaMAC layer send frame
 *
 * \param [IN] fPort       MAC payload port (must be > 0)
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 *
 * \retval status          [0: OK, 1: Busy, 2: No network joined,
 *                          3: Length or port error, 4: Unknown MAC command
 *                          5: Unable to find a free channel
 *                          6: Device switched off]
 */
uint8_t LoRaMacSendFrame( uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*!
 * LoRaMAC layer send frame
 *
 * \param [IN] fPort       MAC payload port (must be > 0)
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \param [IN] fBufferSize MAC data buffer size
 * \param [IN] nbRetries   Number of retries to receive the acknowledgement
 *
 * \retval status          [0: OK, 1: Busy, 2: No network joined,
 *                          3: Length or port error, 4: Unknown MAC command
 *                          5: Unable to find a free channel
 *                          6: Device switched off]
 */
uint8_t LoRaMacSendConfirmedFrame( uint8_t fPort, void *fBuffer, uint16_t fBufferSize, uint8_t nbRetries );

/*!
 * ============================================================================
 * = LoRaMac test functions                                                   =
 * ============================================================================
 */

/*!
 * LoRaMAC layer generic send frame
 *
 * \param [IN] macHdr      MAC header field
 * \param [IN] fOpts       MAC commands buffer
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          [0: OK, 1: Busy, 2: No network joined,
 *                          3: Length or port error, 4: Unknown MAC command
 *                          5: Unable to find a free channel
 *                          6: Device switched off]
 */
uint8_t LoRaMacSend( LoRaMacHeader_t *macHdr, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*!
 * LoRaMAC layer frame buffer initialization.
 *
 * \param [IN] channel     Channel parameters
 * \param [IN] macHdr      MAC header field
 * \param [IN] fCtrl       MAC frame control field
 * \param [IN] fOpts       MAC commands buffer
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          [0: OK, 1: N/A, 2: No network joined,
 *                          3: Length or port error, 4: Unknown MAC command]
 */
uint8_t LoRaMacPrepareFrame( ChannelParams_t channel,LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*!
 * LoRaMAC layer prepared frame buffer transmission with channel specification
 *
 * \remark LoRaMacPrepareFrame must be called at least once before calling this
 *         function.
 *
 * \param [IN] channel     Channel parameters
 * \retval status          [0: OK, 1: Busy]
 */
uint8_t LoRaMacSendFrameOnChannel( ChannelParams_t channel );

/*!
 * LoRaMAC layer generic send frame with channel specification
 *
 * \param [IN] channel     Channel parameters
 * \param [IN] macHdr      MAC header field
 * \param [IN] fCtrl       MAC frame control field
 * \param [IN] fOpts       MAC commands buffer
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          [0: OK, 1: Busy, 2: No network joined,
 *                          3: Length or port error, 4: Unknown MAC command]
 */
uint8_t LoRaMacSendOnChannel( ChannelParams_t channel, LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t *fOpts, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*
 * TODO: Add documentation
 */
void LoRaMacSetDeviceClass( DeviceClass_t deviceClass );

/*
 * TODO: Add documentation
 */
void LoRaMacSetPublicNetwork( bool enable );

/*
 * TODO: Add documentation
 */
void LoRaMacSetDutyCycleOn( bool enable );

/*
 * TODO: Add documentation
 */
void LoRaMacSetChannel( uint8_t id, ChannelParams_t params );

/*
 * TODO: Add documentation
 */
void LoRaMacSetRx2Channel( Rx2ChannelParams_t param );

/*
 * TODO: Add documentation
 */
void LoRaMacSetChannelsMask( uint16_t mask );

/*
 * TODO: Add documentation
 */
void LoRaMacSetChannelsNbRep( uint8_t nbRep );

/*
 * TODO: Add documentation
 */
void LoRaMacSetMaxRxWindow( uint32_t delay );

/*
 * TODO: Add documentation
 */
void LoRaMacSetReceiveDelay1( uint32_t delay );

/*
 * TODO: Add documentation
 */
void LoRaMacSetReceiveDelay2( uint32_t delay );

/*
 * TODO: Add documentation
 */
void LoRaMacSetJoinAcceptDelay1( uint32_t delay );

/*
 * TODO: Add documentation
 */
void LoRaMacSetJoinAcceptDelay2( uint32_t delay );

/*!
 * Sets channels datarate
 *
 * \param [IN] datarate eu868 - [DR_0, DR_1, DR_2, DR_3, DR_4, DR_5, DR_6, DR_7]
 */
void LoRaMacSetChannelsDatarate( int8_t datarate );

/*!
 * Sets channels tx output power
 *
 * \param [IN] txPower [TX_POWER_20_DBM, TX_POWER_14_DBM,
                        TX_POWER_11_DBM, TX_POWER_08_DBM,
                        TX_POWER_05_DBM, TX_POWER_02_DBM]
 */
void LoRaMacSetChannelsTxPower( int8_t txPower );

/*!
 * Disables/Enables the reception windows opening
 *
 * \param [IN] enable [true: enable, false: disable]
 */
void LoRaMacTestRxWindowsOn( bool enable );

/*
 * TODO: Add documentation
 */
uint32_t LoRaMacGetUpLinkCounter( void );

/*
 * TODO: Add documentation
 */
uint32_t LoRaMacGetDownLinkCounter( void );

/*!
 * Enables the MIC field test
 *
 * \param [IN] txPacketCounter Fixed Tx packet counter value
 */
void LoRaMacSetMicTest( uint16_t txPacketCounter );

#endif // __LORAMAC_H__
