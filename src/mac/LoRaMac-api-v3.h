/*!
 * \file      LoRaMac-api-v3.h
 *
 * \brief     LoRa MAC wrapper layer implementation
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
 *              (C)2013 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jäckle ( STACKFORCE )
 */
#ifndef __LORAMAC_API_V3_H__
#define __LORAMAC_API_V3_H__

// Includes board dependent definitions such as channels frequencies
#include "LoRaMac.h"
#include "LoRaMac-board.h"

/*!
 * Beacon interval in ms
 */
#define BEACON_INTERVAL                             128000

/*!
 * Class A&B receive delay 1 in ms
 */
#define RECEIVE_DELAY1                              1000

/*!
 * Class A&B receive delay 2 in ms
 */
#define RECEIVE_DELAY2                              2000

/*!
 * Join accept receive delay 1 in ms
 */
#define JOIN_ACCEPT_DELAY1                          5000

/*!
 * Join accept receive delay 2 in ms
 */
#define JOIN_ACCEPT_DELAY2                          6000

/*!
 * Class A&B maximum receive window delay in ms
 */
#define MAX_RX_WINDOW                               3000

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
 * AckTimeout = \ref ACK_TIMEOUT + Random( -\ref ACK_TIMEOUT_RND, \ref ACK_TIMEOUT_RND )
 */
#define ACK_TIMEOUT                                 2000

/*!
 * Random number of seconds after the start of the second reception window without
 * receiving an acknowledge
 * AckTimeout = \ref ACK_TIMEOUT + Random( -\ref ACK_TIMEOUT_RND, \ref ACK_TIMEOUT_RND )
 */
#define ACK_TIMEOUT_RND                             1000

/*!
 * Check the Mac layer state every MAC_STATE_CHECK_TIMEOUT in ms
 */
#define MAC_STATE_CHECK_TIMEOUT                     1000

/*!
 * Maximum number of times the MAC layer tries to get an acknowledge.
 */
#define MAX_ACK_RETRIES                             8

/*!
 * RSSI free threshold [dBm]
 */
#define RSSI_FREE_TH                                ( int8_t )( -90 )

/*!
 * Frame direction definition for up-link communications
 */
#define UP_LINK                                     0

/*!
 * Frame direction definition for down-link communications
 */
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
    }Bits;
}LoRaMacEventFlags_t;

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
}LoRaMacEventInfo_t;

/*!
 * LoRaMAC events structure
 * Used to notify upper layers of MAC events
 */
typedef struct sLoRaMacCallbacks
{
    /*!
     * MAC layer event callback prototype.
     *
     * \param [IN] flags Bit field indicating the MAC events occurred
     * \param [IN] info  Details about MAC events occurred
     */
    void ( *MacEvent )( LoRaMacEventFlags_t *flags, LoRaMacEventInfo_t *info );
    /*!
     * Function callback to get the current battery level
     *
     * \retval batteryLevel Current battery level
     */
    uint8_t ( *GetBatteryLevel )( void );
}LoRaMacCallbacks_t;

/*!
 * LoRaMAC layer initialization
 *
 * \param [IN] callbacks     Pointer to a structure defining the LoRaMAC
 *                           callback functions.
 */
void LoRaMacInit( LoRaMacCallbacks_t *callbacks );

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
 * Wrapper function which calls \ref LoRaMacMulticastChannelLink.
 */
void LoRaMacMulticastChannelAdd( MulticastParams_t *channelParam );

/*
 * Wrapper function which calls \ref LoRaMacMulticastChannelUnlink.
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

/*!
 * ============================================================================
 * = LoRaMac setup functions                                                  =
 * ============================================================================
 */

/*
 * Wrapper function which calls \ref LoRaMacMibSetRequestConfirm to
 * set the LoRaWan device class.
 */
void LoRaMacSetDeviceClass( DeviceClass_t deviceClass );

