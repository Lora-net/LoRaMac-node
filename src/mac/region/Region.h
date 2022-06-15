/*!
 * \file      Region.h
 *
 * \brief     Region implementation.
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
 *              (C)2013-2017 Semtech
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
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \defgroup  REGION Region implementation
 *            This is the common API to access the specific
 *            regional implementations.
 *
 *            Preprocessor options:
 *            - LoRaWAN regions can be activated by defining the related preprocessor
 *              definition. It is possible to define more than one region.
 *              The following regions are supported:
 *              - #define REGION_AS923
 *              - #define REGION_AU915
 *              - #define REGION_CN470
 *              - #define REGION_CN779
 *              - #define REGION_EU433
 *              - #define REGION_EU868
 *              - #define REGION_KR920
 *              - #define REGION_IN865
 *              - #define REGION_US915
 *              - #define REGION_RU864
 *
 * \{
 */
#ifndef __REGION_H__
#define __REGION_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "utilities.h"
#include "LoRaMac.h"
#include "timer.h"
#include "RegionCommon.h"

/*!
 * Macro to compute bit of a channel index.
 */
#define LC( channelIndex )                          ( uint16_t )( 1 << ( channelIndex - 1 ) )

#ifndef REGION_VERSION
/*!
 * Regional parameters version definition.
 * RP002-1.0.3
 */
#define REGION_VERSION                              0x02010003
#endif



/*!
 * Enumeration of phy attributes.
 */
typedef enum ePhyAttribute
{
    /*!
     * Frequency. It is available
     * to perform a verification with RegionVerify().
     */
    PHY_FREQUENCY,
    /*!
     * Minimum RX datarate.
     */
    PHY_MIN_RX_DR,
    /*!
     * Minimum TX datarate.
     */
    PHY_MIN_TX_DR,
    /*!
     * Maximum RX datarate.
     */
    PHY_MAX_RX_DR,
    /*!
     * Maximum TX datarate.
     */
    PHY_MAX_TX_DR,
    /*!
     * TX datarate.
     * This is a parameter which can't be queried. It is available
     * to perform a verification with RegionVerify().
     */
    PHY_TX_DR,
    /*!
     * Default TX datarate.
     */
    PHY_DEF_TX_DR,
    /*!
     * RX datarate. It is available
     * to perform a verification with RegionVerify().
     */
    PHY_RX_DR,
    /*!
     * Maximum TX power.
     */
    PHY_MAX_TX_POWER,
    /*!
     * TX power. It is available
     * to perform a verification with RegionVerify().
     */
    PHY_TX_POWER,
    /*!
     * Default TX power.
     */
    PHY_DEF_TX_POWER,
    /*!
     * Default ADR_ACK_LIMIT value.
     */
    PHY_DEF_ADR_ACK_LIMIT,
    /*!
     * Default ADR_ACK_DELAY value.
     */
    PHY_DEF_ADR_ACK_DELAY,
    /*!
     * Maximum payload possible.
     */
    PHY_MAX_PAYLOAD,
    /*!
     * Duty cycle.
     */
    PHY_DUTY_CYCLE,
    /*!
     * Maximum receive window duration.
     */
    PHY_MAX_RX_WINDOW,
    /*!
     * Receive delay for window 1.
     */
    PHY_RECEIVE_DELAY1,
    /*!
     * Receive delay for window 2.
     */
    PHY_RECEIVE_DELAY2,
    /*!
     * Join accept delay for window 1.
     */
    PHY_JOIN_ACCEPT_DELAY1,
    /*!
     * Join accept delay for window 2.
     */
    PHY_JOIN_ACCEPT_DELAY2,
    /*!
     * Acknowledgement time out.
     */
    PHY_RETRANSMIT_TIMEOUT,
    /*!
     * Default datarate offset for window 1.
     */
    PHY_DEF_DR1_OFFSET,
    /*!
     * Default receive window 2 frequency.
     */
    PHY_DEF_RX2_FREQUENCY,
    /*!
     * Default receive window 2 datarate.
     */
    PHY_DEF_RX2_DR,
    /*!
     * Channels mask.
     */
    PHY_CHANNELS_MASK,
    /*!
     * Channels default mask.
     */
    PHY_CHANNELS_DEFAULT_MASK,
    /*!
     * Maximum number of supported channels
     */
    PHY_MAX_NB_CHANNELS,
    /*!
     * Channels.
     */
    PHY_CHANNELS,
    /*!
     * Default value of the uplink dwell time.
     */
    PHY_DEF_UPLINK_DWELL_TIME,
    /*!
     * Default value of the downlink dwell time.
     */
    PHY_DEF_DOWNLINK_DWELL_TIME,
    /*!
     * Default value of the MaxEIRP.
     */
    PHY_DEF_MAX_EIRP,
    /*!
     * Default value of the antenna gain.
     */
    PHY_DEF_ANTENNA_GAIN,
    /*!
     * Next lower datarate.
     */
    PHY_NEXT_LOWER_TX_DR,
    /*!
     * Beacon interval in ms.
     */
    PHY_BEACON_INTERVAL,
    /*!
     * Beacon reserved time in ms.
     */
    PHY_BEACON_RESERVED,
    /*!
     * Beacon guard time in ms.
     */
    PHY_BEACON_GUARD,
    /*!
     * Beacon window time in ms.
     */
    PHY_BEACON_WINDOW,
    /*!
     * Beacon window time in numer of slots.
     */
    PHY_BEACON_WINDOW_SLOTS,
    /*!
     * Ping slot length time in ms.
     */
    PHY_PING_SLOT_WINDOW,
    /*!
     * Default symbol timeout for beacons and ping slot windows.
     */
    PHY_BEACON_SYMBOL_TO_DEFAULT,
    /*!
     * Maximum symbol timeout for beacons.
     */
    PHY_BEACON_SYMBOL_TO_EXPANSION_MAX,
    /*!
     * Maximum symbol timeout for ping slots.
     */
    PHY_PING_SLOT_SYMBOL_TO_EXPANSION_MAX,
    /*!
     * Symbol expansion value for beacon windows in case of beacon
     * loss in symbols.
     */
    PHY_BEACON_SYMBOL_TO_EXPANSION_FACTOR,
    /*!
     * Symbol expansion value for ping slot windows in case of beacon
     * loss in symbols.
     */
    PHY_PING_SLOT_SYMBOL_TO_EXPANSION_FACTOR,
    /*!
     * Maximum allowed beacon less time in ms.
     */
    PHY_MAX_BEACON_LESS_PERIOD,
    /*!
     * Delay time for the BeaconTimingAns in ms.
     */
    PHY_BEACON_DELAY_BEACON_TIMING_ANS,
    /*!
     * Beacon channel frequency.
     */
    PHY_BEACON_CHANNEL_FREQ,
    /*!
     * The format of the beacon.
     */
    PHY_BEACON_FORMAT,
    /*!
     * The beacon channel datarate.
     */
    PHY_BEACON_CHANNEL_DR,
    /*!
     * The number of channels for the beacon reception.
     */
    PHY_BEACON_NB_CHANNELS,
    /*!
     * The static offset for the downlink channel calculation.
     */
    PHY_BEACON_CHANNEL_OFFSET,
    /*!
     * Ping slot channel frequency.
     */
    PHY_PING_SLOT_CHANNEL_FREQ,
    /*!
     * The datarate of a ping slot channel.
     */
    PHY_PING_SLOT_CHANNEL_DR,
    /*!
     * The number of channels for the ping slot reception.
     */
    PHY_PING_SLOT_NB_CHANNELS,
    /*!
     * The equivalent spreading factor value from datarate
     */
    PHY_SF_FROM_DR,
    /*!
     * The equivalent bandwith index from datarate
     */
    PHY_BW_FROM_DR,
}PhyAttribute_t;

/*!
 * Enumeration of initialization types.
 */
typedef enum eInitType
{
    /*!
     * Initializes the regional default settings for the band,
     * channel and default channels mask. Some regions also initiate
     * other default configurations. In general, this type is intended
     * to be called once during the initialization.
     */
    INIT_TYPE_DEFAULTS,
    /*!
     * Resets the channels mask to the default channels. Deactivates
     * all other channels.
     */
    INIT_TYPE_RESET_TO_DEFAULT_CHANNELS,
    /*!
     * Activates the default channels. Leaves all other active channels
     * active.
     */
    INIT_TYPE_ACTIVATE_DEFAULT_CHANNELS
}InitType_t;

typedef enum eChannelsMask
{
    /*!
     * The channels mask.
     */
    CHANNELS_MASK,
    /*!
     * The channels default mask.
     */
    CHANNELS_DEFAULT_MASK
}ChannelsMask_t;