/*
 * Wrapper function which calls \ref LoRaMacMibSetRequestConfirm to
 * set the network type to public or private.
 */
void LoRaMacSetPublicNetwork( bool enable );

/*
 * Wrapper function which calls \ref LoRaMacChannelAdd.
 */
void LoRaMacSetChannel( uint8_t id, ChannelParams_t params );

/*
 * Wrapper function which calls \ref LoRaMacMibSetRequestConfirm to
 * set the receive window 2 channel.
 */
void LoRaMacSetRx2Channel( Rx2ChannelParams_t param );

/*!
 * Sets channels tx output power
 *
 * \param [IN] txPower [TX_POWER_20_DBM, TX_POWER_14_DBM,
                        TX_POWER_11_DBM, TX_POWER_08_DBM,
                        TX_POWER_05_DBM, TX_POWER_02_DBM]
 */
void LoRaMacSetChannelsTxPower( int8_t txPower );

/*!
 * Sets channels datarate
 *
 * \param [IN] datarate eu868 - [DR_0, DR_1, DR_2, DR_3, DR_4, DR_5, DR_6, DR_7]
 *                      us915 - [DR_0, DR_1, DR_2, DR_3, DR_4]
 */
void LoRaMacSetChannelsDatarate( int8_t datarate );

/*
 * Wrapper function which calls \ref LoRaMacMibSetRequestConfirm to
 * set the channels mask.
 */
void LoRaMacSetChannelsMask( uint16_t *mask );

/*
 * Wrapper function which calls \ref LoRaMacMibSetRequestConfirm to
 * set the number of repetitions on a channel.
 */
void LoRaMacSetChannelsNbRep( uint8_t nbRep );

/*
 * Wrapper function which calls \ref LoRaMacMibSetRequestConfirm to
 * set the maximum receive window duration in [ms].
 */
void LoRaMacSetMaxRxWindow( uint32_t delay );

/*
 * Wrapper function which calls \ref LoRaMacMibSetRequestConfirm to
 * set the receive delay 1 in [ms].
 */
void LoRaMacSetReceiveDelay1( uint32_t delay );

/*
 * Wrapper function which calls \ref LoRaMacMibSetRequestConfirm to
 * set the receive delay 2 in [ms].
 */
void LoRaMacSetReceiveDelay2( uint32_t delay );

/*
 * Wrapper function which calls \ref LoRaMacMibSetRequestConfirm to
 * set the join accept delay 1 in [ms].
 */
void LoRaMacSetJoinAcceptDelay1( uint32_t delay );

/*
 * Wrapper function which calls \ref LoRaMacMibSetRequestConfirm to
 * set the join accept delay 2 in [ms].
 */
void LoRaMacSetJoinAcceptDelay2( uint32_t delay );

/*
 * Wrapper function which calls \ref LoRaMacMibGetRequestConfirm to
 * get the up-link counter.
 */
uint32_t LoRaMacGetUpLinkCounter( void );

/*
 * Wrapper function which calls \ref LoRaMacMibGetRequestConfirm to
 * get the down-link counter.
 */
uint32_t LoRaMacGetDownLinkCounter( void );

/*
 * ============================================================================
 * = LoRaMac test functions                                                   =
 * ============================================================================
 */

/*!
 * Disables/Enables the duty cycle enforcement (EU868)
 *
 * \param   [IN] enable - Enabled or disables the duty cycle
 */
void LoRaMacTestSetDutyCycleOn( bool enable );

/*!
 * Disables/Enables the reception windows opening
 *
 * \param [IN] enable [true: enable, false: disable]
 */
void LoRaMacTestRxWindowsOn( bool enable );

/*!
 * Enables the MIC field test
 *
 * \param [IN] upLinkCounter Fixed Tx packet counter value
 */
void LoRaMacTestSetMic( uint16_t upLinkCounter );

#endif /* __LORAMAC_API_V3_H__ */