/*!
 * Structure containing the beacon format
 */
typedef struct sBeaconFormat
{
    /*!
     * Size of the beacon
     */
    uint8_t BeaconSize;
    /*!
     * Size of the RFU 1 data field
     */
    uint8_t Rfu1Size;
    /*!
     * Size of the RFU 2 data field
     */
    uint8_t Rfu2Size;
}BeaconFormat_t;

/*!
 * Union for the structure uGetPhyParams
 */
typedef union uPhyParam
{
    /*!
     * A parameter value.
     */
    uint32_t Value;
    /*!
     * A floating point value.
     */
    float fValue;
    /*!
     * Pointer to the channels mask.
     */
    uint16_t* ChannelsMask;
    /*!
     * Pointer to the channels.
     */
    ChannelParams_t* Channels;
    /*!
     * Beacon format
     */
    BeaconFormat_t BeaconFormat;
    /*!
     * Duty Cycle Period
     */
    TimerTime_t DutyCycleTimePeriod;
}PhyParam_t;

/*!
 * Parameter structure for the function RegionGetPhyParam.
 */
typedef struct sGetPhyParams
{
    /*!
     * Setup the parameter to get.
     */
    PhyAttribute_t Attribute;
    /*!
     * Datarate.
     * The parameter is needed for the following queries:
     * PHY_MAX_PAYLOAD, PHY_NEXT_LOWER_TX_DR, PHY_SF_FROM_DR, PHY_BW_FROM_DR.
     */
    int8_t Datarate;
    /*!
     * Uplink dwell time. This parameter must be set to query:
     * PHY_MAX_PAYLOAD, PHY_MIN_TX_DR.
     * The parameter is needed for the following queries:
     * PHY_MIN_TX_DR, PHY_MAX_PAYLOAD, PHY_NEXT_LOWER_TX_DR.
     */
    uint8_t UplinkDwellTime;
    /*!
     * Downlink dwell time. This parameter must be set to query:
     * PHY_MAX_PAYLOAD, PHY_MIN_RX_DR.
     * The parameter is needed for the following queries:
     * PHY_MIN_RX_DR, PHY_MAX_PAYLOAD.
     */
    uint8_t DownlinkDwellTime;
    /*!
     * Specification of the downlink channel. Used in Class B only.
     * The parameter is needed for the following queries:
     * PHY_BEACON_CHANNEL_FREQ, PHY_PING_SLOT_CHANNEL_FREQ
     */
    uint8_t Channel;
}GetPhyParams_t;

/*!
 * Parameter structure for the function RegionSetBandTxDone.
 */
typedef struct sSetBandTxDoneParams
{
    /*!
     * Channel to update.
     */
    uint8_t Channel;
    /*!
     * Joined Set to true, if the node has joined the network
     */
    bool Joined;
    /*!
     * Last TX done time.
     */
    TimerTime_t LastTxDoneTime;
    /*!
     * Time-on-air of the last transmission.
     */
    TimerTime_t LastTxAirTime;
    /*!
     * Elapsed time since initialization.
     */
    SysTime_t ElapsedTimeSinceStartUp;
}SetBandTxDoneParams_t;

/*!
 * Parameter structure for the function RegionInitDefaults.
 */
typedef struct sInitDefaultsParams
{
    /*!
     * Pointer to region NVM group1.
     */
    void* NvmGroup1;
    /*!
     * Pointer to region NVM group2.
     */
    void* NvmGroup2;
    /*!
     * Pointer to common region band storage.
     */
    void* Bands;
    /*!
     * Sets the initialization type.
     */
    InitType_t Type;
}InitDefaultsParams_t;

/*!
 * Parameter structure for the function RegionVerify.
 */
typedef union uVerifyParams
{
    /*!
     * Channel frequency to verify
     */
    uint32_t Frequency;
    /*!
     * TX power to verify.
     */
    int8_t TxPower;
    /*!
     * Set to true, if the duty cycle is enabled, otherwise false.
     */
    bool DutyCycle;
    /*!
     * Datarate to verify.
     */
    struct sDatarateParams
    {
        /*!
        * Datarate to verify.
        */
        int8_t Datarate;
        /*!
        * The downlink dwell time.
        */
        uint8_t DownlinkDwellTime;
        /*!
        * The up link dwell time.
        */
        uint8_t UplinkDwellTime;
    }DatarateParams;
}VerifyParams_t;

/*!
 * Parameter structure for the function RegionApplyCFList.
 */
typedef struct sApplyCFListParams
{
    uint8_t JoinChannel;
    /*!
     * Payload which contains the CF list.
     */
    uint8_t* Payload;
    /*!
     * Size of the payload.
     */
    uint8_t Size;
}ApplyCFListParams_t;

/*!
 * Parameter structure for the function RegionChanMaskSet.
 */
typedef struct sChanMaskSetParams
{
    /*!
     * Pointer to the channels mask which should be set.
     */
    uint16_t* ChannelsMaskIn;
    /*!
     * Pointer to the channels mask which should be set.
     */
    ChannelsMask_t ChannelsMaskType;
}ChanMaskSetParams_t;

/*!
 * Parameter structure for the function RegionRxConfig.
 */
typedef struct sRxConfigParams
{
    /*!
     * The RX channel.
     */
    uint8_t Channel;
    /*!
     * RX datarate.
     */
    int8_t Datarate;
    /*!
     * RX bandwidth.
     */
    uint8_t Bandwidth;
    /*!
     * RX datarate offset.
     */
    int8_t DrOffset;
    /*!
     * RX frequency.
     */
    uint32_t Frequency;
    /*!
     * RX window timeout
     */
     uint32_t WindowTimeout;
    /*!
     * RX window offset
     */
    int32_t WindowOffset;
    /*!
     * Downlink dwell time.
     */
    uint8_t DownlinkDwellTime;
    /*!
     * Set to true, if RX should be continuous.
     */
    bool RxContinuous;
    /*!
     * Sets the RX window.
     */
    LoRaMacRxSlot_t RxSlot;
    /*!
     * LoRaWAN Network End-Device Activation ( ACTIVATION_TYPE_NONE, ACTIVATION_TYPE_ABP
     * or ACTIVATION_TYPE_OTTA )
     *
     * Related MIB type: \ref MIB_NETWORK_ACTIVATION
     */
    ActivationType_t NetworkActivation;
}RxConfigParams_t;

/*!
 * Parameter structure for the function RegionTxConfig.
 */
typedef struct sTxConfigParams
{
    /*!
     * The TX channel.
     */
    uint8_t Channel;
    /*!
     * The TX datarate.
     */
    int8_t Datarate;
    /*!
     * The TX power.
     */
    int8_t TxPower;
    /*!
     * The Max EIRP, if applicable.
     */
    float MaxEirp;
    /*!
     * The antenna gain, if applicable.
     */
    float AntennaGain;
    /*!
     * Frame length to setup.
     */
    uint16_t PktLen;
}TxConfigParams_t;

/*!
 * Parameter structure for the function RegionLinkAdrReq.
 */
typedef struct sLinkAdrReqParams
{
    /*!
     * Current LoRaWAN Version
     */
    Version_t Version;
    /*!
     * Pointer to the payload which contains the MAC commands.
     */
    uint8_t* Payload;
    /*!
     * Size of the payload.
     */
    uint8_t PayloadSize;
    /*!
     * Uplink dwell time.
     */
    uint8_t UplinkDwellTime;
    /*!
     * Set to true, if ADR is enabled.
     */
    bool AdrEnabled;
    /*!
     * The current datarate.
     */
    int8_t CurrentDatarate;
    /*!
     * The current TX power.
     */
    int8_t CurrentTxPower;
    /*!
     * The current number of repetitions.
     */
    uint8_t CurrentNbRep;
}LinkAdrReqParams_t;

/*!
 * Parameter structure for the function RegionRxParamSetupReq.
 */
typedef struct sRxParamSetupReqParams
{
    /*!
     * The datarate to setup.
     */
    int8_t Datarate;
    /*!
     * Datarate offset.
     */
    int8_t DrOffset;
    /*!
     * The frequency to setup.
     */
    uint32_t Frequency;
}RxParamSetupReqParams_t;

/*!
 * Parameter structure for the function RegionNewChannelReq.
 */
typedef struct sNewChannelReqParams
{
    /*!
     * Pointer to the new channels.
     */
    ChannelParams_t* NewChannel;
    /*!
     * Channel id.
     */
    int8_t ChannelId;
}NewChannelReqParams_t;

/*!
 * Parameter structure for the function RegionTxParamSetupReq.
 */
typedef struct sTxParamSetupReqParams
{
    /*!
     * Uplink dwell time.
     */
    uint8_t UplinkDwellTime;
    /*!
     * Downlink dwell time.
     */
    uint8_t DownlinkDwellTime;
    /*!
     * Max EIRP.
     */
    uint8_t MaxEirp;
}TxParamSetupReqParams_t;

/*!
 * Parameter structure for the function RegionDlChannelReq.
 */
typedef struct sDlChannelReqParams
{
    /*!
     * Channel Id to add the frequency.
     */
    uint8_t ChannelId;
    /*!
     * Alternative frequency for the Rx1 window.
     */
    uint32_t Rx1Frequency;
}DlChannelReqParams_t;

/*!
 * Enumeration of alternation type
 */
typedef enum eAlternateDrType
{
    /*!
     * Type to use for an alternation
     */
    ALTERNATE_DR,
    /*!
     * Type to use to restore one alternation
     */
    ALTERNATE_DR_RESTORE
}AlternateDrType_t;

/*!
 * Parameter structure for the function RegionNextChannel.
 */
typedef struct sNextChanParams
{
    /*!
     * Aggregated time-off time.
     */
    TimerTime_t AggrTimeOff;
    /*!
     * Time of the last aggregated TX.
     */
    TimerTime_t LastAggrTx;
    /*!
     * Current datarate.
     */
    int8_t Datarate;
    /*!
     * Set to true, if the node has already joined a network, otherwise false.
     */
    bool Joined;
    /*!
     * Set to true, if the duty cycle is enabled, otherwise false.
     */
    bool DutyCycleEnabled;
    /*!
     * Elapsed time since the start of the node.
     */
    SysTime_t ElapsedTimeSinceStartUp;
    /*!
     * Joined Set to true, if the last uplink was a join request
     */
    bool LastTxIsJoinRequest;
    /*!
     * Payload length of the next frame
     */
    uint16_t PktLen;
}NextChanParams_t;

/*!
 * Parameter structure for the function RegionChannelsAdd.
 */
typedef struct sChannelAddParams
{
    /*!
     * Pointer to the new channel to add.
     */
    ChannelParams_t* NewChannel;
    /*!
     * Channel id to add.
     */
    uint8_t ChannelId;
}ChannelAddParams_t;

/*!
 * Parameter structure for the function RegionChannelsRemove.
 */
typedef struct sChannelRemoveParams
{
    /*!
     * Channel id to remove.
     */
    uint8_t ChannelId;
}ChannelRemoveParams_t;

/*!
 * Parameter structure for the function RegionRxBeaconSetup
 */
typedef struct sRxBeaconSetupParams
{
    /*!
     * Symbol timeout.
     */
    uint16_t SymbolTimeout;
    /*!
     * Receive time.
     */
    uint32_t RxTime;
    /*!
     * The frequency to setup.
     */
    uint32_t Frequency;
}RxBeaconSetup_t;



/*!
 * \brief The function verifies if a region is active or not. If a region
 *        is not active, it cannot be used.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \retval Return true, if the region is supported.
 */
bool RegionIsActive( LoRaMacRegion_t region );

/*!
 * \brief The function gets a value of a specific phy attribute.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] getPhy Pointer to the function parameters.
 *
 * \retval Returns a structure containing the PHY parameter.
 */
PhyParam_t RegionGetPhyParam( LoRaMacRegion_t region, GetPhyParams_t* getPhy );

/*!
 * \brief Updates the last TX done parameters of the current channel.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] txDone Pointer to the function parameters.
 */
void RegionSetBandTxDone( LoRaMacRegion_t region, SetBandTxDoneParams_t* txDone );

/*!
 * \brief Initializes the channels masks and the channels.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] params Pointer to the function parameters.
 */
void RegionInitDefaults( LoRaMacRegion_t region, InitDefaultsParams_t* params );

/*!
 * \brief Verifies a parameter.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] verify Pointer to the function parameters.
 *
 * \param [IN] type Sets the initialization type.
 *
 * \retval Returns true, if the parameter is valid.
 */
bool RegionVerify( LoRaMacRegion_t region, VerifyParams_t* verify, PhyAttribute_t phyAttribute );

/*!
 * \brief The function parses the input buffer and sets up the channels of the
 *        CF list.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] applyCFList Pointer to the function parameters.
 */
void RegionApplyCFList( LoRaMacRegion_t region, ApplyCFListParams_t* applyCFList );

/*!
 * \brief Sets a channels mask.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] chanMaskSet Pointer to the function parameters.
 *
 * \retval Returns true, if the channels mask could be set.
 */
bool RegionChanMaskSet( LoRaMacRegion_t region, ChanMaskSetParams_t* chanMaskSet );

/*!
 * \brief Configuration of the RX windows.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] rxConfig Pointer to the function parameters.
 *
 * \param [OUT] datarate The datarate index which was set.
 *
 * \retval Returns true, if the configuration was applied successfully.
 */
bool RegionRxConfig( LoRaMacRegion_t region, RxConfigParams_t* rxConfig, int8_t* datarate );

/*
 * Rx window precise timing
 *
 * For more details please consult the following document, chapter 3.1.2.
 * https://www.semtech.com/uploads/documents/SX1272_settings_for_LoRaWAN_v2.0.pdf
 * or
 * https://www.semtech.com/uploads/documents/SX1276_settings_for_LoRaWAN_v2.0.pdf
 *
 *                 Downlink start: T = Tx + 1s (+/- 20 us)
 *                            |
 *             TRxEarly       |        TRxLate
 *                |           |           |
 *                |           |           +---+---+---+---+---+---+---+---+
 *                |           |           |       Latest Rx window        |
 *                |           |           +---+---+---+---+---+---+---+---+
 *                |           |           |
 *                +---+---+---+---+---+---+---+---+
 *                |       Earliest Rx window      |
 *                +---+---+---+---+---+---+---+---+
 *                            |
 *                            +---+---+---+---+---+---+---+---+
 *Downlink preamble 8 symbols |   |   |   |   |   |   |   |   |
 *                            +---+---+---+---+---+---+---+---+
 *
 *                     Worst case Rx window timings
 *
 * TRxLate  = DEFAULT_MIN_RX_SYMBOLS * tSymbol - RADIO_WAKEUP_TIME
 * TRxEarly = 8 - DEFAULT_MIN_RX_SYMBOLS * tSymbol - RxWindowTimeout - RADIO_WAKEUP_TIME
 *
 * TRxLate - TRxEarly = 2 * DEFAULT_SYSTEM_MAX_RX_ERROR
 *
 * RxOffset = ( TRxLate + TRxEarly ) / 2
 *
 * RxWindowTimeout = ( 2 * DEFAULT_MIN_RX_SYMBOLS - 8 ) * tSymbol + 2 * DEFAULT_SYSTEM_MAX_RX_ERROR
 * RxOffset = 4 * tSymbol - RxWindowTimeout / 2 - RADIO_WAKE_UP_TIME
 *
 * Minimal value of RxWindowTimeout must be 5 symbols which implies that the system always tolerates at least an error of 1.5 * tSymbol
 */
/*!
 * Computes the Rx window timeout and offset.
 *
 * \param [IN] region       LoRaWAN region.
 *
 * \param [IN] datarate     Rx window datarate index to be used
 *
 * \param [IN] minRxSymbols Minimum required number of symbols to detect an Rx frame.
 *
 * \param [IN] rxError      System maximum timing error of the receiver. In milliseconds
 *                          The receiver will turn on in a [-rxError : +rxError] ms
 *                          interval around RxOffset
 *
 * \param [OUT]rxConfigParams Returns updated WindowTimeout and WindowOffset fields.
 */
void RegionComputeRxWindowParameters( LoRaMacRegion_t region, int8_t datarate, uint8_t minRxSymbols, uint32_t rxError, RxConfigParams_t *rxConfigParams );

/*!
 * \brief TX configuration.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] txConfig Pointer to the function parameters.
 *
 * \param [OUT] txPower The tx power index which was set.
 *
 * \param [OUT] txTimeOnAir The time-on-air of the frame.
 *
 * \retval Returns true, if the configuration was applied successfully.
 */
bool RegionTxConfig( LoRaMacRegion_t region, TxConfigParams_t* txConfig, int8_t* txPower, TimerTime_t* txTimeOnAir );

/*!
 * \brief The function processes a Link ADR Request.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] linkAdrReq Pointer to the function parameters.
 *
 * \param [OUT] drOut The datarate which was applied.
 *
 * \param [OUT] txPowOut The TX power which was applied.
 *
 * \param [OUT] nbRepOut The number of repetitions to apply.
 *
 * \param [OUT] nbBytesParsed The number bytes which were parsed.
 *
 * \retval Returns the status of the operation, according to the LoRaMAC specification.
 */
uint8_t RegionLinkAdrReq( LoRaMacRegion_t region, LinkAdrReqParams_t* linkAdrReq, int8_t* drOut, int8_t* txPowOut, uint8_t* nbRepOut, uint8_t* nbBytesParsed );

/*!
 * \brief The function processes a RX Parameter Setup Request.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] rxParamSetupReq Pointer to the function parameters.
 *
 * \retval Returns the status of the operation, according to the LoRaMAC specification.
 */
uint8_t RegionRxParamSetupReq( LoRaMacRegion_t region, RxParamSetupReqParams_t* rxParamSetupReq );

/*!
 * \brief The function processes a New Channel Request.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] newChannelReq Pointer to the function parameters.
 *
 * \retval Returns the status of the operation, according to the LoRaMAC specification.
 */
int8_t RegionNewChannelReq( LoRaMacRegion_t region, NewChannelReqParams_t* newChannelReq );

/*!
 * \brief The function processes a TX ParamSetup Request.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] txParamSetupReq Pointer to the function parameters.
 *
 * \retval Returns the status of the operation, according to the LoRaMAC specification.
 *         Returns -1, if the functionality is not implemented. In this case, the end node
 *         shall ignore the command.
 */
int8_t RegionTxParamSetupReq( LoRaMacRegion_t region, TxParamSetupReqParams_t* txParamSetupReq );

/*!
 * \brief The function processes a DlChannel Request.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] dlChannelReq Pointer to the function parameters.
 *
 * \retval Returns the status of the operation, according to the LoRaMAC specification.
 */
int8_t RegionDlChannelReq( LoRaMacRegion_t region, DlChannelReqParams_t* dlChannelReq );

/*!
 * \brief Alternates the datarate of the channel for the join request.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] currentDr Current datarate.
 *
 * \param [IN] type Alternation type.
 *
 * \retval Datarate to apply.
 */
int8_t RegionAlternateDr( LoRaMacRegion_t region, int8_t currentDr, AlternateDrType_t type );

/*!
 * \brief Searches and set the next random available channel
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [OUT] channel Next channel to use for TX.
 *
 * \param [OUT] time Time to wait for the next transmission according to the duty
 *              cycle.
 *
 * \param [OUT] aggregatedTimeOff Updates the aggregated time off.
 *
 * \retval Function status [1: OK, 0: Unable to find a channel on the current datarate].
 */
LoRaMacStatus_t RegionNextChannel( LoRaMacRegion_t region, NextChanParams_t* nextChanParams, uint8_t* channel, TimerTime_t* time, TimerTime_t* aggregatedTimeOff );

/*!
 * \brief Adds a channel.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] channelAdd Pointer to the function parameters.
 *
 * \retval Status of the operation.
 */
LoRaMacStatus_t RegionChannelAdd( LoRaMacRegion_t region, ChannelAddParams_t* channelAdd );

/*!
 * \brief Removes a channel.
 *
 * \param [IN] region LoRaWAN region.
 *
 * \param [IN] channelRemove Pointer to the function parameters.
 *
 * \retval Returns true, if the channel was removed successfully.
 */
bool RegionChannelsRemove( LoRaMacRegion_t region, ChannelRemoveParams_t* channelRemove );

/*!
 * \brief Computes new datarate according to the given offset
 *
 * \param [IN] downlinkDwellTime Downlink dwell time configuration. 0: No limit, 1: 400ms
 *
 * \param [IN] dr Current datarate
 *
 * \param [IN] drOffset Offset to be applied
 *
 * \retval newDr Computed datarate.
 */
uint8_t RegionApplyDrOffset( LoRaMacRegion_t region, uint8_t downlinkDwellTime, int8_t dr, int8_t drOffset );

/*!
 * \brief Sets the radio into beacon reception mode
 *
 * \param [IN] rxBeaconSetup Pointer to the function parameters
 *
 * \param [out] outDr Datarate used to receive the beacon
 */
void RegionRxBeaconSetup( LoRaMacRegion_t region, RxBeaconSetup_t* rxBeaconSetup, uint8_t* outDr );

/*!
 * \brief Gets the version of the regional parameters implementation.
 *
 * \retval Version of the regional parameters.
 */
Version_t RegionGetVersion( void );

/*! \} defgroup REGION */

#ifdef __cplusplus
}
#endif

#endif // __REGION_H__
