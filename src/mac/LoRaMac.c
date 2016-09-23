/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech
 ___ _____ _   ___ _  _____ ___  ___  ___ ___
/ __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
\__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
|___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
embedded.connectivity.solutions===============

Description: LoRa MAC layer implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis ( Semtech ), Gregory Cristian ( Semtech ) and Daniel Jäckle ( STACKFORCE )
*/
#include "board.h"

#include "LoRaMacCrypto.h"
#include "LoRaMac.h"
#include "LoRaMacTest.h"

/*!
 * Maximum PHY layer payload size
 */
#define LORAMAC_PHY_MAXPAYLOAD                      255

/*!
 * Maximum MAC commands buffer size
 */
#define LORA_MAC_COMMAND_MAX_LENGTH                 15

/*!
 * FRMPayload overhead to be used when setting the Radio.SetMaxPayloadLength
 * in RxWindowSetup function.
 * Maximum PHYPayload = MaxPayloadOfDatarate/MaxPayloadOfDatarateRepeater + LORA_MAC_FRMPAYLOAD_OVERHEAD
 */
#define LORA_MAC_FRMPAYLOAD_OVERHEAD                13 // MHDR(1) + FHDR(7) + Port(1) + MIC(4)

/*!
 * Device IEEE EUI
 */
static uint8_t *LoRaMacDevEui;

/*!
 * Application IEEE EUI
 */
static uint8_t *LoRaMacAppEui;

/*!
 * AES encryption/decryption cipher application key
 */
static uint8_t *LoRaMacAppKey;

/*!
 * AES encryption/decryption cipher network session key
 */
static uint8_t LoRaMacNwkSKey[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*!
 * AES encryption/decryption cipher application session key
 */
static uint8_t LoRaMacAppSKey[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*!
 * Device nonce is a random value extracted by issuing a sequence of RSSI
 * measurements
 */
static uint16_t LoRaMacDevNonce;

/*!
 * Network ID ( 3 bytes )
 */
static uint32_t LoRaMacNetID;

/*!
 * Mote Address
 */
static uint32_t LoRaMacDevAddr;

/*!
 * Multicast channels linked list
 */
static MulticastParams_t *MulticastChannels = NULL;

/*!
 * Actual device class
 */
static DeviceClass_t LoRaMacDeviceClass;

/*!
 * Indicates if the node is connected to a private or public network
 */
static bool PublicNetwork;

/*!
 * Indicates if the node supports repeaters
 */
static bool RepeaterSupport;

/*!
 * Buffer containing the data to be sent or received.
 */
static uint8_t LoRaMacBuffer[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * Length of packet in LoRaMacBuffer
 */
static uint16_t LoRaMacBufferPktLen = 0;

/*!
 * Length of the payload in LoRaMacBuffer
 */
static uint8_t LoRaMacTxPayloadLen = 0;

/*!
 * Buffer containing the upper layer data.
 */
static uint8_t LoRaMacPayload[LORAMAC_PHY_MAXPAYLOAD];
static uint8_t LoRaMacRxPayload[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * LoRaMAC frame counter. Each time a packet is sent the counter is incremented.
 * Only the 16 LSB bits are sent
 */
static uint32_t UpLinkCounter = 1;

/*!
 * LoRaMAC frame counter. Each time a packet is received the counter is incremented.
 * Only the 16 LSB bits are received
 */
static uint32_t DownLinkCounter = 0;

/*!
 * IsPacketCounterFixed enables the MIC field tests by fixing the
 * UpLinkCounter value
 */
static bool IsUpLinkCounterFixed = false;

/*!
 * Used for test purposes. Disables the opening of the reception windows.
 */
static bool IsRxWindowsEnabled = true;

/*!
 * Indicates if the MAC layer has already joined a network.
 */
static bool IsLoRaMacNetworkJoined = false;

/*!
 * LoRaMac ADR control status
 */
static bool AdrCtrlOn = false;

/*!
 * Counts the number of missed ADR acknowledgements
 */
static uint32_t AdrAckCounter = 0;

/*!
 * If the node has sent a FRAME_TYPE_DATA_CONFIRMED_UP this variable indicates
 * if the nodes needs to manage the server acknowledgement.
 */
static bool NodeAckRequested = false;

/*!
 * If the server has sent a FRAME_TYPE_DATA_CONFIRMED_DOWN this variable indicates
 * if the ACK bit must be set for the next transmission
 */
static bool SrvAckRequested = false;

/*!
 * Indicates if the MAC layer wants to send MAC commands
 */
static bool MacCommandsInNextTx = false;

/*!
 * Contains the current MacCommandsBuffer index
 */
static uint8_t MacCommandsBufferIndex = 0;

/*!
 * Contains the current MacCommandsBuffer index for MAC commands to repeat
 */
static uint8_t MacCommandsBufferToRepeatIndex = 0;

/*!
 * Buffer containing the MAC layer commands
 */
static uint8_t MacCommandsBuffer[LORA_MAC_COMMAND_MAX_LENGTH];

/*!
 * Buffer containing the MAC layer commands which must be repeated
 */
static uint8_t MacCommandsBufferToRepeat[LORA_MAC_COMMAND_MAX_LENGTH];

#if defined( USE_BAND_433 )
/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 12, 11, 10,  9,  8,  7,  7, 50 };

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 59, 59, 59, 123, 250, 250, 250, 250 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 59, 59, 59, 123, 230, 230, 230, 230 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 20, 14, 11,  8,  5,  2 };

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS] =
{
    LC1,
    LC2,
    LC3,
};
#elif defined( USE_BAND_470 )

/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 12, 11, 10,  9,  8,  7 };

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 51, 51, 51, 115, 222, 222 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 51, 51, 51, 115, 222, 222 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 17, 16, 14, 12, 10, 7, 5, 2 };

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS];

/*!
 * Defines the first channel for RX window 1 for CN470 band
 */
#define LORAMAC_FIRST_RX1_CHANNEL           ( (uint32_t) 500.3e6 )

/*!
 * Defines the last channel for RX window 1 for CN470 band
 */
#define LORAMAC_LAST_RX1_CHANNEL            ( (uint32_t) 509.7e6 )

/*!
 * Defines the step width of the channels for RX window 1
 */
#define LORAMAC_STEPWIDTH_RX1_CHANNEL       ( (uint32_t) 200e3 )

/*!
 * Beacon frame size in bytes
 *
 * Remark: This is just a place holder and must be verified with
 * future specifications.
 */
#define BEACON_SIZE                                 17

/*!
 * Beacon channel frequency
 *
 * Remark: This is just a place holder and must be verified with
 * future specifications.
 */
#define BEACON_CHANNEL_FREQ( )                      505300000

/*!
 * Beacon channel frequency by index
 *
 * Remark: This is just a place holder and must be verified with
 * future specifications.
 */
#define BEACON_CHANNEL_FREQ_IDX( x )                505300000

/*!
 * Beacon channel datarate
 *
 * Remark: This is just a place holder and must be verified with
 * future specifications.
 */
#define BEACON_CHANNEL_DR                           DR_0

/*!
 * Beacon channel bandwidth
 *
 * Remark: This is just a place holder and must be verified with
 * future specifications.
 */
#define BEACON_CHANNEL_BW                           0

/*!
 * Ping slot channel frequency
 *
 * Remark: This is just a place holder and must be verified with
 * future specifications.
 */
#define PINGSLOT_CHANNEL_FREQ( x )                  505300000

#elif defined( USE_BAND_780 )
/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 12, 11, 10,  9,  8,  7,  7, 50 };

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 59, 59, 59, 123, 250, 250, 250, 250 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 59, 59, 59, 123, 230, 230, 230, 230 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 20, 14, 11,  8,  5,  2 };

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS] =
{
    LC1,
    LC2,
    LC3,
};
#elif defined( USE_BAND_868 )
/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 12, 11, 10,  9,  8,  7,  7, 50 };

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 51, 51, 51, 115, 242, 242, 242, 242 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 51, 51, 51, 115, 222, 222, 222, 222 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 20, 14, 11,  8,  5,  2 };

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
    BAND1,
    BAND2,
    BAND3,
    BAND4,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS] =
{
    LC1,
    LC2,
    LC3,
};
/*!
 * Beacon frame size in bytes
 */
#define BEACON_SIZE                                 17

/*!
 * Beacon channel frequency
 */
#define BEACON_CHANNEL_FREQ( )                      869525000

/*!
 * Beacon channel frequency by index
 */
#define BEACON_CHANNEL_FREQ_IDX( x )                869525000

/*!
 * Beacon channel datarate
 */
#define BEACON_CHANNEL_DR                           DR_3

/*!
 * Beacon channel bandwidth
 */
#define BEACON_CHANNEL_BW                           0

#define PINGSLOT_CHANNEL_FREQ( x )                  869525000

#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
/*!
 * Data rates table definition
 */
const uint8_t Datarates[]  = { 10, 9, 8,  7,  8,  0,  0, 0, 12, 11, 10, 9, 8, 7, 0, 0 };

/*!
 * Up/Down link data rates offset definition
 */
const int8_t datarateOffsets[5][4] =
{
    { DR_10, DR_9 , DR_8 , DR_8  }, // DR_0
    { DR_11, DR_10, DR_9 , DR_8  }, // DR_1
    { DR_12, DR_11, DR_10, DR_9  }, // DR_2
    { DR_13, DR_12, DR_11, DR_10 }, // DR_3
    { DR_13, DR_13, DR_12, DR_11 }, // DR_4
};

/*!
 * Maximum payload with respect to the datarate index. Cannot operate with repeater.
 */
const uint8_t MaxPayloadOfDatarate[] = { 11, 53, 125, 242, 242, 0, 0, 0, 53, 129, 242, 242, 242, 242, 0, 0 };

/*!
 * Maximum payload with respect to the datarate index. Can operate with repeater.
 */
const uint8_t MaxPayloadOfDatarateRepeater[] = { 11, 53, 125, 242, 242, 0, 0, 0, 33, 109, 222, 222, 222, 222, 0, 0 };

/*!
 * Tx output powers table definition
 */
const int8_t TxPowers[]    = { 30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10 };

/*!
 * LoRaMac bands
 */
static Band_t Bands[LORA_MAX_NB_BANDS] =
{
    BAND0,
};

/*!
 * LoRaMAC channels
 */
static ChannelParams_t Channels[LORA_MAX_NB_CHANNELS];

/*!
 * Contains the channels which remain to be applied.
 */
static uint16_t ChannelsMaskRemaining[6];

/*!
 * Defines the first channel for RX window 1 for US band
 */
#define LORAMAC_FIRST_RX1_CHANNEL           ( (uint32_t) 923.3e6 )

/*!
 * Defines the last channel for RX window 1 for US band
 */
#define LORAMAC_LAST_RX1_CHANNEL            ( (uint32_t) 927.5e6 )

/*!
 * Defines the step width of the channels for RX window 1
 */
#define LORAMAC_STEPWIDTH_RX1_CHANNEL       ( (uint32_t) 600e3 )

/*!
 * Beacon frame size in bytes
 */
#define BEACON_SIZE                                 19

/*!
 * Beacon channel frequency
 */
#define BEACON_CHANNEL_FREQ( )                      ( 923.3e6 + ( BeaconChannel( 0 ) * 600e3 ) )

/*!
 * Beacon channel frequency by index
 */
#define BEACON_CHANNEL_FREQ_IDX( x )                ( 923.3e6 + ( x * 600e3 ) )

/*!
 * Beacon channel datarate
 */
#define BEACON_CHANNEL_DR                           DR_10

/*!
 * Beacon channel bandwidth
 */
#define BEACON_CHANNEL_BW                           2

#define PINGSLOT_CHANNEL_FREQ( x )                  ( 923.3e6 + ( BeaconChannel( x ) * 600e3 ) )

#else
    #error "Please define a frequency band in the compiler options."
#endif

/*!
 * LoRaMac parameters
 */
LoRaMacParams_t LoRaMacParams;

/*!
 * LoRaMac default parameters
 */
LoRaMacParams_t LoRaMacParamsDefaults;

/*!
 * Uplink messages repetitions counter
 */
static uint8_t ChannelsNbRepCounter = 0;

/*!
 * Maximum duty cycle
 * \remark Possibility to shutdown the device.
 */
static uint8_t MaxDCycle = 0;

/*!
 * Aggregated duty cycle management
 */
static uint16_t AggregatedDCycle;
static TimerTime_t AggregatedLastTxDoneTime;
static TimerTime_t AggregatedTimeOff;

/*!
 * Enables/Disables duty cycle management (Test only)
 */
static bool DutyCycleOn;

/*!
 * Current channel index
 */
static uint8_t Channel;

/*!
 * Channel index of the last transmission
 */
static uint8_t LastTxChannel;

/*!
 * LoRaMac internal states
 */
enum eLoRaMacState
{
    MAC_IDLE          = 0x00000000,
    MAC_TX_RUNNING    = 0x00000001,
    MAC_RX            = 0x00000002,
    MAC_ACK_REQ       = 0x00000004,
    MAC_ACK_RETRY     = 0x00000008,
    MAC_TX_DELAYED    = 0x00000010,
    MAC_TX_CONFIG     = 0x00000020,
    MAC_RX_ABORT      = 0x00000040,
};

/*!
 * LoRaMac internal state
 */
uint32_t LoRaMacState = MAC_IDLE;

/*!
 * LoRaMac timer used to check the LoRaMacState (runs every second)
 */
static TimerEvent_t MacStateCheckTimer;

/*!
 * LoRaMac upper layer event functions
 */
static LoRaMacPrimitives_t *LoRaMacPrimitives;

/*!
 * LoRaMac upper layer callback functions
 */
static LoRaMacCallback_t *LoRaMacCallbacks;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*!
 * LoRaMac duty cycle delayed Tx timer
 */
static TimerEvent_t TxDelayedTimer;

/*!
 * LoRaMac reception windows timers
 */
static TimerEvent_t RxWindowTimer1;
static TimerEvent_t RxWindowTimer2;

/*!
 * LoRaMac reception windows delay
 * \remark normal frame: RxWindowXDelay = ReceiveDelayX - RADIO_WAKEUP_TIME
 *         join frame  : RxWindowXDelay = JoinAcceptDelayX - RADIO_WAKEUP_TIME
 */
static uint32_t RxWindow1Delay;
static uint32_t RxWindow2Delay;

/*!
 * Acknowledge timeout timer. Used for packet retransmissions.
 */
static TimerEvent_t AckTimeoutTimer;

/*!
 * Number of trials to get a frame acknowledged
 */
static uint8_t AckTimeoutRetries = 1;

/*!
 * Number of trials to get a frame acknowledged
 */
static uint8_t AckTimeoutRetriesCounter = 1;

/*!
 * Indicates if the AckTimeout timer has expired or not
 */
static bool AckTimeoutRetry = false;

/*!
 * Last transmission time on air
 */
TimerTime_t TxTimeOnAir = 0;

/*!
 * Number of trials for the Join Request
 */
static uint16_t JoinRequestTrials;

/*!
 * Structure to hold an MCPS indication data.
 */
static McpsIndication_t McpsIndication;

/*!
 * Structure to hold MCPS confirm data.
 */
static McpsConfirm_t McpsConfirm;

/*!
 * Structure to hold MLME indication data.
 */
static MlmeIndication_t MlmeIndication;

/*!
 * Structure to hold MLME confirm data.
 */
static MlmeConfirm_t MlmeConfirm;

/*!
 * Structure to hold multiple MLME request confirm data
 */
typedef struct sMlmeConfirmQueue
{
    /*!
     * Holds the previously performed MLME-Request
     */
    Mlme_t MlmeRequest;
    /*!
     * Status of the operation
     */
    LoRaMacEventInfoStatus_t Status;
}sMlmeConfirmQueue_t;

/*!
 * MlmeConfirm queue data structure
 */
static sMlmeConfirmQueue_t MlmeConfirmQueue[LORA_MAC_MLME_CONFIRM_QUEUE_LEN];

/*!
 * Counts the number of MlmeConfirms to process
 */
static uint8_t MlmeConfirmQueueCnt;

/*!
 * Holds the current rx window slot
 */
static uint8_t RxSlot = 0;

/*!
 * LoRaMac tx/rx operation state
 */
LoRaMacFlags_t LoRaMacFlags;

/*!
 * States of the class B beacon acquisition and tracking
 */
typedef enum eBeaconState
{
    /*!
     * Initial state to acquire the beacon
     */
    BEACON_STATE_ACQUISITION,
    /*!
     * Handles the state when the beacon reception fails
     */
    BEACON_STATE_TIMEOUT,
    /*!
     * Reacquisition state which applies the algorithm to enlarge the reception
     * windows
     */
    BEACON_STATE_REACQUISITION,
    /*!
     * The node has locked a beacon successfully
     */
    BEACON_STATE_LOCKED,
    /*!
     * The beacon state machine is stopped due to operations with higher priority
     */
    BEACON_STATE_HALT,
    /*!
     * The node currently operates in the beacon window and is idle. In this
     * state, the temperature measurement takes place
     */
    BEACON_STATE_IDLE,
    /*!
     * The node operates in the guard time of class B
     */
    BEACON_STATE_GUARD,
    /*!
     * The node is in receive mode to lock a beacon
     */
    BEACON_STATE_RX,
    /*!
     * The nodes switches the device class
     */
    BEACON_STATE_SWITCH_CLASS,
}BeaconState_t;

/*!
 * State of the beaconing mechanism
 */
static BeaconState_t BeaconState;

/*!
 * States of the class B ping slot mechanism
 */
typedef enum ePingSlotState
{
    /*!
     * Calculation of the ping slot offset
     */
    PINGSLOT_STATE_CALC_PING_OFFSET,
    /*!
     * State to set the timer to open the next ping slot
     */
    PINGSLOT_STATE_SET_TIMER,
    /*!
     * The node is in idle state
     */
    PINGSLOT_STATE_IDLE,
    /*!
     * The node opens up a ping slot window
     */
    PINGSLOT_STATE_RX,
}PingSlotState_t;

/*!
 * State of the ping slot mechanism
 */
static PingSlotState_t PingSlotState;

/*!
 * State of the multicast slot mechanism
 */
static PingSlotState_t MulticastSlotState;

/*!
 * Class B ping slot context structure
 */
typedef struct sPingSlotContext
{
    struct sPingSlotCtrl
    {
        /*!
         * Set when the server assigned a ping slot to the node
         */
        uint8_t Assigned         : 1;
        /*!
         * Set when a custom frequency is used
         */
        uint8_t CustomFreq       : 1;
    }Ctrl;

    struct sPingSlotCfg
    {
        /*!
         * Ping slot length time in ms
         */
        uint32_t PingSlotWindow;
        /*!
         * Maximum symbol timeout for ping slots
         */
        uint32_t SymbolToExpansionMax;
        /*!
         * Symbol expansion value for ping slot windows in case of beacon
         * loss in symbols
         */
        uint32_t SymbolToExpansionFactor;
    }Cfg;
    /*!
     * Number of ping slots
     */
    uint8_t PingNb;
    /*!
     * Period of the ping slots
     */
    uint16_t PingPeriod;
    /*!
     * Ping offset
     */
    uint16_t PingOffset;
    /*!
     * Reception frequency of the ping slot windows
     */
    uint32_t Frequency;
    /*!
     * Datarate of the ping slot
     */
    int8_t Datarate;
    /*!
     * Data range of the ping slot windows
     */
    DrRange_t DrRange;
    /*!
     * Current symbol timeout. The node enlarges this variable in case of beacon
     * loss.
     */
    uint16_t SymbolTimeout;
    /*!
     * The multicast channel which will be enabled next.
     */
    MulticastParams_t *NextMulticastChannel;
}sPingSlotContext_t;

/*!
 * Class B ping slot context
 */
static sPingSlotContext_t PingSlotCtx;

/*!
 * Class B beacon context structure
 */
typedef struct sBeaconContext
{
    struct sBeaconCtrl
    {
        /*!
         * Set if the node has lost the beacon reference
         */
        uint8_t BeaconLess          : 1;
        /*!
         * Set if the node has a custom frequency for beaconing and ping slots
         */
        uint8_t CustomFreq          : 1;
        /*!
         * Set if a beacon delay was set for the beacon acquisition
         */
        uint8_t BeaconDelaySet      : 1;
        /*!
         * Set if a beacon channel was set for the beacon acquisition
         */
        uint8_t BeaconChannelSet    : 1;
        /*!
         * Set if beacon acquisition is pending
         */
        uint8_t AcquisitionPending  : 1;
    }Ctrl;

    struct sBeaconCfg
    {
        /*!
         * Beacon interval in ms
         */
        uint32_t Interval;
        /*!
         * Beacon reserved time in ms
         */
        uint32_t Reserved;
        /*!
         * Beacon guard time in ms
         */
        uint32_t Guard;
        /*!
         * Beacon window time in ms
         */
        uint32_t Window;
        /*!
         * Beacon window time in numer of slots
         */
        uint32_t WindowSlots;
        /*!
         * Default symbol timeout for beacons and ping slot windows
         */
        uint32_t SymbolToDefault;
        /*!
         * Maximum symbol timeout for beacons
         */
        uint32_t SymbolToExpansionMax;
        /*!
         * Symbol expansion value for beacon windows in case of beacon
         * loss in symbols
         */
        uint32_t SymbolToExpansionFactor;
        /*!
         * Symbol expansion value for ping slot windows in case of beacon
         * loss in symbols
         */
        uint32_t MaxBeaconLessPeriod;
        /*!
         * Delay time for the BeaconTimingAns in ms
         */
        uint32_t DelayBeaconTimingAns;
    }Cfg;
    /*!
     * Beacon reception frequency
     */
    uint32_t Frequency;
    /*!
     * Current temperature
     */
    float Temperature;
    /*!
     * Beacon time received with the beacon frame
     */
    TimerTime_t BeaconTime;
    /*!
     * Time when the last beacon was received
     */
    TimerTime_t LastBeaconRx;
    /*!
     * Time when the next beacon will be received
     */
    TimerTime_t NextBeaconRx;
    /*!
     * Current symbol timeout. The node enlarges this variable in case of beacon
     * loss.
     */
    uint16_t SymbolTimeout;
    /*!
     * Listen time for the beacon in case of reception timeout
     */
    TimerTime_t ListenTime;
    /*!
     * Beacon timing channel for next beacon
     */
    uint8_t BeaconTimingChannel;
    /*!
     * Delay for next beacon in ms
     */
    TimerTime_t BeaconTimingDelay;
}BeaconContext_t;

/*!
 * Class B beacon context
 */
static BeaconContext_t BeaconCtx;

/*!
 * Timer for CLASS B beacon acquisition and tracking.
 */
static TimerEvent_t BeaconTimer;

/*!
 * Timer for CLASS B ping slot timer.
 */
static TimerEvent_t PingSlotTimer;

/*!
 * Timer for CLASS B multicast ping slot timer.
 */
static TimerEvent_t MulticastSlotTimer;

/*!
 * \brief Function to be executed on Radio Tx Done event
 */
static void OnRadioTxDone( void );

/*!
 * \brief This function prepares the MAC to abort the execution of function
 *        OnRadioRxDone in case of a reception error.
 */
static void PrepareRxDoneAbort( void );

/*!
 * \brief Function to be executed on Radio Rx Done event
 */
static void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

/*!
 * \brief Function executed on Radio Tx Timeout event
 */
static void OnRadioTxTimeout( void );

/*!
 * \brief Function executed on Radio Rx error event
 */
static void OnRadioRxError( void );

/*!
 * \brief Function executed on Radio Rx Timeout event
 */
static void OnRadioRxTimeout( void );

/*!
 * \brief Function executed on Resend Frame timer event.
 */
static void OnMacStateCheckTimerEvent( void );

/*!
 * \brief Function executed on duty cycle delayed Tx  timer event
 */
static void OnTxDelayedTimerEvent( void );

/*!
 * \brief Function executed on first Rx window timer event
 */
static void OnRxWindow1TimerEvent( void );

/*!
 * \brief Function executed on second Rx window timer event
 */
static void OnRxWindow2TimerEvent( void );

/*!
 * \brief Function executed on AckTimeout timer event
 */
static void OnAckTimeoutTimerEvent( void );

/*!
 * \brief Searches and set the next random available channel
 *
 * \param [OUT] Time to wait for the next transmission according to the duty
 *              cycle.
 *
 * \retval status  Function status [1: OK, 0: Unable to find a channel on the
 *                                  current datarate]
 */
static bool SetNextChannel( TimerTime_t* time );

/*!
 * \brief Sets the network to public or private. Updates the sync byte.
 *
 * \param [IN] enable if true, it enables a public network
 */
static void SetPublicNetwork( bool enable );

/*!
 * \brief Returns the symbol timeout for the given datarate
 *
 * \param [IN] datarate Current datarate
 *
 * \retval Symbol timeout
 */
static uint16_t GetRxSymbolTimeout( int8_t datarate );

/*!
 * \brief Returns the bandwidth for the given datarate
 *
 * \param [IN] datarate Current datarate
 *
 * \retval Bandwidth
 */
static uint32_t GetRxBandwidth( int8_t datarate );

/*!
 * \brief Initializes and opens the reception window
 *
 * \param [IN] freq window channel frequency
 * \param [IN] datarate window channel datarate
 * \param [IN] bandwidth window channel bandwidth
 * \param [IN] timeout window channel timeout
 */
static void RxWindowSetup( uint32_t freq, int8_t datarate, uint32_t bandwidth, uint16_t timeout, bool rxContinuous );

/*!
 * \brief Verifies if the RX window 2 frequency is in range
 *
 * \param [IN] freq window channel frequency
 *
 * \retval status  Function status [1: OK, 0: Frequency not applicable]
 */
static bool Rx2FreqInRange( uint32_t freq );

/*!
 * \brief Switches the device class
 *
 * \param [IN] deviceClass Device class to switch to
 */
static LoRaMacStatus_t SwitchClass( DeviceClass_t deviceClass );

/*!
 * \brief Beacon acquisition and tracking state machine
 */
static void OnBeaconTimerEvent( void );

/*!
 * \brief Switches the device class
 *
 * \param [IN] slotOffset The ping slot offset
 * \param [IN] pingPeriod The ping period
 * \param [OUT] timeOffset Time offset of the next slot, based on current time
 *
 * \retval [true: ping slot found, false: no ping slot found]
 */
static bool CalcNextSlotTime( uint16_t slotOffset, uint16_t pingPeriod, TimerTime_t* timeOffset );

/*!
 * \brief Ping slot state machine
 */
static void OnPingSlotTimerEvent( void );

/*!
 * \brief Multicast slot state machine
 */
static void OnMulticastSlotTimerEvent( void );

/*!
 * \brief Initializes and opens the beacon reception window
 *
 * \param [IN] timeout window channel timeout
 * \param [IN] rxTime time to spend in rx mode
 */
static void RxBeaconSetup( uint16_t timeout, uint32_t rxTime );

/*!
 * \brief Receives and decodes the beacon frame
 *
 * \param [IN] payload Pointer to the payload
 * \param [IN] size Size of the payload
 */
static bool RxBeacon( uint8_t *payload, uint16_t size );

/*!
 * \brief Calculates CRC's of the beacon frame
 *
 * \param [IN] buffer Pointer to the data
 * \param [IN] length Length of the data
 *
 * \retval CRC
 */
static uint16_t BeaconCrc( uint8_t *buffer, uint16_t length );

/*!
 * \brief The function validates, if the node expects a beacon at the current
 *        time.
 *
 * \retval [true, if the node expects a beacon; false, if not]
 */
static bool IsBeaconExpected( void );

/*!
 * \brief The function validates, if the node expects a ping slot at the current
 *        time
 *
 * \retval [true, if the node expects a ping slot; false, if not]
 */
static bool IsPingExpected( void );

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
/*!
 * \brief Calculates the reception channel for the beacon for US915
 *
 * \param [IN] devAddr Device address
 *
 * \retval Channel
 */
static uint8_t BeaconChannel( uint32_t devAddr );
#endif

/*!
 * \brief Stops the beacon and ping slot operation.
 */
static void HaltBeaconing( void );

/*!
 * \brief Resumes the beacon and ping slot operation.
 */
static void ResumeBeaconing( void );

/*!
 * \brief Adds a new MAC command to be sent.
 *
 * \Remark MAC layer internal function
 *
 * \param [in] cmd MAC command to be added
 *                 [MOTE_MAC_LINK_CHECK_REQ,
 *                  MOTE_MAC_LINK_ADR_ANS,
 *                  MOTE_MAC_DUTY_CYCLE_ANS,
 *                  MOTE_MAC_RX2_PARAM_SET_ANS,
 *                  MOTE_MAC_DEV_STATUS_ANS
 *                  MOTE_MAC_NEW_CHANNEL_ANS]
 * \param [in] p1  1st parameter ( optional depends on the command )
 * \param [in] p2  2nd parameter ( optional depends on the command )
 *
 * \retval status  Function status [0: OK, 1: Unknown command, 2: Buffer full]
 */
static LoRaMacStatus_t AddMacCommand( uint8_t cmd, uint8_t p1, uint8_t p2 );

/*!
 * \brief Parses the MAC commands which must be repeated.
 *
 * \Remark MAC layer internal function
 *
 * \param [IN] cmdBufIn  Buffer which stores the MAC commands to send
 * \param [IN] length  Length of the input buffer to parse
 * \param [OUT] cmdBufOut  Buffer which stores the MAC commands which must be
 *                         repeated.
 *
 * \retval Size of the MAC commands to repeat.
 */
static uint8_t ParseMacCommandsToRepeat( uint8_t* cmdBufIn, uint8_t length, uint8_t* cmdBufOut );

/*!
 * \brief Validates if the payload fits into the frame, taking the datarate
 *        into account.
 *
 * \details Refer to chapter 4.3.2 of the LoRaWAN specification, v1.0
 *
 * \param lenN Length of the application payload. The length depends on the
 *             datarate and is region specific
 *
 * \param datarate Current datarate
 *
 * \param fOptsLen Length of the fOpts field
 *
 * \retval [false: payload does not fit into the frame, true: payload fits into
 *          the frame]
 */
static bool ValidatePayloadLength( uint8_t lenN, int8_t datarate, uint8_t fOptsLen );

/*!
 * \brief Counts the number of bits in a mask.
 *
 * \param [IN] mask A mask from which the function counts the active bits.
 * \param [IN] nbBits The number of bits to check.
 *
 * \retval Number of enabled bits in the mask.
 */
static uint8_t CountBits( uint16_t mask, uint8_t nbBits );

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
/*!
 * \brief Counts the number of enabled 125 kHz channels in the channel mask.
 *        This function can only be applied to US915 band.
 *
 * \param [IN] channelsMask Pointer to the first element of the channel mask
 *
 * \retval Number of enabled channels in the channel mask
 */
static uint8_t CountNbEnabled125kHzChannels( uint16_t *channelsMask );

#if defined( USE_BAND_915_HYBRID )
/*!
 * \brief Validates the correctness of the channel mask for US915, hybrid mode.
 *
 * \param [IN] mask Block definition to set.
 * \param [OUT] channelsMask Pointer to the first element of the channel mask
 */
static void ReenableChannels( uint16_t mask, uint16_t* channelsMask );

/*!
 * \brief Validates the correctness of the channel mask for US915, hybrid mode.
 *
 * \param [IN] channelsMask Pointer to the first element of the channel mask
 *
 * \retval [true: channel mask correct, false: channel mask not correct]
 */
static bool ValidateChannelMask( uint16_t* channelsMask );
#endif

#endif

/*!
 * \brief Validates the correctness of the datarate against the enable channels.
 *
 * \param [IN] datarate Datarate to be check
 * \param [IN] channelsMask Pointer to the first element of the channel mask
 *
 * \retval [true: datarate can be used, false: datarate can not be used]
 */
static bool ValidateDatarate( int8_t datarate, uint16_t* channelsMask );

/*!
 * \brief Limits the Tx power according to the number of enabled channels
 *
 * \retval Returns the maximum valid tx power
 */
static int8_t LimitTxPower( int8_t txPower );

/*!
 * \brief Verifies, if a value is in a given range.
 *
 * \param [IN] value Value to verify, if it is in range
 * \param [IN] min Minimum possible value
 * \param [IN] max Maximum possible value
 *
 * \retval Returns true, if the value is in range.
 */
static bool ValueInRange( int8_t value, int8_t min, int8_t max );

/*!
 * \brief Verifies, if a the datarate range is valid.
 *
 * \param [IN] drRange Datarate range to validate.
 * \param [IN] min Minimum possible value.
 * \param [IN] max Maximum possible value.
 *
 * \retval Returns true, if drRange is valid.
 */
static bool ValidateDrRange( DrRange_t drRange, int8_t min, int8_t max );

/*!
 * \brief Calculates the next datarate to set, when ADR is on or off
 *
 * \param [IN] adrEnabled Specify whether ADR is on or off
 *
 * \param [IN] updateChannelMask Set to true, if the channel masks shall be updated
 *
 * \param [OUT] datarateOut Reports the datarate which will be used next
 *
 * \retval Returns the state of ADR ack request
 */
static bool AdrNextDr( bool adrEnabled, bool updateChannelMask, int8_t* datarateOut );

/*!
 * \brief Disables channel in a specified channel mask
 *
 * \param [IN] id - Id of the channel
 *
 * \param [IN] mask - Pointer to the channel mask to edit
 *
 * \retval [true, if disable was successful, false if not]
 */
static bool DisableChannelInMask( uint8_t id, uint16_t* mask );

/*!
 * \brief Decodes MAC commands in the fOpts field and in the payload
 */
static void ProcessMacCommands( uint8_t *payload, uint8_t macIndex, uint8_t commandsSize, uint8_t snr );

/*!
 * \brief LoRaMAC layer generic send frame
 *
 * \param [IN] macHdr      MAC header field
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*!
 * \brief LoRaMAC layer frame buffer initialization
 *
 * \param [IN] macHdr      MAC header field
 * \param [IN] fCtrl       MAC frame control field
 * \param [IN] fOpts       MAC commands buffer
 * \param [IN] fPort       MAC payload port
 * \param [IN] fBuffer     MAC data buffer to be sent
 * \param [IN] fBufferSize MAC data buffer size
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t PrepareFrame( LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t fPort, void *fBuffer, uint16_t fBufferSize );

/*
 * \brief Schedules the frame according to the duty cycle
 *
 * \retval Status of the operation
 */
static LoRaMacStatus_t ScheduleTx( void );

/*
 * \brief Sets the duty cycle for retransmissions
 *
 * \retval Duty cycle
 */
static uint16_t RetransmissionDutyCylce( void );

/*
 * \brief Calculates the back-off time for the band of a channel.
 *
 * \param [IN] channel     The last Tx channel index
 */
static void CalculateBackOff( uint8_t channel );

/*
 * \brief Alternates the datarate of the channel for the join request.
 *
 * \param [IN] nbTrials    Number of performed join requests.
 * \retval Datarate to apply
 */
static int8_t AlternateDatarate( uint16_t nbTrials );

/*
 * \brief Gets the index of the confirm queue of a specific MLME-request
 *
 * \param [IN] queue        MLME-Confirm queue pointer
 * \param [IN] req          MLME-Request to validate
 * \param [IN] length       Number of items to check
 * \retval Index of the MLME-Confirm. 0xFF is no entry found.
 */
static uint8_t GetMlmeConfirmIndex( sMlmeConfirmQueue_t* queue, Mlme_t req, uint8_t length );

/*!
 * \brief LoRaMAC layer prepared frame buffer transmission with channel specification
 *
 * \remark PrepareFrame must be called at least once before calling this
 *         function.
 *
 * \param [IN] channel     Channel parameters
 * \retval status          Status of the operation.
 */
TimerTime_t SendFrameOnChannel( ChannelParams_t channel );

/*!
 * \brief Resets MAC specific parameters to default
 */
static void ResetMacParameters( void );

static void OnRadioTxDone( void )
{
    TimerTime_t curTime = TimerGetCurrentTime( );
    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    // Store last Tx channel
    LastTxChannel = Channel;
    // Update last tx done time for the current channel
    Bands[Channels[LastTxChannel].Band].LastTxDoneTime = curTime;
    // Update Aggregated last tx done time
    AggregatedLastTxDoneTime = curTime;

    if( IsRxWindowsEnabled == true )
    {
        TimerSetValue( &RxWindowTimer1, RxWindow1Delay );
        TimerStart( &RxWindowTimer1 );
        if( LoRaMacDeviceClass != CLASS_C )
        {
            TimerSetValue( &RxWindowTimer2, RxWindow2Delay );
            TimerStart( &RxWindowTimer2 );
        }
        if( ( LoRaMacDeviceClass == CLASS_C ) || ( NodeAckRequested == true ) )
        {
            TimerSetValue( &AckTimeoutTimer, RxWindow2Delay + ACK_TIMEOUT +
                                             randr( -ACK_TIMEOUT_RND, ACK_TIMEOUT_RND ) );
            TimerStart( &AckTimeoutTimer );
        }
    }
    else
    {
        McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;

        if( LoRaMacFlags.Value == 0 )
        {
            LoRaMacFlags.Bits.McpsReq = 1;
        }
        LoRaMacFlags.Bits.MacDone = 1;
    }

    if( NodeAckRequested == false )
    {
        McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
        ChannelsNbRepCounter++;
    }
}

static void PrepareRxDoneAbort( void )
{
    LoRaMacState |= MAC_RX_ABORT;

    if( NodeAckRequested )
    {
        OnAckTimeoutTimerEvent( );
    }

    if( ( RxSlot == 0 ) && ( LoRaMacDeviceClass == CLASS_C ) )
    {
        OnRxWindow2TimerEvent( );
    }

    LoRaMacFlags.Bits.McpsInd = 1;
    LoRaMacFlags.Bits.MacDone = 1;

    // Trig OnMacCheckTimerEvent call as soon as possible
    TimerSetValue( &MacStateCheckTimer, 1 );
    TimerStart( &MacStateCheckTimer );
}

static void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;
    bool skipIndication = false;

    uint8_t pktHeaderLen = 0;
    uint32_t address = 0;
    uint8_t appPayloadStartIndex = 0;
    uint8_t port = 0xFF;
    uint8_t frameLen = 0;
    uint32_t mic = 0;
    uint32_t micRx = 0;

    uint16_t sequenceCounter = 0;
    uint16_t sequenceCounterPrev = 0;
    uint16_t sequenceCounterDiff = 0;
    uint32_t downLinkCounter = 0;

    MulticastParams_t *curMulticastParams = NULL;
    uint8_t *nwkSKey = LoRaMacNwkSKey;
    uint8_t *appSKey = LoRaMacAppSKey;

    uint8_t multicast = 0;

    bool isMicOk = false;

    McpsConfirm.AckReceived = false;
    McpsIndication.Rssi = rssi;
    McpsIndication.Snr = snr;
    McpsIndication.RxSlot = RxSlot;
    McpsIndication.Port = 0;
    McpsIndication.Multicast = 0;
    McpsIndication.FramePending = 0;
    McpsIndication.Buffer = NULL;
    McpsIndication.BufferSize = 0;
    McpsIndication.RxData = false;
    McpsIndication.AckReceived = false;
    McpsIndication.DownLinkCounter = 0;
    McpsIndication.McpsIndication = MCPS_UNCONFIRMED;

    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    TimerStop( &RxWindowTimer2 );

    if( IsBeaconExpected( ) == true )
    {
        if( RxBeacon( payload, size ) == true )
        {
            return;
        }
    }
    if( IsPingExpected( ) == true )
    {
        PingSlotState = PINGSLOT_STATE_SET_TIMER;
        OnPingSlotTimerEvent( );
    }

    macHdr.Value = payload[pktHeaderLen++];

    switch( macHdr.Bits.MType )
    {
        case FRAME_TYPE_JOIN_ACCEPT:
            if( IsLoRaMacNetworkJoined == true )
            {
                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
                PrepareRxDoneAbort( );
                return;
            }
            LoRaMacJoinDecrypt( payload + 1, size - 1, LoRaMacAppKey, LoRaMacRxPayload + 1 );

            LoRaMacRxPayload[0] = macHdr.Value;

            LoRaMacJoinComputeMic( LoRaMacRxPayload, size - LORAMAC_MFR_LEN, LoRaMacAppKey, &mic );

            micRx |= ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN];
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 1] << 8 );
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 2] << 16 );
            micRx |= ( ( uint32_t )LoRaMacRxPayload[size - LORAMAC_MFR_LEN + 3] << 24 );

            index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_JOIN, MlmeConfirmQueueCnt );
            if( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
            {
                if( micRx == mic )
                {
                    LoRaMacJoinComputeSKeys( LoRaMacAppKey, LoRaMacRxPayload + 1, LoRaMacDevNonce, LoRaMacNwkSKey, LoRaMacAppSKey );

                    LoRaMacNetID = ( uint32_t )LoRaMacRxPayload[4];
                    LoRaMacNetID |= ( ( uint32_t )LoRaMacRxPayload[5] << 8 );
                    LoRaMacNetID |= ( ( uint32_t )LoRaMacRxPayload[6] << 16 );

                    LoRaMacDevAddr = ( uint32_t )LoRaMacRxPayload[7];
                    LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[8] << 8 );
                    LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[9] << 16 );
                    LoRaMacDevAddr |= ( ( uint32_t )LoRaMacRxPayload[10] << 24 );

                    // DLSettings
                    LoRaMacParams.Rx1DrOffset = ( LoRaMacRxPayload[11] >> 4 ) & 0x07;
                    LoRaMacParams.Rx2Channel.Datarate = LoRaMacRxPayload[11] & 0x0F;

                    // RxDelay
                    LoRaMacParams.ReceiveDelay1 = ( LoRaMacRxPayload[12] & 0x0F );
                    if( LoRaMacParams.ReceiveDelay1 == 0 )
                    {
                        LoRaMacParams.ReceiveDelay1 = 1;
                    }
                    LoRaMacParams.ReceiveDelay1 *= 1e3;
                    LoRaMacParams.ReceiveDelay2 = LoRaMacParams.ReceiveDelay1 + 1e3;

#if !( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
                    //CFList
                    if( ( size - 1 ) > 16 )
                    {
                        ChannelParams_t param;
                        param.DrRange.Value = ( DR_5 << 4 ) | DR_0;

                        LoRaMacState |= MAC_TX_CONFIG;
                        for( uint8_t i = 3, j = 0; i < ( 5 + 3 ); i++, j += 3 )
                        {
                            param.Frequency = ( ( uint32_t )LoRaMacRxPayload[13 + j] | ( ( uint32_t )LoRaMacRxPayload[14 + j] << 8 ) | ( ( uint32_t )LoRaMacRxPayload[15 + j] << 16 ) ) * 100;
                            LoRaMacChannelAdd( i, param );
                        }
                        LoRaMacState &= ~MAC_TX_CONFIG;
                    }
#endif

                    MlmeConfirmQueue[index].Status = LORAMAC_EVENT_INFO_STATUS_OK;

                    IsLoRaMacNetworkJoined = true;
                    LoRaMacParams.ChannelsDatarate = LoRaMacParamsDefaults.ChannelsDatarate;
                }
                else
                {
                    MlmeConfirmQueue[index].Status = LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL;
                }
            }
            break;
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
            {
                address = payload[pktHeaderLen++];
                address |= ( (uint32_t)payload[pktHeaderLen++] << 8 );
                address |= ( (uint32_t)payload[pktHeaderLen++] << 16 );
                address |= ( (uint32_t)payload[pktHeaderLen++] << 24 );

                fCtrl.Value = payload[pktHeaderLen++];

                if( address != LoRaMacDevAddr )
                {
                    curMulticastParams = MulticastChannels;
                    while( curMulticastParams != NULL )
                    {
                        if( address == curMulticastParams->Address )
                        {
                            multicast = 1;
                            nwkSKey = curMulticastParams->NwkSKey;
                            appSKey = curMulticastParams->AppSKey;
                            downLinkCounter = curMulticastParams->DownLinkCounter;
                            break;
                        }
                        curMulticastParams = curMulticastParams->Next;
                    }
                    if( multicast == 0 )
                    {
                        // We are not the destination of this frame.
                        McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL;
                        PrepareRxDoneAbort( );
                        return;
                    }
                    if( ( macHdr.Bits.MType != FRAME_TYPE_DATA_UNCONFIRMED_DOWN ) ||
                        ( fCtrl.Bits.Ack == 1 ) ||
                        ( fCtrl.Bits.AdrAckReq == 1 ) )
                    {
                        // Wrong multicast message format. Refer to chapter 11.2.2 of the specification
                        McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_MULTICAST_FAIL;
                        PrepareRxDoneAbort( );
                        return;
                    }
                }
                else
                {
                    multicast = 0;
                    nwkSKey = LoRaMacNwkSKey;
                    appSKey = LoRaMacAppSKey;
                    downLinkCounter = DownLinkCounter;
                }

                sequenceCounter = ( uint16_t )payload[pktHeaderLen++];
                sequenceCounter |= ( uint16_t )payload[pktHeaderLen++] << 8;

                appPayloadStartIndex = 8 + fCtrl.Bits.FOptsLen;

                micRx |= ( uint32_t )payload[size - LORAMAC_MFR_LEN];
                micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 1] << 8 );
                micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 2] << 16 );
                micRx |= ( ( uint32_t )payload[size - LORAMAC_MFR_LEN + 3] << 24 );

                sequenceCounterPrev = ( uint16_t )downLinkCounter;
                sequenceCounterDiff = ( sequenceCounter - sequenceCounterPrev );

                if( sequenceCounterDiff < ( 1 << 15 ) )
                {
                    downLinkCounter += sequenceCounterDiff;
                    LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounter, &mic );
                    if( micRx == mic )
                    {
                        isMicOk = true;
                    }
                }
                else
                {
                    // check for sequence roll-over
                    uint32_t  downLinkCounterTmp = downLinkCounter + 0x10000 + ( int16_t )sequenceCounterDiff;
                    LoRaMacComputeMic( payload, size - LORAMAC_MFR_LEN, nwkSKey, address, DOWN_LINK, downLinkCounterTmp, &mic );
                    if( micRx == mic )
                    {
                        isMicOk = true;
                        downLinkCounter = downLinkCounterTmp;
                    }
                }

                // Check for a the maximum allowed counter difference
                if( sequenceCounterDiff >= MAX_FCNT_GAP )
                {
                    McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS;
                    McpsIndication.DownLinkCounter = downLinkCounter;
                    PrepareRxDoneAbort( );
                    return;
                }

                if( isMicOk == true )
                {
                    McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                    McpsIndication.Multicast = multicast;
                    McpsIndication.FramePending = fCtrl.Bits.FPending;
                    McpsIndication.Buffer = NULL;
                    McpsIndication.BufferSize = 0;
                    McpsIndication.DownLinkCounter = downLinkCounter;

                    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;

                    AdrAckCounter = 0;
                    MacCommandsBufferToRepeatIndex = 0;

                    // Update 32 bits downlink counter
                    if( multicast == 1 )
                    {
                        McpsIndication.McpsIndication = MCPS_MULTICAST;

                        if( ( curMulticastParams->DownLinkCounter == downLinkCounter ) &&
                            ( curMulticastParams->DownLinkCounter != 0 ) )
                        {
                            McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED;
                            McpsIndication.DownLinkCounter = downLinkCounter;
                            PrepareRxDoneAbort( );
                            return;
                        }
                        curMulticastParams->DownLinkCounter = downLinkCounter;
                    }
                    else
                    {
                        if( macHdr.Bits.MType == FRAME_TYPE_DATA_CONFIRMED_DOWN )
                        {
                            SrvAckRequested = true;
                            McpsIndication.McpsIndication = MCPS_CONFIRMED;

                            if( ( DownLinkCounter == downLinkCounter ) &&
                                ( DownLinkCounter != 0 ) )
                            {
                                // Duplicated confirmed downlink. Skip indication.
                                skipIndication = true;
                            }
                        }
                        else
                        {
                            SrvAckRequested = false;
                            McpsIndication.McpsIndication = MCPS_UNCONFIRMED;

                            if( ( DownLinkCounter == downLinkCounter ) &&
                                ( DownLinkCounter != 0 ) )
                            {
                                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED;
                                McpsIndication.DownLinkCounter = downLinkCounter;
                                PrepareRxDoneAbort( );
                                return;
                            }
                        }
                        DownLinkCounter = downLinkCounter;
                    }

                    if( ( ( size - 4 ) - appPayloadStartIndex ) > 0 )
                    {
                        port = payload[appPayloadStartIndex++];
                        frameLen = ( size - 4 ) - appPayloadStartIndex;

                        McpsIndication.Port = port;

                        if( port == 0 )
                        {
                            if( ( fCtrl.Bits.FOptsLen == 0 ) && ( multicast == 0 ) )
                            {
                                LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                                       frameLen,
                                                       nwkSKey,
                                                       address,
                                                       DOWN_LINK,
                                                       downLinkCounter,
                                                       LoRaMacRxPayload );

                                // Decode frame payload MAC commands
                                ProcessMacCommands( LoRaMacRxPayload, 0, frameLen, snr );
                            }
                            else
                            {
                                skipIndication = true;
                            }
                        }
                        else
                        {
                            if( ( fCtrl.Bits.FOptsLen > 0 ) && ( multicast == 0 ) )
                            {
                                // Decode Options field MAC commands. Omit the fPort.
                                ProcessMacCommands( payload, 8, appPayloadStartIndex - 1, snr );
                            }

                            LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                                   frameLen,
                                                   appSKey,
                                                   address,
                                                   DOWN_LINK,
                                                   downLinkCounter,
                                                   LoRaMacRxPayload );

                            if( skipIndication == false )
                            {
                                McpsIndication.Buffer = LoRaMacRxPayload;
                                McpsIndication.BufferSize = frameLen;
                                McpsIndication.RxData = true;
                            }
                        }
                    }
                    else
                    {
                        if( fCtrl.Bits.FOptsLen > 0 )
                        {
                            // Decode Options field MAC commands
                            ProcessMacCommands( payload, 8, appPayloadStartIndex, snr );
                        }
                    }

                    if( skipIndication == false )
                    {
                        // Check if the frame is an acknowledgement
                        if( fCtrl.Bits.Ack == 1 )
                        {
                            McpsConfirm.AckReceived = true;
                            McpsIndication.AckReceived = true;

                            // Stop the AckTimeout timer as no more retransmissions
                            // are needed.
                            TimerStop( &AckTimeoutTimer );
                        }
                        else
                        {
                            McpsConfirm.AckReceived = false;

                            if( AckTimeoutRetriesCounter > AckTimeoutRetries )
                            {
                                // Stop the AckTimeout timer as no more retransmissions
                                // are needed.
                                TimerStop( &AckTimeoutTimer );
                            }
                        }
                        LoRaMacFlags.Bits.McpsInd = 1;
                    }
                }
                else
                {
                    McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_MIC_FAIL;

                    PrepareRxDoneAbort( );
                    return;
                }
            }
            break;
        case FRAME_TYPE_PROPRIETARY:
            {
                memcpy1( LoRaMacRxPayload, &payload[pktHeaderLen], size );

                McpsIndication.McpsIndication = MCPS_PROPRIETARY;
                McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                McpsIndication.Buffer = LoRaMacRxPayload;
                McpsIndication.BufferSize = size - pktHeaderLen;

                LoRaMacFlags.Bits.McpsInd = 1;
                break;
            }
        default:
            McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            PrepareRxDoneAbort( );
            break;
    }

    if( ( RxSlot == 0 ) && ( LoRaMacDeviceClass == CLASS_C ) )
    {
        OnRxWindow2TimerEvent( );
    }
    LoRaMacFlags.Bits.MacDone = 1;

    // Trig OnMacCheckTimerEvent call as soon as possible
    TimerSetValue( &MacStateCheckTimer, 1 );
    TimerStart( &MacStateCheckTimer );
}

static void OnRadioTxTimeout( void )
{
    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
    MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
    LoRaMacFlags.Bits.MacDone = 1;
}

static void OnRadioRxError( void )
{
    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    if( IsBeaconExpected( ) == true )
    {
        BeaconState = BEACON_STATE_TIMEOUT;
        OnBeaconTimerEvent( );
    }
    if( IsPingExpected( ) == true )
    {
        PingSlotState = PINGSLOT_STATE_SET_TIMER;
        OnPingSlotTimerEvent( );
    }
    else
    {
        if( RxSlot == 1 )
        {
            if( NodeAckRequested == true )
            {
                McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_ERROR;
            }
            MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_ERROR;
            LoRaMacFlags.Bits.MacDone = 1;
        }
    }
}

static void OnRadioRxTimeout( void )
{
    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    if( IsBeaconExpected( ) == true )
    {
        BeaconState = BEACON_STATE_TIMEOUT;
        OnBeaconTimerEvent( );
    }
    if( IsPingExpected( ) == true )
    {
        PingSlotState = PINGSLOT_STATE_SET_TIMER;
        OnPingSlotTimerEvent( );
    }
    else
    {
        if( RxSlot == 1 )
        {
            if( NodeAckRequested == true )
            {
                McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;
            }
            MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;
            LoRaMacFlags.Bits.MacDone = 1;
        }
    }
}

static void OnMacStateCheckTimerEvent( void )
{
    bool noTx = false;
    uint8_t index = 0;
    uint8_t i, j = 0;

    TimerStop( &MacStateCheckTimer );

    if( LoRaMacFlags.Bits.MacDone == 1 )
    {
        if( ( LoRaMacState & MAC_RX_ABORT ) == MAC_RX_ABORT )
        {
            LoRaMacState &= ~MAC_RX_ABORT;
            LoRaMacState &= ~MAC_TX_RUNNING;
        }

        if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) || ( ( LoRaMacFlags.Bits.McpsReq == 1 ) ) )
        {
            if( ( McpsConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) ||
                ( MlmeConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) )
            {
                // Stop transmit cycle due to tx timeout.
                LoRaMacState &= ~MAC_TX_RUNNING;
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                McpsConfirm.AckReceived = false;
                McpsConfirm.TxTimeOnAir = 0;
                noTx = true;
            }

            index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_BEACON_ACQUISITION, MlmeConfirmQueueCnt );
            if( ( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN ) && ( LoRaMacFlags.Bits.McpsReq == 0 ) )
            {
                if( LoRaMacFlags.Bits.MlmeReq == 1 )
                {
                    noTx = true;
                    LoRaMacState &= ~MAC_TX_RUNNING;
                }
            }
        }

        if( ( NodeAckRequested == false ) && ( noTx == false ) )
        {
            if( LoRaMacFlags.Bits.MlmeReq == 1 )
            {
                index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_JOIN, MlmeConfirmQueueCnt );
                if( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
                {
                    // Retransmit only if the answer is not OK
                    ChannelsNbRepCounter = 0;

                    if( MlmeConfirmQueue[index].Status == LORAMAC_EVENT_INFO_STATUS_OK )
                    {
                        // Stop retransmission
                        ChannelsNbRepCounter = LoRaMacParams.ChannelsNbRep;
                        UpLinkCounter = 0;
                    }
                }
            }
            if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) || ( ( LoRaMacFlags.Bits.McpsReq == 1 ) ) )
            {
                if( ( ChannelsNbRepCounter >= LoRaMacParams.ChannelsNbRep ) || ( LoRaMacFlags.Bits.McpsInd == 1 ) )
                {
                    ChannelsNbRepCounter = 0;

                    AdrAckCounter++;
                    if( IsUpLinkCounterFixed == false )
                    {
                        UpLinkCounter++;
                    }

                    LoRaMacState &= ~MAC_TX_RUNNING;
                }
                else
                {
                    LoRaMacFlags.Bits.MacDone = 0;
                    // Sends the same frame again
                    OnTxDelayedTimerEvent( );
                }
            }
        }

        if( LoRaMacFlags.Bits.McpsInd == 1 )
        {
            if( ( McpsConfirm.AckReceived == true ) || ( AckTimeoutRetriesCounter > AckTimeoutRetries ) )
            {
                AckTimeoutRetry = false;
                NodeAckRequested = false;
                if( IsUpLinkCounterFixed == false )
                {
                    UpLinkCounter++;
                }
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;

                LoRaMacState &= ~MAC_TX_RUNNING;
            }
        }

        if( ( AckTimeoutRetry == true ) && ( ( LoRaMacState & MAC_TX_DELAYED ) == 0 ) )
        {
            AckTimeoutRetry = false;
            if( ( AckTimeoutRetriesCounter < AckTimeoutRetries ) && ( AckTimeoutRetriesCounter <= MAX_ACK_RETRIES ) )
            {
                AckTimeoutRetriesCounter++;

                if( ( AckTimeoutRetriesCounter % 2 ) == 1 )
                {
                    LoRaMacParams.ChannelsDatarate = MAX( LoRaMacParams.ChannelsDatarate - 1, LORAMAC_TX_MIN_DATARATE );
                }
                if( ValidatePayloadLength( LoRaMacTxPayloadLen, LoRaMacParams.ChannelsDatarate, MacCommandsBufferIndex ) == true )
                {
                    LoRaMacFlags.Bits.MacDone = 0;
                    // Sends the same frame again
                    ScheduleTx( );
                }
                else
                {
                    // The DR is not applicable for the payload size
                    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR;

                    LoRaMacState &= ~MAC_TX_RUNNING;
                    NodeAckRequested = false;
                    McpsConfirm.AckReceived = false;
                    McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                    McpsConfirm.Datarate = LoRaMacParams.ChannelsDatarate;
                    if( IsUpLinkCounterFixed == false )
                    {
                        UpLinkCounter++;
                    }
                }
            }
            else
            {
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
                // Re-enable default channels LC1, LC2, LC3
                LoRaMacParams.ChannelsMask[0] = LoRaMacParams.ChannelsMask[0] | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );
#elif defined( USE_BAND_470 )
                // Re-enable default channels
                LoRaMacParams.ChannelsMask[0] = 0xFFFF;
                LoRaMacParams.ChannelsMask[1] = 0xFFFF;
                LoRaMacParams.ChannelsMask[2] = 0xFFFF;
                LoRaMacParams.ChannelsMask[3] = 0xFFFF;
                LoRaMacParams.ChannelsMask[4] = 0xFFFF;
                LoRaMacParams.ChannelsMask[5] = 0xFFFF;
#elif defined( USE_BAND_915 )
                // Re-enable default channels
                LoRaMacParams.ChannelsMask[0] = 0xFFFF;
                LoRaMacParams.ChannelsMask[1] = 0xFFFF;
                LoRaMacParams.ChannelsMask[2] = 0xFFFF;
                LoRaMacParams.ChannelsMask[3] = 0xFFFF;
                LoRaMacParams.ChannelsMask[4] = 0x00FF;
                LoRaMacParams.ChannelsMask[5] = 0x0000;
#elif defined( USE_BAND_915_HYBRID )
                // Re-enable default channels
                ReenableChannels( LoRaMacParams.ChannelsMask[4], LoRaMacParams.ChannelsMask );
#else
    #error "Please define a frequency band in the compiler options."
#endif
                LoRaMacState &= ~MAC_TX_RUNNING;

                NodeAckRequested = false;
                McpsConfirm.AckReceived = false;
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;
                if( IsUpLinkCounterFixed == false )
                {
                    UpLinkCounter++;
                }
            }
        }
    }
    // Handle reception for Class B and Class C
    if( ( LoRaMacState & MAC_RX ) == MAC_RX )
    {
        LoRaMacState &= ~MAC_RX;
    }
    if( LoRaMacState == MAC_IDLE )
    {
        if( LoRaMacFlags.Bits.McpsReq == 1 )
        {
            LoRaMacPrimitives->MacMcpsConfirm( &McpsConfirm );
            LoRaMacFlags.Bits.McpsReq = 0;
        }

        if( LoRaMacFlags.Bits.MlmeReq == 1 )
        {
            j = MlmeConfirmQueueCnt;
            for( i = 0; i < MlmeConfirmQueueCnt; i++ )
            {
                if( MlmeConfirmQueue[i].MlmeRequest == MLME_BEACON_ACQUISITION )
                {
                    if( BeaconCtx.Ctrl.AcquisitionPending == 1 )
                    {
                        MlmeConfirmQueue[0].MlmeRequest = MLME_BEACON_ACQUISITION;
                        MlmeConfirmQueue[0].Status = MlmeConfirmQueue[i].Status;
                        continue;
                    }
                }
                j--;
                MlmeConfirm.Status = MlmeConfirmQueue[i].Status;
                MlmeConfirm.MlmeRequest = MlmeConfirmQueue[i].MlmeRequest;
                LoRaMacPrimitives->MacMlmeConfirm( &MlmeConfirm );
            }
            MlmeConfirmQueueCnt = j;

            if( MlmeConfirmQueueCnt == 0 )
            {
                LoRaMacFlags.Bits.MlmeReq = 0;
            }
        }

        if( LoRaMacFlags.Bits.MlmeInd == 1 )
        {
            LoRaMacPrimitives->MacMlmeIndication( &MlmeIndication );
            LoRaMacFlags.Bits.MlmeInd = 0;
        }
        LoRaMacFlags.Bits.MacDone = 0;

        ResumeBeaconing( );
    }
    else
    {
        // Operation not finished restart timer
        TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
        TimerStart( &MacStateCheckTimer );
    }

    if( LoRaMacFlags.Bits.McpsInd == 1 )
    {
        LoRaMacPrimitives->MacMcpsIndication( &McpsIndication );
        LoRaMacFlags.Bits.McpsInd = 0;
    }
}

static void OnTxDelayedTimerEvent( void )
{
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;
    uint8_t index = 0;

    TimerStop( &TxDelayedTimer );
    LoRaMacState &= ~MAC_TX_DELAYED;

    index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_JOIN, MlmeConfirmQueueCnt );

    if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN ) )
    {
        ResetMacParameters( );
        JoinRequestTrials++;
        LoRaMacParams.ChannelsDatarate = AlternateDatarate( JoinRequestTrials );

        macHdr.Value = 0;
        macHdr.Bits.MType = FRAME_TYPE_JOIN_REQ;

        fCtrl.Value = 0;
        fCtrl.Bits.Adr = AdrCtrlOn;

        /* In case of a join request retransmission, the stack must prepare
         * the frame again, because the network server keeps track of the random
         * LoRaMacDevNonce values to prevent reply attacks. */
        PrepareFrame( &macHdr, &fCtrl, 0, NULL, 0 );
    }

    ScheduleTx( );
}

static void OnRxWindow1TimerEvent( void )
{
    int8_t datarate = 0;

    TimerStop( &RxWindowTimer1 );
    RxSlot = 0;

    if( LoRaMacDeviceClass == CLASS_C )
    {
        Radio.Standby( );
    }

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    datarate = LoRaMacParams.ChannelsDatarate - LoRaMacParams.Rx1DrOffset;
    if( datarate < 0 )
    {
        datarate = DR_0;
    }

    RxWindowSetup( Channels[Channel].Frequency, datarate, GetRxBandwidth( datarate ),
                   GetRxSymbolTimeout( datarate ), false );
#elif defined( USE_BAND_470 )
    datarate = LoRaMacParams.ChannelsDatarate - LoRaMacParams.Rx1DrOffset;
    if( datarate < 0 )
    {
        datarate = DR_0;
    }

    RxWindowSetup( LORAMAC_FIRST_RX1_CHANNEL + ( Channel % 48 ) * LORAMAC_STEPWIDTH_RX1_CHANNEL, datarate, GetRxBandwidth( datarate ),
                   GetRxSymbolTimeout( datarate ), false );
#elif ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
    datarate = datarateOffsets[LoRaMacParams.ChannelsDatarate][LoRaMacParams.Rx1DrOffset];
    if( datarate < 0 )
    {
        datarate = DR_0;
    }

    RxWindowSetup( LORAMAC_FIRST_RX1_CHANNEL + ( Channel % 8 ) * LORAMAC_STEPWIDTH_RX1_CHANNEL, datarate, GetRxBandwidth( datarate ),
                   GetRxSymbolTimeout( datarate ), false );
#endif
}

static void OnRxWindow2TimerEvent( void )
{
    TimerStop( &RxWindowTimer2 );
    RxSlot = 1;

    if( LoRaMacDeviceClass != CLASS_C )
    {
        RxWindowSetup( LoRaMacParams.Rx2Channel.Frequency, LoRaMacParams.Rx2Channel.Datarate, GetRxBandwidth( LoRaMacParams.Rx2Channel.Datarate ),
                       GetRxSymbolTimeout( LoRaMacParams.Rx2Channel.Datarate ), false );
    }
    else
    {
        RxWindowSetup( LoRaMacParams.Rx2Channel.Frequency, LoRaMacParams.Rx2Channel.Datarate, GetRxBandwidth( LoRaMacParams.Rx2Channel.Datarate ),
                       GetRxSymbolTimeout( LoRaMacParams.Rx2Channel.Datarate ), true );
    }
}

static void OnAckTimeoutTimerEvent( void )
{
    TimerStop( &AckTimeoutTimer );

    if( NodeAckRequested == true )
    {
        AckTimeoutRetry = true;
        LoRaMacState &= ~MAC_ACK_REQ;
    }
    if( LoRaMacDeviceClass == CLASS_C )
    {
        LoRaMacFlags.Bits.MacDone = 1;
    }
}

static bool SetNextChannel( TimerTime_t* time )
{
    uint8_t nbEnabledChannels = 0;
    uint8_t delayTx = 0;
    uint8_t enabledChannels[LORA_MAX_NB_CHANNELS];
    TimerTime_t nextTxDelay = ( TimerTime_t )( -1 );

    memset1( enabledChannels, 0, LORA_MAX_NB_CHANNELS );

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    if( CountNbEnabled125kHzChannels( ChannelsMaskRemaining ) == 0 )
    { // Restore default channels
        memcpy1( ( uint8_t* ) ChannelsMaskRemaining, ( uint8_t* ) LoRaMacParams.ChannelsMask, 8 );
    }
    if( ( LoRaMacParams.ChannelsDatarate >= DR_4 ) && ( ( ChannelsMaskRemaining[4] & 0x00FF ) == 0 ) )
    { // Make sure, that the channels are activated
        ChannelsMaskRemaining[4] = LoRaMacParams.ChannelsMask[4];
    }
#elif defined( USE_BAND_470 )
    if( ( CountBits( LoRaMacParams.ChannelsMask[0], 16 ) == 0 ) &&
        ( CountBits( LoRaMacParams.ChannelsMask[1], 16 ) == 0 ) &&
        ( CountBits( LoRaMacParams.ChannelsMask[2], 16 ) == 0 ) &&
        ( CountBits( LoRaMacParams.ChannelsMask[3], 16 ) == 0 ) &&
        ( CountBits( LoRaMacParams.ChannelsMask[4], 16 ) == 0 ) &&
        ( CountBits( LoRaMacParams.ChannelsMask[5], 16 ) == 0 ) )
    {
        LoRaMacParams.ChannelsMask[0] = LoRaMacParamsDefaults.ChannelsMask[0];
        LoRaMacParams.ChannelsMask[1] = LoRaMacParamsDefaults.ChannelsMask[1];
        LoRaMacParams.ChannelsMask[2] = LoRaMacParamsDefaults.ChannelsMask[2];
        LoRaMacParams.ChannelsMask[3] = LoRaMacParamsDefaults.ChannelsMask[3];
        LoRaMacParams.ChannelsMask[4] = LoRaMacParamsDefaults.ChannelsMask[4];
        LoRaMacParams.ChannelsMask[5] = LoRaMacParamsDefaults.ChannelsMask[5];
    }
#else
    if( CountBits( LoRaMacParams.ChannelsMask[0], 16 ) == 0 )
    {
        // Re-enable default channels, if no channel is enabled
        LoRaMacParams.ChannelsMask[0] = LoRaMacParams.ChannelsMask[0] | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );
    }
#endif

    // Update Aggregated duty cycle
    if( AggregatedTimeOff <= TimerGetElapsedTime( AggregatedLastTxDoneTime ) )
    {
        AggregatedTimeOff = 0;

        // Update bands Time OFF
        for( uint8_t i = 0; i < LORA_MAX_NB_BANDS; i++ )
        {
            if( DutyCycleOn == true )
            {
                if( Bands[i].TimeOff <= TimerGetElapsedTime( Bands[i].LastTxDoneTime ) )
                {
                    Bands[i].TimeOff = 0;
                }
                if( Bands[i].TimeOff != 0 )
                {
                    nextTxDelay = MIN( Bands[i].TimeOff -
                                       TimerGetElapsedTime( Bands[i].LastTxDoneTime ),
                                       nextTxDelay );
                }
            }
            else
            {
                nextTxDelay = 0;
                Bands[i].TimeOff = 0;
            }
        }

        // Search how many channels are enabled
        for( uint8_t i = 0, k = 0; i < LORA_MAX_NB_CHANNELS; i += 16, k++ )
        {
            for( uint8_t j = 0; j < 16; j++ )
            {
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                if( ( ChannelsMaskRemaining[k] & ( 1 << j ) ) != 0 )
#else
                if( ( LoRaMacParams.ChannelsMask[k] & ( 1 << j ) ) != 0 )
#endif
                {
                    if( Channels[i + j].Frequency == 0 )
                    { // Check if the channel is enabled
                        continue;
                    }
#if defined( USE_BAND_868 ) || defined( USE_BAND_433 ) || defined( USE_BAND_780 )
                    if( IsLoRaMacNetworkJoined == false )
                    {
                        if( ( JOIN_CHANNELS & ( 1 << j ) ) == 0 )
                        {
                            continue;
                        }
                    }
#endif
                    if( ( ( Channels[i + j].DrRange.Fields.Min <= LoRaMacParams.ChannelsDatarate ) &&
                          ( LoRaMacParams.ChannelsDatarate <= Channels[i + j].DrRange.Fields.Max ) ) == false )
                    { // Check if the current channel selection supports the given datarate
                        continue;
                    }
                    if( Bands[Channels[i + j].Band].TimeOff > 0 )
                    { // Check if the band is available for transmission
                        delayTx++;
                        continue;
                    }
                    enabledChannels[nbEnabledChannels++] = i + j;
                }
            }
        }
    }
    else
    {
        delayTx++;
        nextTxDelay = AggregatedTimeOff - TimerGetElapsedTime( AggregatedLastTxDoneTime );
    }

    if( nbEnabledChannels > 0 )
    {
        Channel = enabledChannels[randr( 0, nbEnabledChannels - 1 )];
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
        if( Channel < ( LORA_MAX_NB_CHANNELS - 8 ) )
        {
            DisableChannelInMask( Channel, ChannelsMaskRemaining );
        }
#endif
        *time = 0;
        return true;
    }
    else
    {
        if( delayTx > 0 )
        {
            // Delay transmission due to AggregatedTimeOff or to a band time off
            *time = nextTxDelay;
            return true;
        }
        // Datarate not supported by any channel
        *time = 0;
        return false;
    }
}

static void SetPublicNetwork( bool enable )
{
    PublicNetwork = enable;
    Radio.SetModem( MODEM_LORA );
    if( PublicNetwork == true )
    {
        // Change LoRa modem SyncWord
        Radio.Write( REG_LR_SYNCWORD, LORA_MAC_PUBLIC_SYNCWORD );
    }
    else
    {
        // Change LoRa modem SyncWord
        Radio.Write( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
    }
}

static uint16_t GetRxSymbolTimeout( int8_t datarate )
{
#if ( defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 ) )
    if( ( datarate == DR_3 ) || ( datarate == DR_4 ) )
    { // DR_4, DR_3
        return 8;
    }
    else if( datarate == DR_5 )
    {
        return 10;
    }
    else if( datarate == DR_6 )
    {
        return 14;
    }
    return 5; // DR_2, DR_1, DR_0
#elif defined( USE_BAND_470 )
    if( ( datarate == DR_3 ) || ( datarate == DR_4 ) )
    { // DR_4, DR_3
        return 8;
    }
    else if( datarate == DR_5 )
    {
        return 10;
    }
    return 5; // DR_2, DR_1, DR_0
#elif ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
    switch( datarate )
    {
        case DR_0:       // SF10 - BW125
            return 5;

        case DR_1:       // SF9  - BW125
        case DR_2:       // SF8  - BW125
        case DR_8:       // SF12 - BW500
        case DR_9:       // SF11 - BW500
        case DR_10:      // SF10 - BW500
            return 8;

        case DR_3:       // SF7  - BW125
        case DR_11:     // SF9  - BW500
            return 10;

        case DR_4:       // SF8  - BW500
        case DR_12:      // SF8  - BW500
            return 14;

        case DR_13:      // SF7  - BW500
            return 16;

        default:
            return 0;   // LoRa 125 kHz
    }
#endif
}

static uint32_t GetRxBandwidth( int8_t datarate )
{
#if ( defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 ) )
    if( datarate == DR_6 )
    {// LoRa 250 kHz
        return 1;
    }
    return 0; // LoRa 125 kHz
#elif defined( USE_BAND_470 )
    return 0; // LoRa 125 kHz
#elif ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
    if( datarate >= DR_4 )
    {// LoRa 500 kHz
        return 2;
    }
    return 0; // LoRa 125 kHz
#endif
}

static void RxWindowSetup( uint32_t freq, int8_t datarate, uint32_t bandwidth, uint16_t timeout, bool rxContinuous )
{
    uint8_t downlinkDatarate = Datarates[datarate];
    RadioModems_t modem;

    if( Radio.GetStatus( ) == RF_IDLE )
    {
        Radio.SetChannel( freq );

        // Store downlink datarate
        McpsIndication.RxDatarate = ( uint8_t ) datarate;

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
        if( datarate == DR_7 )
        {
            modem = MODEM_FSK;
            Radio.SetRxConfig( modem, 50e3, downlinkDatarate * 1e3, 0, 83.333e3, 5, 0, false, 0, true, 0, 0, false, rxContinuous );
        }
        else
        {
            modem = MODEM_LORA;
            Radio.SetRxConfig( modem, bandwidth, downlinkDatarate, 1, 0, 8, timeout, false, 0, false, 0, 0, true, rxContinuous );
        }
#elif defined( USE_BAND_470 ) || defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
        modem = MODEM_LORA;
        Radio.SetRxConfig( modem, bandwidth, downlinkDatarate, 1, 0, 8, timeout, false, 0, false, 0, 0, true, rxContinuous );
#endif

        if( RepeaterSupport == true )
        {
            Radio.SetMaxPayloadLength( modem, MaxPayloadOfDatarateRepeater[datarate] + LORA_MAC_FRMPAYLOAD_OVERHEAD );
        }
        else
        {
            Radio.SetMaxPayloadLength( modem, MaxPayloadOfDatarate[datarate] + LORA_MAC_FRMPAYLOAD_OVERHEAD );
        }

        if( rxContinuous == false )
        {
            Radio.Rx( LoRaMacParams.MaxRxWindow );
        }
        else
        {
            Radio.Rx( 0 ); // Continuous mode
        }
    }
}

static bool Rx2FreqInRange( uint32_t freq )
{
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    if( Radio.CheckRfFrequency( freq ) == true )
#elif defined( USE_BAND_470 ) || defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    if( ( Radio.CheckRfFrequency( freq ) == true ) &&
        ( freq >= LORAMAC_FIRST_RX1_CHANNEL ) &&
        ( freq <= LORAMAC_LAST_RX1_CHANNEL ) &&
        ( ( ( freq - ( uint32_t ) LORAMAC_FIRST_RX1_CHANNEL ) % ( uint32_t ) LORAMAC_STEPWIDTH_RX1_CHANNEL ) == 0 ) )
#endif
    {
        return true;
    }
    return false;
}

static LoRaMacStatus_t SwitchClass( DeviceClass_t deviceClass )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_PARAMETER_INVALID;
    MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;

    switch( LoRaMacDeviceClass )
    {
        case CLASS_A:
        {
            if( deviceClass == CLASS_B )
            {
                if( PingSlotCtx.Ctrl.Assigned == 1 )
                {
                    LoRaMacDeviceClass = deviceClass;

                    BeaconState = BEACON_STATE_ACQUISITION;
                    LoRaMacState |= MAC_TX_RUNNING;

                    // Default temperature
                    BeaconCtx.Temperature = 25.0;
                    // Measure temperature
                    if( ( LoRaMacCallbacks != NULL ) && ( LoRaMacCallbacks->GetTemperatureLevel != NULL ) )
                    {
                        BeaconCtx.Temperature = LoRaMacCallbacks->GetTemperatureLevel( );
                    }

                    // Start class B algorithm
                    OnBeaconTimerEvent( );

                    status = LORAMAC_STATUS_OK;
                }
            }

            if( deviceClass == CLASS_C )
            {
                LoRaMacDeviceClass = deviceClass;

                // Set the NodeAckRequested indicator to default
                NodeAckRequested = false;
                OnRxWindow2TimerEvent( );

                TimerStart( &MacStateCheckTimer );

                status = LORAMAC_STATUS_OK;
            }
            break;
        }
        case CLASS_B:
        {
            if( deviceClass == CLASS_A )
            {
                BeaconState = BEACON_STATE_SWITCH_CLASS;
                // Start class B algorithm
                OnBeaconTimerEvent( );

                LoRaMacDeviceClass = deviceClass;

                status = LORAMAC_STATUS_OK;
            }
            break;
        }
        case CLASS_C:
        {
            if( deviceClass == CLASS_A )
            {
                LoRaMacDeviceClass = deviceClass;

                // Set the radio into sleep to setup a defined state
                Radio.Sleep( );

                status = LORAMAC_STATUS_OK;
            }
            break;
        }
    }

    if( status == LORAMAC_STATUS_OK )
    {
        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
    }

    return status;
}

static void OnBeaconTimerEvent( void )
{
    uint8_t index = 0;
    bool activateTimer = false;
    TimerTime_t beaconEventTime = 1;
    TimerTime_t currentTime = TimerGetCurrentTime( );

    TimerStop( &BeaconTimer );

    switch( BeaconState )
    {
        case BEACON_STATE_ACQUISITION:
        {
            activateTimer = true;

            if( BeaconCtx.Ctrl.AcquisitionPending == 1 )
            {
                BeaconCtx.Ctrl.AcquisitionPending = 0;
                BeaconState = BEACON_STATE_SWITCH_CLASS;
            }
            else
            {
                // Default symbol timeouts
                BeaconCtx.SymbolTimeout = BeaconCtx.Cfg.SymbolToDefault;
                PingSlotCtx.SymbolTimeout = BeaconCtx.Cfg.SymbolToDefault;

                if( BeaconCtx.Ctrl.BeaconDelaySet == 1 )
                {
                    BeaconCtx.Ctrl.BeaconDelaySet = 0;

                    if( BeaconCtx.BeaconTimingDelay > 0 )
                    {
                        beaconEventTime = BeaconCtx.BeaconTimingDelay;
                        beaconEventTime = TimerTempCompensation( beaconEventTime, BeaconCtx.Temperature );
                    }
                    else
                    {
                        BeaconCtx.Ctrl.AcquisitionPending = 1;
                        beaconEventTime = BeaconCtx.Cfg.Reserved;
                        RxBeaconSetup( BeaconCtx.SymbolTimeout, 0 );
                    }
                }
                else
                {
                    BeaconCtx.Ctrl.AcquisitionPending = 1;
                    beaconEventTime = BeaconCtx.Cfg.Interval;
                    RxBeaconSetup( BeaconCtx.SymbolTimeout, 0 );
                }
            }
            break;
        }
        case BEACON_STATE_TIMEOUT:
        {
            // Store listen time
            BeaconCtx.ListenTime = currentTime - BeaconCtx.NextBeaconRx;

            // Update symbol timeout
            BeaconCtx.SymbolTimeout *= BeaconCtx.Cfg.SymbolToExpansionFactor;
            if( BeaconCtx.SymbolTimeout > BeaconCtx.Cfg.SymbolToExpansionMax )
            {
                BeaconCtx.SymbolTimeout = BeaconCtx.Cfg.SymbolToExpansionMax;
            }
            PingSlotCtx.SymbolTimeout *= PingSlotCtx.Cfg.SymbolToExpansionFactor;
            if( PingSlotCtx.SymbolTimeout > PingSlotCtx.Cfg.SymbolToExpansionMax )
            {
                PingSlotCtx.SymbolTimeout = PingSlotCtx.Cfg.SymbolToExpansionMax;
            }
            // We have to update the beacon time, since we missed a beacon
            BeaconCtx.BeaconTime += ( BeaconCtx.Cfg.Interval / 1000 );
            // Setup next state
            BeaconState = BEACON_STATE_REACQUISITION;
            // no break here
        }
        case BEACON_STATE_REACQUISITION:
        {
            if( ( currentTime - BeaconCtx.LastBeaconRx ) > MAX_BEACON_LESS_PERIOD )
            {
                activateTimer = true;
                BeaconState = BEACON_STATE_SWITCH_CLASS;
            }
            else
            {
                activateTimer = true;
                // Calculate the point in time of the next beacon
                beaconEventTime = ( ( currentTime - BeaconCtx.LastBeaconRx ) % BeaconCtx.Cfg.Interval );
                beaconEventTime = BeaconCtx.Cfg.Interval - beaconEventTime;
                // Take window enlargement into account
                beaconEventTime -= ( ( BeaconCtx.ListenTime * BeaconCtx.Cfg.SymbolToExpansionFactor ) >> 1 );
                beaconEventTime = TimerTempCompensation( beaconEventTime, BeaconCtx.Temperature );
                BeaconCtx.NextBeaconRx = currentTime + beaconEventTime;

                // Make sure to transit to the correct state
                if( ( currentTime + BeaconCtx.Cfg.Guard ) < BeaconCtx.NextBeaconRx )
                {
                    beaconEventTime -= BeaconCtx.Cfg.Guard;
                    BeaconState = BEACON_STATE_IDLE;
                }
                else
                {
                    BeaconState = BEACON_STATE_GUARD;
                }

                PingSlotState = PINGSLOT_STATE_CALC_PING_OFFSET;
                TimerSetValue( &PingSlotTimer, 1 );
                TimerStart( &PingSlotTimer );

                MulticastSlotState = PINGSLOT_STATE_CALC_PING_OFFSET;
                TimerSetValue( &MulticastSlotTimer, 1 );
                TimerStart( &MulticastSlotTimer );

                // Start the timer only if the MAC does not switch the class.
                TimerSetValue( &MacStateCheckTimer, 1 );
                TimerStart( &MacStateCheckTimer );
            }
            BeaconCtx.Ctrl.BeaconLess = 1;

            MlmeIndication.MlmeIndication = MLME_BEACON;
            MlmeIndication.Status = LORAMAC_EVENT_INFO_STATUS_BEACON_LOST;
            LoRaMacFlags.Bits.MlmeInd = 1;
            break;
        }
        case BEACON_STATE_LOCKED:
        {
            BeaconCtx.Ctrl.AcquisitionPending = 0;

            activateTimer = true;
            // Calculate the point in time of the next beacon
            beaconEventTime = ( ( currentTime - BeaconCtx.LastBeaconRx ) % BeaconCtx.Cfg.Interval );
            beaconEventTime = BeaconCtx.Cfg.Interval - beaconEventTime;
            beaconEventTime = TimerTempCompensation( beaconEventTime, BeaconCtx.Temperature );
            BeaconCtx.NextBeaconRx = currentTime + beaconEventTime;

            // Make sure to transit to the correct state
            if( ( currentTime + BeaconCtx.Cfg.Guard ) < BeaconCtx.NextBeaconRx )
            {
                beaconEventTime -= BeaconCtx.Cfg.Guard;
                BeaconState = BEACON_STATE_IDLE;
            }
            else
            {
                BeaconState = BEACON_STATE_GUARD;
            }

            if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( MlmeConfirm.MlmeRequest == MLME_SWITCH_CLASS ) )
            {
                index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_BEACON_ACQUISITION, MlmeConfirmQueueCnt );
                if( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
                {
                    MlmeConfirmQueue[index].Status = LORAMAC_EVENT_INFO_STATUS_OK;
                    MlmeConfirm.TxTimeOnAir = 0;
                }
                LoRaMacFlags.Bits.MacDone = 1;
            }
            MlmeIndication.MlmeIndication = MLME_BEACON;
            MlmeIndication.Status = LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED;
            LoRaMacFlags.Bits.MlmeInd = 1;

            PingSlotState = PINGSLOT_STATE_CALC_PING_OFFSET;
            TimerSetValue( &PingSlotTimer, 1 );
            TimerStart( &PingSlotTimer );

            MulticastSlotState = PINGSLOT_STATE_CALC_PING_OFFSET;
            TimerSetValue( &MulticastSlotTimer, 1 );
            TimerStart( &MulticastSlotTimer );

            TimerSetValue( &MacStateCheckTimer, 1 );
            TimerStart( &MacStateCheckTimer );
            break;
        }
        case BEACON_STATE_IDLE:
        {
            activateTimer = true;
            if( ( LoRaMacCallbacks != NULL ) && ( LoRaMacCallbacks->GetTemperatureLevel != NULL ) )
            {
                BeaconCtx.Temperature = LoRaMacCallbacks->GetTemperatureLevel( );
            }
            beaconEventTime = BeaconCtx.NextBeaconRx - RADIO_WAKEUP_TIME;

            if( beaconEventTime > currentTime )
            {
                BeaconState = BEACON_STATE_GUARD;
                beaconEventTime -= currentTime;
                beaconEventTime = TimerTempCompensation( beaconEventTime, BeaconCtx.Temperature );
            }
            else
            {
                BeaconState = BEACON_STATE_REACQUISITION;
                beaconEventTime = 1;
            }
            break;
        }
        case BEACON_STATE_GUARD:
        {
            BeaconState = BEACON_STATE_RX;
            RxBeaconSetup( BeaconCtx.SymbolTimeout, BeaconCtx.Cfg.Reserved );
            break;
        }
        case BEACON_STATE_SWITCH_CLASS:
        {
            LoRaMacDeviceClass = CLASS_A;

            PingSlotCtx.Ctrl.Assigned = 0;

            if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( MlmeConfirm.MlmeRequest == MLME_SWITCH_CLASS ) )
            {
                index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_BEACON_ACQUISITION, MlmeConfirmQueueCnt );
                if( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
                {
                    MlmeConfirmQueue[index].Status = LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND;
                }
                LoRaMacFlags.Bits.MacDone = 1;
            }
            else
            {
                MlmeIndication.MlmeIndication = MLME_SWITCH_CLASS;
                MlmeIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                LoRaMacFlags.Bits.MlmeInd = 1;
            }
            BeaconState = BEACON_STATE_ACQUISITION;

            TimerSetValue( &MacStateCheckTimer, beaconEventTime );
            TimerStart( &MacStateCheckTimer );
            break;
        }
        default:
        {
            BeaconState = BEACON_STATE_ACQUISITION;
            break;
        }
    }

    if( activateTimer == true )
    {
        TimerSetValue( &BeaconTimer, beaconEventTime );
        TimerStart( &BeaconTimer );
    }
}

static bool CalcNextSlotTime( uint16_t slotOffset, uint16_t pingPeriod, TimerTime_t* timeOffset )
{
    uint8_t currentPingSlot = 0;
    TimerTime_t slotTime = 0;
    TimerTime_t currentTime = TimerGetCurrentTime( );

    // Calculate the point in time of the last beacon even if we missed it
    slotTime = ( ( currentTime - BeaconCtx.LastBeaconRx ) % BeaconCtx.Cfg.Interval );
    slotTime = currentTime - slotTime;

    // Add the reserved time and the ping offset
    slotTime += BeaconCtx.Cfg.Reserved;
    slotTime += slotOffset * PingSlotCtx.Cfg.PingSlotWindow;

    if( slotTime < currentTime )
    {
        currentPingSlot = ( ( currentTime - slotTime ) /
                          ( pingPeriod * PingSlotCtx.Cfg.PingSlotWindow ) ) + 1;
        slotTime += ( ( TimerTime_t )( currentPingSlot * pingPeriod ) *
                    PingSlotCtx.Cfg.PingSlotWindow );
    }

    if( currentPingSlot < PingSlotCtx.PingNb )
    {
        if( slotTime <= ( BeaconCtx.NextBeaconRx - BeaconCtx.Cfg.Guard - PingSlotCtx.Cfg.PingSlotWindow ) )
        {
            // Calculate the relative ping slot time
            slotTime -= currentTime;
            slotTime -= RADIO_WAKEUP_TIME;
            slotTime = TimerTempCompensation( slotTime, BeaconCtx.Temperature );
            *timeOffset = slotTime;
            return true;
        }
    }
    return false;
}

static void OnPingSlotTimerEvent( void )
{
    TimerTime_t pingSlotTime = 0;

    TimerStop( &PingSlotTimer );

    switch( PingSlotState )
    {
        case PINGSLOT_STATE_CALC_PING_OFFSET:
        {
            LoRaMacBeaconComputePingOffset( BeaconCtx.BeaconTime,
                                            LoRaMacDevAddr,
                                            PingSlotCtx.PingPeriod,
                                            &( PingSlotCtx.PingOffset ) );
            PingSlotState = PINGSLOT_STATE_SET_TIMER;
            // no break
        }
        case PINGSLOT_STATE_SET_TIMER:
        {
            if( CalcNextSlotTime( PingSlotCtx.PingOffset, PingSlotCtx.PingPeriod, &pingSlotTime ) == true )
            {
                // Start the timer if the ping slot time is in range
                PingSlotState = PINGSLOT_STATE_IDLE;
                TimerSetValue( &PingSlotTimer, pingSlotTime );
                TimerStart( &PingSlotTimer );
            }
            break;
        }
        case PINGSLOT_STATE_IDLE:
        {
            uint32_t frequency = PingSlotCtx.Frequency;

            if( PingSlotCtx.Ctrl.CustomFreq == 0 )
            {
                // Restore floor plan
                frequency = PINGSLOT_CHANNEL_FREQ( LoRaMacDevAddr );
            }

            if( MulticastSlotState != PINGSLOT_STATE_RX )
            {
                if( BeaconCtx.Ctrl.BeaconLess == false )
                {
                    PingSlotCtx.SymbolTimeout = GetRxSymbolTimeout( PingSlotCtx.Datarate );
                }
                PingSlotState = PINGSLOT_STATE_RX;
                RxWindowSetup( frequency, PingSlotCtx.Datarate, GetRxBandwidth( PingSlotCtx.Datarate ),
                               PingSlotCtx.SymbolTimeout, false );

            }
            else
            {
                // Multicast slots have priority. Skip Rx
                PingSlotState = PINGSLOT_STATE_SET_TIMER;
                TimerSetValue( &PingSlotTimer, PingSlotCtx.Cfg.PingSlotWindow );
                TimerStart( &PingSlotTimer );
            }
            break;
        }
        default:
        {
            PingSlotState = PINGSLOT_STATE_SET_TIMER;
            break;
        }
    }
}

static void OnMulticastSlotTimerEvent( void )
{
    TimerTime_t multicastSlotTime = 0;
    TimerTime_t slotTime = 0;
    MulticastParams_t *cur = MulticastChannels;

    TimerStop( &MulticastSlotTimer );

    if( cur == NULL )
    {
        return;
    }

    switch( MulticastSlotState )
    {
        case PINGSLOT_STATE_CALC_PING_OFFSET:
        {
            while( cur != NULL )
            {
                LoRaMacBeaconComputePingOffset( BeaconCtx.BeaconTime,
                                                cur->Address,
                                                PingSlotCtx.PingPeriod,
                                                &( cur->PingOffset ) );
                cur = cur->Next;
            }
            MulticastSlotState = PINGSLOT_STATE_SET_TIMER;
            // no break
        }
        case PINGSLOT_STATE_SET_TIMER:
        {
            cur = MulticastChannels;
            PingSlotCtx.NextMulticastChannel = NULL;

            while( cur != NULL )
            {
                if( CalcNextSlotTime( cur->PingOffset, PingSlotCtx.PingPeriod, &slotTime ) == true )
                {
                    if( ( multicastSlotTime == 0 ) || ( multicastSlotTime > slotTime ) )
                    {
                        // Update the slot time and the next multicast channel
                        multicastSlotTime = slotTime;
                        PingSlotCtx.NextMulticastChannel = cur;
                    }
                }
                cur = cur->Next;
            }

            if( PingSlotCtx.NextMulticastChannel != NULL )
            {
                // Start the timer if the ping slot time is in range
                MulticastSlotState = PINGSLOT_STATE_IDLE;
                TimerSetValue( &MulticastSlotTimer, multicastSlotTime );
                TimerStart( &MulticastSlotTimer );
            }
            break;
        }
        case PINGSLOT_STATE_IDLE:
        {
            uint32_t frequency = PingSlotCtx.Frequency;

            if( PingSlotCtx.NextMulticastChannel == NULL )
            {
                MulticastSlotState = PINGSLOT_STATE_SET_TIMER;
                TimerSetValue( &MulticastSlotTimer, 1 );
                TimerStart( &MulticastSlotTimer );
                break;
            }

            if( PingSlotCtx.Ctrl.CustomFreq == 0 )
            {
                // Restore floor plan
                frequency = PINGSLOT_CHANNEL_FREQ( PingSlotCtx.NextMulticastChannel->Address );
            }

            if( BeaconCtx.Ctrl.BeaconLess == false )
            {
                PingSlotCtx.SymbolTimeout = GetRxSymbolTimeout( PingSlotCtx.Datarate );
            }
            MulticastSlotState = PINGSLOT_STATE_RX;
            RxWindowSetup( frequency, PingSlotCtx.Datarate, GetRxBandwidth( PingSlotCtx.Datarate ),
                           PingSlotCtx.SymbolTimeout, false );
            break;
        }
        default:
        {
            MulticastSlotState = PINGSLOT_STATE_SET_TIMER;
            break;
        }
    }
}

static void RxBeaconSetup( uint16_t timeout, uint32_t rxTime )
{
    bool rxContinuous = true;
    uint32_t frequency = BeaconCtx.Frequency;

    Radio.Sleep( );

    if( BeaconCtx.Ctrl.CustomFreq == 0 )
    {
        // Restore floor plan
        frequency = BEACON_CHANNEL_FREQ( );
    }

    if( BeaconCtx.Ctrl.BeaconChannelSet == 1 )
    {
        // Take the frequency of the next beacon
        BeaconCtx.Ctrl.BeaconChannelSet = 0;
        frequency = BEACON_CHANNEL_FREQ_IDX( BeaconCtx.BeaconTimingChannel );
    }

    Radio.SetChannel( frequency );

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    Radio.SetMaxPayloadLength( MODEM_LORA, 17 );
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    Radio.SetMaxPayloadLength( MODEM_LORA, 19 );
#endif

    if( rxTime != 0 )
    {
        rxContinuous = false;
    }

    // Store downlink datarate
    McpsIndication.RxDatarate = BEACON_CHANNEL_DR;

    Radio.SetRxConfig( MODEM_LORA, BEACON_CHANNEL_BW, Datarates[BEACON_CHANNEL_DR],
                       1, 0, 6, timeout, true, BEACON_SIZE, false, 0, 0, true, rxContinuous );

    Radio.Rx( rxTime );
}

static bool RxBeacon( uint8_t *payload, uint16_t size )
{
    bool beaconReceived = false;
    uint16_t crc0 = 0;
    uint16_t crc1 = 0;
    uint16_t beaconCrc0 = 0;
    uint16_t beaconCrc1 = 0;
    uint8_t rfuOffset1 = 0;
    uint8_t rfuOffset2 = 0;

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    rfuOffset1 = 1;
    rfuOffset2 = 1;
#endif


    TimerStop( &BeaconTimer );

    BeaconState = BEACON_STATE_REACQUISITION;

    if( size == BEACON_SIZE )
    {
        beaconCrc0 = ( ( uint16_t ) payload[6 + rfuOffset1] ) & 0x00FF;
        beaconCrc0 |= ( ( uint16_t ) payload[7 + rfuOffset1] << 8 ) & 0xFF00;
        crc0 = BeaconCrc( payload, 6 + rfuOffset1 );

        // Validate the first crc of the beacon frame
        if( crc0 == beaconCrc0 )
        {
            BeaconCtx.BeaconTime  = ( ( uint32_t ) payload[2 + rfuOffset1] ) & 0x000000FF;
            BeaconCtx.BeaconTime |= ( ( uint32_t ) ( payload[3 + rfuOffset1] << 8 ) ) & 0x0000FF00;
            BeaconCtx.BeaconTime |= ( ( uint32_t ) ( payload[4 + rfuOffset1] << 16 ) ) & 0x00FF0000;
            BeaconCtx.BeaconTime |= ( ( uint32_t ) ( payload[5 + rfuOffset1] << 24 ) ) & 0xFF000000;
            beaconReceived = true;
        }
        else
        {
            // If the crc of the beacon time part is not correct, the node has to
            // keep the internal timing.
            BeaconCtx.BeaconTime += ( BeaconCtx.Cfg.Interval / 1000 );
        }
        MlmeIndication.BeaconInfo.Time = BeaconCtx.BeaconTime;

        beaconCrc1 = ( ( uint16_t ) payload[15 + rfuOffset1 + rfuOffset2] ) & 0x00FF;
        beaconCrc1 |= ( ( uint16_t ) payload[16 + rfuOffset1 + rfuOffset2] << 8 ) & 0xFF00;
        crc1 = BeaconCrc( &payload[8 + rfuOffset1], 7 + rfuOffset2 );

        // Validate the second crc of the beacon frame
        if( crc1 == beaconCrc1 )
        {
            MlmeIndication.BeaconInfo.GwSpecific.InfoDesc = payload[8 + rfuOffset1];
            memcpy1( MlmeIndication.BeaconInfo.GwSpecific.Info, &payload[9 + rfuOffset1], 7 );
            beaconReceived = true;
        }

        // Reset beacon variables, if one of the crc is valid
        if( beaconReceived == true )
        {
            BeaconCtx.LastBeaconRx = TimerGetCurrentTime( ) - Radio.TimeOnAir( MODEM_LORA, size );
            BeaconCtx.Ctrl.BeaconLess = 0;
            BeaconCtx.SymbolTimeout = BeaconCtx.Cfg.SymbolToDefault;
            BeaconState = BEACON_STATE_LOCKED;
        }
    }

    if( BeaconState != BEACON_STATE_ACQUISITION )
    {
        OnBeaconTimerEvent( );
    }

    return true;
}

static uint16_t BeaconCrc( uint8_t *buffer, uint16_t length )
{
    // The CRC calculation follows CCITT
    const uint16_t polynom = 0x1021;
    // CRC initial value
    uint16_t crc = 0xFFFF;

    if( buffer == NULL )
    {
        return 0;
    }

    for( uint16_t i = 0; i < length; ++i )
    {
        crc ^= ( uint16_t ) buffer[i] << 8;
        for( uint16_t j = 0; j < 8; ++j )
        {
            crc = ( crc & 0x8000 ) ? ( crc << 1 ) ^ polynom : ( crc << 1 );
        }
    }

    return crc;
}

static bool IsBeaconExpected( void )
{
    if( LoRaMacDeviceClass == CLASS_B )
    {
        if( ( BeaconState == BEACON_STATE_ACQUISITION ) || ( BeaconState == BEACON_STATE_RX ) )
        {
            return true;
        }
    }
    return false;
}

static bool IsPingExpected( void )
{
    if( LoRaMacDeviceClass == CLASS_B )
    {
        if( PingSlotState == PINGSLOT_STATE_RX )
        {
            return true;
        }
    }
    return false;
}

#if ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
static uint8_t BeaconChannel( uint32_t devAddr )
{
    uint32_t frequency = 0;

    frequency = devAddr + ( BeaconCtx.BeaconTime / ( BeaconCtx.Cfg.Interval / 1000 ) );
    return ( ( uint8_t )( frequency % 8 ) );
}
#endif

static void HaltBeaconing( void )
{
    if( LoRaMacDeviceClass == CLASS_B )
    {
        if( BeaconState == BEACON_STATE_TIMEOUT )
        {
            // Update the state machine before halt
            OnBeaconTimerEvent( );
        }

        if( BeaconState != BEACON_STATE_SWITCH_CLASS )
        {
            // Halt beacon state machine
            BeaconState = BEACON_STATE_HALT;
            TimerStop( &BeaconTimer );

            // Halt ping slot state machine
            TimerStop( &PingSlotTimer );

            // Halt multicast ping slot state machine
            TimerStop( &MulticastSlotTimer );
        }
    }
}

static void ResumeBeaconing( void )
{
    if( LoRaMacDeviceClass == CLASS_B )
    {
        if( BeaconState == BEACON_STATE_HALT )
        {
            TimerTime_t currentTime = TimerGetCurrentTime( );

            // Set default state
            BeaconState = BEACON_STATE_LOCKED;

            if( BeaconCtx.Ctrl.BeaconLess == true )
            {
                // Set the default state for beacon less operation
                BeaconState = BEACON_STATE_REACQUISITION;
            }
            if( currentTime > BeaconCtx.NextBeaconRx )
            {
                // We have to update the beacon time, since we missed a beacon
                BeaconCtx.BeaconTime += ( BeaconCtx.Cfg.Interval / 1000 );
            }
            TimerSetValue( &BeaconTimer, 1 );
            TimerStart( &BeaconTimer );
        }
    }
}

static bool ValidatePayloadLength( uint8_t lenN, int8_t datarate, uint8_t fOptsLen )
{
    uint16_t maxN = 0;
    uint16_t payloadSize = 0;

    // Get the maximum payload length
    if( RepeaterSupport == true )
    {
        maxN = MaxPayloadOfDatarateRepeater[datarate];
    }
    else
    {
        maxN = MaxPayloadOfDatarate[datarate];
    }

    // Calculate the resulting payload size
    payloadSize = ( lenN + fOptsLen );

    // Validation of the application payload size
    if( ( payloadSize <= maxN ) && ( payloadSize <= LORAMAC_PHY_MAXPAYLOAD ) )
    {
        return true;
    }
    return false;
}

static uint8_t CountBits( uint16_t mask, uint8_t nbBits )
{
    uint8_t nbActiveBits = 0;

    for( uint8_t j = 0; j < nbBits; j++ )
    {
        if( ( mask & ( 1 << j ) ) == ( 1 << j ) )
        {
            nbActiveBits++;
        }
    }
    return nbActiveBits;
}

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
static uint8_t CountNbEnabled125kHzChannels( uint16_t *channelsMask )
{
    uint8_t nb125kHzChannels = 0;

    for( uint8_t i = 0, k = 0; i < LORA_MAX_NB_CHANNELS - 8; i += 16, k++ )
    {
        nb125kHzChannels += CountBits( channelsMask[k], 16 );
    }

    return nb125kHzChannels;
}

#if defined( USE_BAND_915_HYBRID )
static void ReenableChannels( uint16_t mask, uint16_t* channelsMask )
{
    uint16_t blockMask = mask;

    for( uint8_t i = 0, j = 0; i < 4; i++, j += 2 )
    {
        channelsMask[i] = 0;
        if( ( blockMask & ( 1 << j ) ) != 0 )
        {
            channelsMask[i] |= 0x00FF;
        }
        if( ( blockMask & ( 1 << ( j + 1 ) ) ) != 0 )
        {
            channelsMask[i] |= 0xFF00;
        }
    }
    channelsMask[4] = blockMask;
    channelsMask[5] = 0x0000;
}

static bool ValidateChannelMask( uint16_t* channelsMask )
{
    bool chanMaskState = false;
    uint16_t block1 = 0;
    uint16_t block2 = 0;
    uint8_t index = 0;

    for( uint8_t i = 0; i < 4; i++ )
    {
        block1 = channelsMask[i] & 0x00FF;
        block2 = channelsMask[i] & 0xFF00;

        if( ( CountBits( block1, 16 ) > 5 ) && ( chanMaskState == false ) )
        {
            channelsMask[i] &= block1;
            channelsMask[4] = 1 << ( i * 2 );
            chanMaskState = true;
            index = i;
        }
        else if( ( CountBits( block2, 16 ) > 5 ) && ( chanMaskState == false ) )
        {
            channelsMask[i] &= block2;
            channelsMask[4] = 1 << ( i * 2 + 1 );
            chanMaskState = true;
            index = i;
        }
    }

    // Do only change the channel mask, if we have found a valid block.
    if( chanMaskState == true )
    {
        for( uint8_t i = 0; i < 4; i++ )
        {
            if( i != index )
            {
                channelsMask[i] = 0;
            }
        }
    }
    return chanMaskState;
}
#endif
#endif

static bool ValidateDatarate( int8_t datarate, uint16_t* channelsMask )
{
    if( ValueInRange( datarate, LORAMAC_TX_MIN_DATARATE, LORAMAC_TX_MAX_DATARATE ) == false )
    {
        return false;
    }
    for( uint8_t i = 0, k = 0; i < LORA_MAX_NB_CHANNELS; i += 16, k++ )
    {
        for( uint8_t j = 0; j < 16; j++ )
        {
            if( ( ( channelsMask[k] & ( 1 << j ) ) != 0 ) )
            {// Check datarate validity for enabled channels
                if( ValueInRange( datarate, Channels[i + j].DrRange.Fields.Min, Channels[i + j].DrRange.Fields.Max ) == true )
                {
                    // At least 1 channel has been found we can return OK.
                    return true;
                }
            }
        }
    }
    return false;
}

static int8_t LimitTxPower( int8_t txPower )
{
    int8_t resultTxPower = txPower;
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    if( ( LoRaMacParams.ChannelsDatarate == DR_4 ) ||
        ( ( LoRaMacParams.ChannelsDatarate >= DR_8 ) && ( LoRaMacParams.ChannelsDatarate <= DR_13 ) ) )
    {// Limit tx power to max 26dBm
        resultTxPower =  MAX( txPower, TX_POWER_26_DBM );
    }
    else
    {
        if( CountNbEnabled125kHzChannels( LoRaMacParams.ChannelsMask ) < 50 )
        {// Limit tx power to max 21dBm
            resultTxPower = MAX( txPower, TX_POWER_20_DBM );
        }
    }
#endif
    return resultTxPower;
}

static bool ValueInRange( int8_t value, int8_t min, int8_t max )
{
    if( ( value >= min ) && ( value <= max ) )
    {
        return true;
    }
    return false;
}

static bool ValidateDrRange( DrRange_t drRange, int8_t min, int8_t max )
{
    int8_t drMin = drRange.Fields.Min & 0x0F;
    int8_t drMax = drRange.Fields.Max & 0x0F;

    if( drMin > drMax )
    {
        return false;
    }
    if( ValueInRange( drMin, min, max ) == false )
    {
        return false;
    }
    if( ValueInRange( drMax, min, max ) == false )
    {
        return false;
    }
    return true;
}

static bool DisableChannelInMask( uint8_t id, uint16_t* mask )
{
    uint8_t index = 0;
    index = id / 16;

    if( ( index > 4 ) || ( id >= LORA_MAX_NB_CHANNELS ) )
    {
        return false;
    }

    // Deactivate channel
    mask[index] &= ~( 1 << ( id % 16 ) );

    return true;
}

static bool AdrNextDr( bool adrEnabled, bool updateChannelMask, int8_t* datarateOut )
{
    bool adrAckReq = false;
    int8_t datarate = LoRaMacParams.ChannelsDatarate;

    if( adrEnabled == true )
    {
        if( datarate == LORAMAC_TX_MIN_DATARATE )
        {
            AdrAckCounter = 0;
            adrAckReq = false;
        }
        else
        {
            if( AdrAckCounter >= ADR_ACK_LIMIT )
            {
                adrAckReq = true;
            }
            else
            {
                adrAckReq = false;
            }
            if( AdrAckCounter >= ( ADR_ACK_LIMIT + ADR_ACK_DELAY ) )
            {
                if( ( AdrAckCounter % ADR_ACK_DELAY ) == 0 )
                {
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
                    if( datarate > LORAMAC_TX_MIN_DATARATE )
                    {
                        datarate--;
                    }
                    if( datarate == LORAMAC_TX_MIN_DATARATE )
                    {
                        if( updateChannelMask == true )
                        {
                            // Re-enable default channels LC1, LC2, LC3
                            LoRaMacParams.ChannelsMask[0] = LoRaMacParams.ChannelsMask[0] | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );
                        }
                    }
#elif defined( USE_BAND_470 )
                    if( datarate > LORAMAC_TX_MIN_DATARATE )
                    {
                        datarate--;
                    }
                    if( datarate == LORAMAC_TX_MIN_DATARATE )
                    {
                        if( updateChannelMask == true )
                        {
                            // Re-enable default channels
                            LoRaMacParams.ChannelsMask[0] = 0xFFFF;
                            LoRaMacParams.ChannelsMask[1] = 0xFFFF;
                            LoRaMacParams.ChannelsMask[2] = 0xFFFF;
                            LoRaMacParams.ChannelsMask[3] = 0xFFFF;
                            LoRaMacParams.ChannelsMask[4] = 0xFFFF;
                            LoRaMacParams.ChannelsMask[5] = 0xFFFF;
                        }
                    }
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                    if( ( datarate > LORAMAC_TX_MIN_DATARATE ) && ( datarate == DR_8 ) )
                    {
                        datarate = DR_4;
                    }
                    else if( datarate > LORAMAC_TX_MIN_DATARATE )
                    {
                        datarate--;
                    }
                    if( datarate == LORAMAC_TX_MIN_DATARATE )
                    {
                        if( updateChannelMask == true )
                        {
#if defined( USE_BAND_915 )
                            // Re-enable default channels
                            LoRaMacParams.ChannelsMask[0] = 0xFFFF;
                            LoRaMacParams.ChannelsMask[1] = 0xFFFF;
                            LoRaMacParams.ChannelsMask[2] = 0xFFFF;
                            LoRaMacParams.ChannelsMask[3] = 0xFFFF;
                            LoRaMacParams.ChannelsMask[4] = 0x00FF;
                            LoRaMacParams.ChannelsMask[5] = 0x0000;
#else // defined( USE_BAND_915_HYBRID )
                            // Re-enable default channels
                            ReenableChannels( LoRaMacParams.ChannelsMask[4], LoRaMacParams.ChannelsMask );
#endif
                        }
                    }
#else
#error "Please define a frequency band in the compiler options."
#endif
                }
            }
        }
    }

    *datarateOut = datarate;

    return adrAckReq;
}

static LoRaMacStatus_t AddMacCommand( uint8_t cmd, uint8_t p1, uint8_t p2 )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_BUSY;
    // The maximum buffer length must take MAC commands to re-send into account.
    uint8_t bufLen = LORA_MAC_COMMAND_MAX_LENGTH - MacCommandsBufferToRepeatIndex;

    switch( cmd )
    {
        case MOTE_MAC_LINK_CHECK_REQ:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this command
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_LINK_ADR_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Margin
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DUTY_CYCLE_ANS:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_RX_PARAM_SETUP_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate ACK, Channel ACK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DEV_STATUS_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 2 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // 1st byte Battery
                // 2nd byte Margin
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                MacCommandsBuffer[MacCommandsBufferIndex++] = p2;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_NEW_CHANNEL_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate range OK, Channel frequency OK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_RX_TIMING_SETUP_ANS:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_PING_SLOT_INFO_REQ:
            if( MacCommandsBufferIndex < ( LORA_MAC_COMMAND_MAX_LENGTH - 1 ) )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Periodicity and Datarate
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_PING_SLOT_FREQ_ANS:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate range OK, Channel frequency OK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_BEACON_TIMING_REQ:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_BEACON_FREQ_ANS:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        default:
            return LORAMAC_STATUS_SERVICE_UNKNOWN;
    }
    if( status == LORAMAC_STATUS_OK )
    {
        MacCommandsInNextTx = true;
    }
    return status;
}

static uint8_t ParseMacCommandsToRepeat( uint8_t* cmdBufIn, uint8_t length, uint8_t* cmdBufOut )
{
    uint8_t i = 0;
    uint8_t cmdCount = 0;

    if( ( cmdBufIn == NULL ) || ( cmdBufOut == NULL ) )
    {
        return 0;
    }

    for( i = 0; i < length; i++ )
    {
        switch( cmdBufIn[i] )
        {
            case MOTE_MAC_RX_PARAM_SETUP_ANS:
            {
                cmdBufOut[cmdCount++] = cmdBufIn[i++];
                cmdBufOut[cmdCount++] = cmdBufIn[i++];
                cmdBufOut[cmdCount++] = cmdBufIn[i];
                break;
            }
            case MOTE_MAC_RX_TIMING_SETUP_ANS:
            {
                cmdBufOut[cmdCount++] = cmdBufIn[i];
                break;
            }
            default:
                break;
        }
    }

    return cmdCount;
}

static void ProcessMacCommands( uint8_t *payload, uint8_t macIndex, uint8_t commandsSize, uint8_t snr )
{
    uint8_t index = 0;

    while( macIndex < commandsSize )
    {
        // Decode Frame MAC commands
        switch( payload[macIndex++] )
        {
            case SRV_MAC_LINK_CHECK_ANS:
                index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_LINK_CHECK, MlmeConfirmQueueCnt );
                if( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
                {
                    MlmeConfirmQueue[index].Status = LORAMAC_EVENT_INFO_STATUS_OK;
                    MlmeConfirm.DemodMargin = payload[macIndex++];
                    MlmeConfirm.NbGateways = payload[macIndex++];
                }
                break;
            case SRV_MAC_LINK_ADR_REQ:
                {
                    uint8_t i;
                    uint8_t status = 0x07;
                    uint16_t chMask;
                    int8_t txPower = 0;
                    int8_t datarate = 0;
                    uint8_t nbRep = 0;
                    uint8_t chMaskCntl = 0;
                    uint16_t channelsMask[6] = { 0, 0, 0, 0, 0, 0 };

                    // Initialize local copy of the channels mask array
                    for( i = 0; i < 6; i++ )
                    {
                        channelsMask[i] = LoRaMacParams.ChannelsMask[i];
                    }
                    datarate = payload[macIndex++];
                    txPower = datarate & 0x0F;
                    datarate = ( datarate >> 4 ) & 0x0F;

                    if( ( AdrCtrlOn == false ) &&
                        ( ( LoRaMacParams.ChannelsDatarate != datarate ) || ( LoRaMacParams.ChannelsTxPower != txPower ) ) )
                    { // ADR disabled don't handle ADR requests if server tries to change datarate or txpower
                        // Answer the server with fail status
                        // Power ACK     = 0
                        // Data rate ACK = 0
                        // Channel mask  = 0
                        AddMacCommand( MOTE_MAC_LINK_ADR_ANS, 0, 0 );
                        macIndex += 3;  // Skip over the remaining bytes of the request
                        break;
                    }
                    chMask = ( uint16_t )payload[macIndex++];
                    chMask |= ( uint16_t )payload[macIndex++] << 8;

                    nbRep = payload[macIndex++];
                    chMaskCntl = ( nbRep >> 4 ) & 0x07;
                    nbRep &= 0x0F;
                    if( nbRep == 0 )
                    {
                        nbRep = 1;
                    }
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
                    if( ( chMaskCntl == 0 ) && ( chMask == 0 ) )
                    {
                        status &= 0xFE; // Channel mask KO
                    }
                    else if( ( ( chMaskCntl >= 1 ) && ( chMaskCntl <= 5 )) ||
                             ( chMaskCntl >= 7 ) )
                    {
                        // RFU
                        status &= 0xFE; // Channel mask KO
                    }
                    else
                    {
                        for( i = 0; i < LORA_MAX_NB_CHANNELS; i++ )
                        {
                            if( chMaskCntl == 6 )
                            {
                                if( Channels[i].Frequency != 0 )
                                {
                                    chMask |= 1 << i;
                                }
                            }
                            else
                            {
                                if( ( ( chMask & ( 1 << i ) ) != 0 ) &&
                                    ( Channels[i].Frequency == 0 ) )
                                {// Trying to enable an undefined channel
                                    status &= 0xFE; // Channel mask KO
                                }
                            }
                        }
                        channelsMask[0] = chMask;
                    }
#elif defined( USE_BAND_470 )
                    if( chMaskCntl == 6 )
                    {
                        // Enable all 125 kHz channels
                        for( uint8_t i = 0, k = 0; i < LORA_MAX_NB_CHANNELS; i += 16, k++ )
                        {
                            for( uint8_t j = 0; j < 16; j++ )
                            {
                                if( Channels[i + j].Frequency != 0 )
                                {
                                    channelsMask[k] |= 1 << j;
                                }
                            }
                        }
                    }
                    else if( chMaskCntl == 7 )
                    {
                        status &= 0xFE; // Channel mask KO
                    }
                    else
                    {
                        for( uint8_t i = 0; i < 16; i++ )
                        {
                            if( ( ( chMask & ( 1 << i ) ) != 0 ) &&
                                ( Channels[chMaskCntl * 16 + i].Frequency == 0 ) )
                            {// Trying to enable an undefined channel
                                status &= 0xFE; // Channel mask KO
                            }
                        }
                        channelsMask[chMaskCntl] = chMask;
                    }
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                    if( chMaskCntl == 6 )
                    {
                        // Enable all 125 kHz channels
                        channelsMask[0] = 0xFFFF;
                        channelsMask[1] = 0xFFFF;
                        channelsMask[2] = 0xFFFF;
                        channelsMask[3] = 0xFFFF;
                        // Apply chMask to channels 64 to 71
                        channelsMask[4] = chMask;
                    }
                    else if( chMaskCntl == 7 )
                    {
                        // Disable all 125 kHz channels
                        channelsMask[0] = 0x0000;
                        channelsMask[1] = 0x0000;
                        channelsMask[2] = 0x0000;
                        channelsMask[3] = 0x0000;
                        // Apply chMask to channels 64 to 71
                        channelsMask[4] = chMask;
                    }
                    else if( chMaskCntl == 5 )
                    {
                        // RFU
                        status &= 0xFE; // Channel mask KO
                    }
                    else
                    {
                        channelsMask[chMaskCntl] = chMask;

                        // FCC 15.247 paragraph F mandates to hop on at least 2 125 kHz channels
                        if( ( datarate < DR_4 ) && ( CountNbEnabled125kHzChannels( channelsMask ) < 2 ) )
                        {
                            status &= 0xFE; // Channel mask KO
                        }

#if defined( USE_BAND_915_HYBRID )
                        if( ValidateChannelMask( channelsMask ) == false )
                        {
                            status &= 0xFE; // Channel mask KO
                        }
#endif
                    }
#else
    #error "Please define a frequency band in the compiler options."
#endif
                    if( ValidateDatarate( datarate, channelsMask ) == false )
                    {
                        status &= 0xFD; // Datarate KO
                    }

                    //
                    // Remark MaxTxPower = 0 and MinTxPower = 5
                    //
                    if( ValueInRange( txPower, LORAMAC_MAX_TX_POWER, LORAMAC_MIN_TX_POWER ) == false )
                    {
                        status &= 0xFB; // TxPower KO
                    }
                    if( ( status & 0x07 ) == 0x07 )
                    {
                        LoRaMacParams.ChannelsDatarate = datarate;
                        LoRaMacParams.ChannelsTxPower = txPower;

                        LoRaMacParams.ChannelsMask[0] = channelsMask[0];
                        LoRaMacParams.ChannelsMask[1] = channelsMask[1];
                        LoRaMacParams.ChannelsMask[2] = channelsMask[2];
                        LoRaMacParams.ChannelsMask[3] = channelsMask[3];
                        LoRaMacParams.ChannelsMask[4] = channelsMask[4];
                        LoRaMacParams.ChannelsMask[5] = channelsMask[5];

                        LoRaMacParams.ChannelsNbRep = nbRep;
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                        // Reset ChannelsMaskRemaining to the new ChannelsMask
                        ChannelsMaskRemaining[0] &= channelsMask[0];
                        ChannelsMaskRemaining[1] &= channelsMask[1];
                        ChannelsMaskRemaining[2] &= channelsMask[2];
                        ChannelsMaskRemaining[3] &= channelsMask[3];
                        ChannelsMaskRemaining[4] = channelsMask[4];
                        ChannelsMaskRemaining[5] = channelsMask[5];
#endif
                    }
                    AddMacCommand( MOTE_MAC_LINK_ADR_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DUTY_CYCLE_REQ:
                MaxDCycle = payload[macIndex++];
                AggregatedDCycle = 1 << MaxDCycle;
                AddMacCommand( MOTE_MAC_DUTY_CYCLE_ANS, 0, 0 );
                break;
            case SRV_MAC_RX_PARAM_SETUP_REQ:
                {
                    uint8_t status = 0x07;
                    int8_t datarate = 0;
                    int8_t drOffset = 0;
                    uint32_t freq = 0;

                    drOffset = ( payload[macIndex] >> 4 ) & 0x07;
                    datarate = payload[macIndex] & 0x0F;
                    macIndex++;

                    freq =  ( uint32_t )payload[macIndex++];
                    freq |= ( uint32_t )payload[macIndex++] << 8;
                    freq |= ( uint32_t )payload[macIndex++] << 16;
                    freq *= 100;

                    if( Rx2FreqInRange( freq ) == false )
                    {
                        status &= 0xFE; // Channel frequency KO
                    }

                    if( ValueInRange( datarate, LORAMAC_RX_MIN_DATARATE, LORAMAC_RX_MAX_DATARATE ) == false )
                    {
                        status &= 0xFD; // Datarate KO
                    }
#if ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) )
                    if( ( ValueInRange( datarate, DR_5, DR_7 ) == true ) ||
                        ( datarate > DR_13 ) )
                    {
                        status &= 0xFD; // Datarate KO
                    }
#endif
                    if( ValueInRange( drOffset, LORAMAC_MIN_RX1_DR_OFFSET, LORAMAC_MAX_RX1_DR_OFFSET ) == false )
                    {
                        status &= 0xFB; // Rx1DrOffset range KO
                    }

                    if( ( status & 0x07 ) == 0x07 )
                    {
                        LoRaMacParams.Rx2Channel.Datarate = datarate;
                        LoRaMacParams.Rx2Channel.Frequency = freq;
                        LoRaMacParams.Rx1DrOffset = drOffset;
                    }
                    AddMacCommand( MOTE_MAC_RX_PARAM_SETUP_ANS, status, 0 );
                }
                break;
            case SRV_MAC_DEV_STATUS_REQ:
                {
                    uint8_t batteryLevel = BAT_LEVEL_NO_MEASURE;
                    if( ( LoRaMacCallbacks != NULL ) && ( LoRaMacCallbacks->GetBatteryLevel != NULL ) )
                    {
                        batteryLevel = LoRaMacCallbacks->GetBatteryLevel( );
                    }
                    AddMacCommand( MOTE_MAC_DEV_STATUS_ANS, batteryLevel, snr );
                    break;
                }
            case SRV_MAC_NEW_CHANNEL_REQ:
                {
                    uint8_t status = 0x03;

#if defined( USE_BAND_470 ) || defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                    status &= 0xFC; // Channel frequency and datarate KO
                    macIndex += 5;
#else
                    int8_t channelIndex = 0;
                    ChannelParams_t chParam;

                    channelIndex = payload[macIndex++];
                    chParam.Frequency = ( uint32_t )payload[macIndex++];
                    chParam.Frequency |= ( uint32_t )payload[macIndex++] << 8;
                    chParam.Frequency |= ( uint32_t )payload[macIndex++] << 16;
                    chParam.Frequency *= 100;
                    chParam.DrRange.Value = payload[macIndex++];

                    LoRaMacState |= MAC_TX_CONFIG;
                    if( chParam.Frequency == 0 )
                    {
                        if( channelIndex < 3 )
                        {
                            status &= 0xFC;
                        }
                        else
                        {
                            if( LoRaMacChannelRemove( channelIndex ) != LORAMAC_STATUS_OK )
                            {
                                status &= 0xFC;
                            }
                        }
                    }
                    else
                    {
                        switch( LoRaMacChannelAdd( channelIndex, chParam ) )
                        {
                            case LORAMAC_STATUS_OK:
                            {
                                break;
                            }
                            case LORAMAC_STATUS_FREQUENCY_INVALID:
                            {
                                status &= 0xFE;
                                break;
                            }
                            case LORAMAC_STATUS_DATARATE_INVALID:
                            {
                                status &= 0xFD;
                                break;
                            }
                            case LORAMAC_STATUS_FREQ_AND_DR_INVALID:
                            {
                                status &= 0xFC;
                                break;
                            }
                            default:
                            {
                                status &= 0xFC;
                                break;
                            }
                        }
                    }
                    LoRaMacState &= ~MAC_TX_CONFIG;
#endif
                    AddMacCommand( MOTE_MAC_NEW_CHANNEL_ANS, status, 0 );
                }
                break;
            case SRV_MAC_RX_TIMING_SETUP_REQ:
                {
                    uint8_t delay = payload[macIndex++] & 0x0F;

                    if( delay == 0 )
                    {
                        delay++;
                    }
                    LoRaMacParams.ReceiveDelay1 = delay * 1e3;
                    LoRaMacParams.ReceiveDelay2 = LoRaMacParams.ReceiveDelay1 + 1e3;
                    AddMacCommand( MOTE_MAC_RX_TIMING_SETUP_ANS, 0, 0 );
                }
                break;
            case SRV_MAC_PING_SLOT_INFO_ANS:
                {
                    index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_PING_SLOT_INFO, MlmeConfirmQueueCnt );
                    if( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
                    {
                        MlmeConfirmQueue[index].Status = LORAMAC_EVENT_INFO_STATUS_OK;
                        PingSlotCtx.Ctrl.Assigned = 1;
                    }
                }
                break;
            case SRV_MAC_PING_SLOT_CHANNEL_REQ:
                {
                    uint8_t status = 0x03;
                    uint32_t frequency = 0;
                    DrRange_t drRange;

                    frequency = ( uint32_t )payload[macIndex++];
                    frequency |= ( uint32_t )payload[macIndex++] << 8;
                    frequency |= ( uint32_t )payload[macIndex++] << 16;
                    frequency *= 100;
                    drRange.Value = payload[macIndex++];

                    if( frequency != 0 )
                    {
                        if( Radio.CheckRfFrequency( frequency ) == false )
                        {
                            status &= 0xFE; // Channel frequency KO
                        }

                        if( ValidateDrRange( drRange, LORAMAC_RX_MIN_DATARATE, LORAMAC_RX_MAX_DATARATE ) == false )
                        {
                            status &= 0xFD; // Datarate range KO
                        }

                        if( status == 0x03 )
                        {
                            PingSlotCtx.Ctrl.CustomFreq = 1;
                            PingSlotCtx.Frequency = frequency;
                            PingSlotCtx.DrRange.Value = drRange.Value;
                        }
                    }
                    else
                    {
                        PingSlotCtx.Ctrl.CustomFreq = 0;
                        PingSlotCtx.DrRange.Fields.Max = BEACON_CHANNEL_DR;
                        PingSlotCtx.DrRange.Fields.Min = BEACON_CHANNEL_DR;
                    }
                    AddMacCommand( MOTE_MAC_PING_SLOT_FREQ_ANS, status, 0 );
                }
                break;
            case SRV_MAC_BEACON_TIMING_ANS:
                {
                    uint16_t beaconTimingDelay = ( uint16_t )payload[macIndex++];
                    beaconTimingDelay |= ( uint16_t )payload[macIndex++] << 8;

                    BeaconCtx.BeaconTimingDelay = ( BeaconCtx.Cfg.DelayBeaconTimingAns * beaconTimingDelay );
                    BeaconCtx.BeaconTimingChannel = payload[macIndex++];
                    BeaconCtx.Ctrl.BeaconDelaySet = 1;
                    BeaconCtx.Ctrl.BeaconChannelSet = 1;


                    index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_BEACON_TIMING, MlmeConfirmQueueCnt );
                    if( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
                    {
#ifndef LORAWAN_CLASSB_ORBIWISE
                    BeaconCtx.BeaconTimingDelay = BeaconCtx.BeaconTimingDelay -
                                                  ( TimerGetCurrentTime( ) - Bands[Channels[LastTxChannel].Band].LastTxDoneTime );
#endif
                    if( BeaconCtx.BeaconTimingDelay > BeaconCtx.Cfg.Interval )
                    {
                        // We missed the beacon already
                        BeaconCtx.BeaconTimingDelay = 0;
                        BeaconCtx.Ctrl.BeaconDelaySet = 0;
                        BeaconCtx.Ctrl.BeaconChannelSet = 0;
                        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND;
                    }

                    MlmeConfirm.BeaconTimingDelay = BeaconCtx.BeaconTimingDelay;
                    MlmeConfirm.BeaconTimingChannel = BeaconCtx.BeaconTimingChannel;
                            MlmeConfirmQueue[index].Status = LORAMAC_EVENT_INFO_STATUS_OK;

                    }
                }
                break;
            case SRV_MAC_BEACON_FREQ_REQ:
                {
                    uint32_t frequency = 0;

                    frequency = ( uint32_t )payload[macIndex++];
                    frequency |= ( uint32_t )payload[macIndex++] << 8;
                    frequency |= ( uint32_t )payload[macIndex++] << 16;
                    frequency *= 100;

                    if( frequency != 0 )
                    {
                        if( Radio.CheckRfFrequency( frequency ) == true )
                        {
                            BeaconCtx.Ctrl.CustomFreq = 1;
                            BeaconCtx.Frequency = frequency;
                            AddMacCommand( MOTE_MAC_PING_SLOT_FREQ_ANS, 0, 0 );
                        }
                    }
                    else
                    {
                        BeaconCtx.Ctrl.CustomFreq = 0;
                        AddMacCommand( MOTE_MAC_PING_SLOT_FREQ_ANS, 0, 0 );
                    }
                }
                break;
            default:
                // Unknown command. ABORT MAC commands processing
                return;
        }
    }
}

LoRaMacStatus_t Send( LoRaMacHeader_t *macHdr, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    LoRaMacFrameCtrl_t fCtrl;
    LoRaMacStatus_t status = LORAMAC_STATUS_PARAMETER_INVALID;

    fCtrl.Value = 0;
    fCtrl.Bits.FOptsLen      = 0;
    if( LoRaMacDeviceClass == CLASS_B )
    {
        fCtrl.Bits.FPending      = 1;
    }
    else
    {
        fCtrl.Bits.FPending      = 0;
    }
    fCtrl.Bits.Ack           = false;
    fCtrl.Bits.AdrAckReq     = false;
    fCtrl.Bits.Adr           = AdrCtrlOn;

    // Prepare the frame
    status = PrepareFrame( macHdr, &fCtrl, fPort, fBuffer, fBufferSize );

    // Validate status
    if( status != LORAMAC_STATUS_OK )
    {
        return status;
    }

    // Reset confirm parameters
    McpsConfirm.NbRetries = 0;
    McpsConfirm.AckReceived = false;
    McpsConfirm.UpLinkCounter = UpLinkCounter;

    status = ScheduleTx( );

    return status;
}

static LoRaMacStatus_t ScheduleTx( )
{
    TimerTime_t dutyCycleTimeOff = 0;
    TimerTime_t mutexTimeLock = 0;
    TimerTime_t timeOff = 0;

    // Check if the device is off
    if( MaxDCycle == 255 )
    {
        return LORAMAC_STATUS_DEVICE_OFF;
    }
    if( MaxDCycle == 0 )
    {
        AggregatedTimeOff = 0;
    }

    CalculateBackOff( LastTxChannel );

    // Select channel
    while( SetNextChannel( &dutyCycleTimeOff ) == false )
    {
        // Set the default datarate
        LoRaMacParams.ChannelsDatarate = LoRaMacParamsDefaults.ChannelsDatarate;

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
        // Re-enable default channels LC1, LC2, LC3
        LoRaMacParams.ChannelsMask[0] = LoRaMacParams.ChannelsMask[0] | ( LC( 1 ) + LC( 2 ) + LC( 3 ) );
#endif
    }

    // Schedule transmission of frame
    if( dutyCycleTimeOff == 0 )
    {
        // Try to send now
        mutexTimeLock = SendFrameOnChannel( Channels[Channel] );
    }

    timeOff = MAX( dutyCycleTimeOff, mutexTimeLock );

    if( timeOff > 0 )
    {
        // Send later - prepare timer
        LoRaMacState |= MAC_TX_DELAYED;
        TimerSetValue( &TxDelayedTimer, timeOff );
        TimerStart( &TxDelayedTimer );
    }
    return LORAMAC_STATUS_OK;
}

static uint16_t RetransmissionDutyCylce( void )
{
    uint16_t dutyCycle = 0;

#if defined( USE_BAND_868 ) || defined( USE_BAND_433 ) || defined( USE_BAND_780 )
    TimerTime_t timeElapsed = TimerGetElapsedTime( 0 );

    if( timeElapsed < 3600000 )
    {
        dutyCycle = BACKOFF_DC_1_HOUR;
    }
    else if( timeElapsed < ( 3600000 + 36000000 ) )
    {
        dutyCycle = BACKOFF_DC_10_HOURS;
    }
    else
    {
        dutyCycle = BACKOFF_DC_24_HOURS;
    }
#endif
    return dutyCycle;
}

static void CalculateBackOff( uint8_t channel )
{
    uint16_t dutyCycle = Bands[Channels[channel].Band].DCycle;
    uint16_t joinDutyCycle = 0;
    bool rndTimeOff = false;

    if( IsLoRaMacNetworkJoined == false )
    {
        joinDutyCycle = RetransmissionDutyCylce( );
        dutyCycle = MAX( dutyCycle, joinDutyCycle );

        // Make sure to not apply the random back-off to the first TX
        if( TxTimeOnAir > 0 )
        {
            rndTimeOff = true;
        }
    }

    // Update Band Time OFF
    if( DutyCycleOn == true )
    {
        Bands[Channels[channel].Band].TimeOff = TxTimeOnAir * dutyCycle - TxTimeOnAir;
    }
    else
    {
        Bands[Channels[channel].Band].TimeOff = 0;
    }

    if( rndTimeOff == true )
    {
        Bands[Channels[channel].Band].TimeOff = randr( Bands[Channels[channel].Band].TimeOff,
                                                       Bands[Channels[channel].Band].TimeOff + BACKOFF_RND_OFFSET );
    }

    // Update Aggregated Time OFF
    AggregatedTimeOff = AggregatedTimeOff + ( TxTimeOnAir * AggregatedDCycle - TxTimeOnAir );
}

static int8_t AlternateDatarate( uint16_t nbTrials )
{
    int8_t datarate = LORAMAC_TX_MIN_DATARATE;
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
#if defined( USE_BAND_915 )
    // Re-enable 500 kHz default channels
    LoRaMacParams.ChannelsMask[4] = 0x00FF;
#else // defined( USE_BAND_915_HYBRID )
    // Re-enable 500 kHz default channels
    ReenableChannels( LoRaMacParams.ChannelsMask[4], LoRaMacParams.ChannelsMask );
#endif

    if( ( nbTrials & 0x01 ) == 0x01 )
    {
        datarate = DR_4;
    }
    else
    {
        datarate = DR_0;
    }
#elif defined( USE_BAND_470 )
    datarate = randr( LORAMAC_TX_MIN_DATARATE, LORAMAC_TX_MAX_DATARATE - 1 );
#else
    if( ( nbTrials % 48 ) == 0 )
    {
        datarate = DR_0;
    }
    else if( ( nbTrials % 32 ) == 0 )
    {
        datarate = DR_1;
    }
    else if( ( nbTrials % 24 ) == 0 )
    {
        datarate = DR_2;
    }
    else if( ( nbTrials % 16 ) == 0 )
    {
        datarate = DR_3;
    }
    else if( ( nbTrials % 8 ) == 0 )
    {
        datarate = DR_4;
    }
    else
    {
        datarate = DR_5;
    }
#endif
    return datarate;
}

static uint8_t GetMlmeConfirmIndex( sMlmeConfirmQueue_t* queue, Mlme_t req, uint8_t length )
{
    for( uint8_t i = 0; i < length; i++ )
    {
        if( queue->MlmeRequest == req )
        {
            return i;
        }
        queue++;
    }

    // Out of band
    return LORA_MAC_MLME_CONFIRM_QUEUE_LEN;
}

static void ResetMacParameters( void )
{
    IsLoRaMacNetworkJoined = false;

    MlmeConfirmQueueCnt = 0;
    memset( &MlmeConfirmQueue, 0xFF, sizeof( MlmeConfirmQueue ) );

    // Counters
    UpLinkCounter = 1;
    DownLinkCounter = 0;
    AdrAckCounter = 0;

    ChannelsNbRepCounter = 0;

    AckTimeoutRetries = 1;
    AckTimeoutRetriesCounter = 1;
    AckTimeoutRetry = false;

    MaxDCycle = 0;
    AggregatedDCycle = 1;

    MacCommandsBufferIndex = 0;
    MacCommandsBufferToRepeatIndex = 0;

    IsRxWindowsEnabled = true;

    LoRaMacParams.ChannelsTxPower = LoRaMacParamsDefaults.ChannelsTxPower;
    LoRaMacParams.ChannelsDatarate = LoRaMacParamsDefaults.ChannelsDatarate;

    LoRaMacParams.MaxRxWindow = LoRaMacParamsDefaults.MaxRxWindow;
    LoRaMacParams.ReceiveDelay1 = LoRaMacParamsDefaults.ReceiveDelay1;
    LoRaMacParams.ReceiveDelay2 = LoRaMacParamsDefaults.ReceiveDelay2;
    LoRaMacParams.JoinAcceptDelay1 = LoRaMacParamsDefaults.JoinAcceptDelay1;
    LoRaMacParams.JoinAcceptDelay2 = LoRaMacParamsDefaults.JoinAcceptDelay2;

    LoRaMacParams.Rx1DrOffset = LoRaMacParamsDefaults.Rx1DrOffset;
    LoRaMacParams.ChannelsNbRep = LoRaMacParamsDefaults.ChannelsNbRep;

    LoRaMacParams.Rx2Channel = LoRaMacParamsDefaults.Rx2Channel;

    memcpy1( ( uint8_t* ) LoRaMacParams.ChannelsMask, ( uint8_t* ) LoRaMacParamsDefaults.ChannelsMask, sizeof( LoRaMacParams.ChannelsMask ) );

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    memcpy1( ( uint8_t* ) ChannelsMaskRemaining, ( uint8_t* ) LoRaMacParamsDefaults.ChannelsMask, sizeof( LoRaMacParams.ChannelsMask ) );
#endif


    NodeAckRequested = false;
    SrvAckRequested = false;
    MacCommandsInNextTx = false;

    // Reset Multicast downlink counters
    MulticastParams_t *cur = MulticastChannels;
    while( cur != NULL )
    {
        cur->DownLinkCounter = 0;
        cur = cur->Next;
    }

    // Initialize channel index.
    Channel = LORA_MAX_NB_CHANNELS;
}

LoRaMacStatus_t PrepareFrame( LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    uint16_t i;
    uint8_t pktHeaderLen = 0;
    uint32_t mic = 0;
    const void* payload = fBuffer;
    uint8_t framePort = fPort;

    LoRaMacBufferPktLen = 0;

    NodeAckRequested = false;

    if( fBuffer == NULL )
    {
        fBufferSize = 0;
    }

    LoRaMacTxPayloadLen = fBufferSize;

    LoRaMacBuffer[pktHeaderLen++] = macHdr->Value;

    switch( macHdr->Bits.MType )
    {
        case FRAME_TYPE_JOIN_REQ:
            RxWindow1Delay = LoRaMacParams.JoinAcceptDelay1 - RADIO_WAKEUP_TIME;
            RxWindow2Delay = LoRaMacParams.JoinAcceptDelay2 - RADIO_WAKEUP_TIME;

            LoRaMacBufferPktLen = pktHeaderLen;

            memcpyr( LoRaMacBuffer + LoRaMacBufferPktLen, LoRaMacAppEui, 8 );
            LoRaMacBufferPktLen += 8;
            memcpyr( LoRaMacBuffer + LoRaMacBufferPktLen, LoRaMacDevEui, 8 );
            LoRaMacBufferPktLen += 8;

            LoRaMacDevNonce = Radio.Random( );

            LoRaMacBuffer[LoRaMacBufferPktLen++] = LoRaMacDevNonce & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( LoRaMacDevNonce >> 8 ) & 0xFF;

            LoRaMacJoinComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen & 0xFF, LoRaMacAppKey, &mic );

            LoRaMacBuffer[LoRaMacBufferPktLen++] = mic & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 8 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 16 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen++] = ( mic >> 24 ) & 0xFF;

            break;
        case FRAME_TYPE_DATA_CONFIRMED_UP:
            NodeAckRequested = true;
            //Intentional falltrough
        case FRAME_TYPE_DATA_UNCONFIRMED_UP:
            if( IsLoRaMacNetworkJoined == false )
            {
                return LORAMAC_STATUS_NO_NETWORK_JOINED; // No network has been joined yet
            }

            fCtrl->Bits.AdrAckReq = AdrNextDr( fCtrl->Bits.Adr, true, &LoRaMacParams.ChannelsDatarate );

            if( ValidatePayloadLength( LoRaMacTxPayloadLen, LoRaMacParams.ChannelsDatarate, MacCommandsBufferIndex ) == false )
            {
                return LORAMAC_STATUS_LENGTH_ERROR;
            }

            RxWindow1Delay = LoRaMacParams.ReceiveDelay1 - RADIO_WAKEUP_TIME;
            RxWindow2Delay = LoRaMacParams.ReceiveDelay2 - RADIO_WAKEUP_TIME;

            if( SrvAckRequested == true )
            {
                SrvAckRequested = false;
                fCtrl->Bits.Ack = 1;
            }

            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 8 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 16 ) & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( LoRaMacDevAddr >> 24 ) & 0xFF;

            LoRaMacBuffer[pktHeaderLen++] = fCtrl->Value;

            LoRaMacBuffer[pktHeaderLen++] = UpLinkCounter & 0xFF;
            LoRaMacBuffer[pktHeaderLen++] = ( UpLinkCounter >> 8 ) & 0xFF;

            // Copy the MAC commands which must be re-send into the MAC command buffer
            memcpy1( &MacCommandsBuffer[MacCommandsBufferIndex], MacCommandsBufferToRepeat, MacCommandsBufferToRepeatIndex );
            MacCommandsBufferIndex += MacCommandsBufferToRepeatIndex;

            if( ( payload != NULL ) && ( LoRaMacTxPayloadLen > 0 ) )
            {
                if( ( MacCommandsBufferIndex <= LORA_MAC_COMMAND_MAX_LENGTH ) && ( MacCommandsInNextTx == true ) )
                {
                    fCtrl->Bits.FOptsLen += MacCommandsBufferIndex;

                    // Update FCtrl field with new value of OptionsLength
                    LoRaMacBuffer[0x05] = fCtrl->Value;
                    for( i = 0; i < MacCommandsBufferIndex; i++ )
                    {
                        LoRaMacBuffer[pktHeaderLen++] = MacCommandsBuffer[i];
                    }
                }
            }
            else
            {
                if( ( MacCommandsBufferIndex > 0 ) && ( MacCommandsInNextTx ) )
                {
                    LoRaMacTxPayloadLen = MacCommandsBufferIndex;
                    payload = MacCommandsBuffer;
                    framePort = 0;
                }
            }
            MacCommandsInNextTx = false;
            // Store MAC commands which must be re-send in case the device does not receive a downlink anymore
            MacCommandsBufferToRepeatIndex = ParseMacCommandsToRepeat( MacCommandsBuffer, MacCommandsBufferIndex, MacCommandsBufferToRepeat );
            if( MacCommandsBufferToRepeatIndex > 0 )
            {
                MacCommandsInNextTx = true;
            }
            MacCommandsBufferIndex = 0;

            if( ( payload != NULL ) && ( LoRaMacTxPayloadLen > 0 ) )
            {
                LoRaMacBuffer[pktHeaderLen++] = framePort;

                if( framePort == 0 )
                {
                    LoRaMacPayloadEncrypt( (uint8_t* ) payload, LoRaMacTxPayloadLen, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, LoRaMacPayload );
                }
                else
                {
                    LoRaMacPayloadEncrypt( (uint8_t* ) payload, LoRaMacTxPayloadLen, LoRaMacAppSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, LoRaMacPayload );
                }
                memcpy1( LoRaMacBuffer + pktHeaderLen, LoRaMacPayload, LoRaMacTxPayloadLen );
            }
            LoRaMacBufferPktLen = pktHeaderLen + LoRaMacTxPayloadLen;

            LoRaMacComputeMic( LoRaMacBuffer, LoRaMacBufferPktLen, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &mic );

            LoRaMacBuffer[LoRaMacBufferPktLen + 0] = mic & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 1] = ( mic >> 8 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 2] = ( mic >> 16 ) & 0xFF;
            LoRaMacBuffer[LoRaMacBufferPktLen + 3] = ( mic >> 24 ) & 0xFF;

            LoRaMacBufferPktLen += LORAMAC_MFR_LEN;

            break;
        case FRAME_TYPE_PROPRIETARY:
            if( ( fBuffer != NULL ) && ( LoRaMacTxPayloadLen > 0 ) )
            {
                memcpy1( LoRaMacBuffer + pktHeaderLen, ( uint8_t* ) fBuffer, LoRaMacTxPayloadLen );
                LoRaMacBufferPktLen = pktHeaderLen + LoRaMacTxPayloadLen;
            }
            break;
        default:
            return LORAMAC_STATUS_SERVICE_UNKNOWN;
    }

    return LORAMAC_STATUS_OK;
}

TimerTime_t SendFrameOnChannel( ChannelParams_t channel )
{
    int8_t datarate = Datarates[LoRaMacParams.ChannelsDatarate];
    int8_t txPowerIndex = 0;
    int8_t txPower = 0;

    txPowerIndex = LimitTxPower( LoRaMacParams.ChannelsTxPower );
    txPower = TxPowers[txPowerIndex];

    if( IsBeaconExpected( ) == true )
    {
        return BeaconCtx.Cfg.Reserved;
    }
    if( IsPingExpected( ) == true )
    {
        return PingSlotCtx.Cfg.PingSlotWindow;
    }

    HaltBeaconing( );


    MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    McpsConfirm.Datarate = LoRaMacParams.ChannelsDatarate;
    McpsConfirm.TxPower = txPowerIndex;

    Radio.SetChannel( channel.Frequency );

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    if( LoRaMacParams.ChannelsDatarate == DR_7 )
    { // High Speed FSK channel
        Radio.SetMaxPayloadLength( MODEM_FSK, LoRaMacBufferPktLen );
        Radio.SetTxConfig( MODEM_FSK, txPower, 25e3, 0, datarate * 1e3, 0, 5, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_FSK, LoRaMacBufferPktLen );

    }
    else if( LoRaMacParams.ChannelsDatarate == DR_6 )
    { // High speed LoRa channel
        Radio.SetMaxPayloadLength( MODEM_LORA, LoRaMacBufferPktLen );
        Radio.SetTxConfig( MODEM_LORA, txPower, 0, 1, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
    else
    { // Normal LoRa channel
        Radio.SetMaxPayloadLength( MODEM_LORA, LoRaMacBufferPktLen );
        Radio.SetTxConfig( MODEM_LORA, txPower, 0, 0, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    Radio.SetMaxPayloadLength( MODEM_LORA, LoRaMacBufferPktLen );
    if( LoRaMacParams.ChannelsDatarate >= DR_4 )
    { // High speed LoRa channel BW500 kHz
        Radio.SetTxConfig( MODEM_LORA, txPower, 0, 2, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
    else
    { // Normal LoRa channel
        Radio.SetTxConfig( MODEM_LORA, txPower, 0, 0, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
        TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
    }
#elif defined( USE_BAND_470 )
    Radio.SetMaxPayloadLength( MODEM_LORA, LoRaMacBufferPktLen );
    Radio.SetTxConfig( MODEM_LORA, txPower, 0, 0, datarate, 1, 8, false, true, 0, 0, false, 3e3 );
    TxTimeOnAir = Radio.TimeOnAir( MODEM_LORA, LoRaMacBufferPktLen );
#else
    #error "Please define a frequency band in the compiler options."
#endif

    // Store the time on air
    McpsConfirm.TxTimeOnAir = TxTimeOnAir;
    MlmeConfirm.TxTimeOnAir = TxTimeOnAir;

    // Starts the MAC layer status check timer
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
    TimerStart( &MacStateCheckTimer );

    LoRaMacState |= MAC_TX_RUNNING;

    // Send now
    Radio.Send( LoRaMacBuffer, LoRaMacBufferPktLen );

    return 0;
}

LoRaMacStatus_t LoRaMacInitialization( LoRaMacPrimitives_t *primitives, LoRaMacCallback_t *callbacks )
{
    if( primitives == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    if( ( primitives->MacMcpsConfirm == NULL ) ||
        ( primitives->MacMcpsIndication == NULL ) ||
        ( primitives->MacMlmeConfirm == NULL ) ||
        ( primitives->MacMlmeIndication == NULL ) )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    LoRaMacPrimitives = primitives;
    LoRaMacCallbacks = callbacks;

    LoRaMacFlags.Value = 0;

    LoRaMacDeviceClass = CLASS_A;
    LoRaMacState = MAC_IDLE;

    JoinRequestTrials = 0;
    RepeaterSupport = false;

    // Reset duty cycle times
    AggregatedLastTxDoneTime = 0;
    AggregatedTimeOff = 0;

    // Duty cycle
#if defined( USE_BAND_433 )
    DutyCycleOn = false;
#elif defined( USE_BAND_470 )
    DutyCycleOn = false;
#elif defined( USE_BAND_780 )
    DutyCycleOn = false;
#elif defined( USE_BAND_868 )
    DutyCycleOn = true;
#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    DutyCycleOn = false;
#else
    #error "Please define a frequency band in the compiler options."
#endif

    // Reset to defaults
    LoRaMacParamsDefaults.ChannelsTxPower = LORAMAC_DEFAULT_TX_POWER;
    LoRaMacParamsDefaults.ChannelsDatarate = LORAMAC_DEFAULT_DATARATE;

    LoRaMacParamsDefaults.MaxRxWindow = MAX_RX_WINDOW;
    LoRaMacParamsDefaults.ReceiveDelay1 = RECEIVE_DELAY1;
    LoRaMacParamsDefaults.ReceiveDelay2 = RECEIVE_DELAY2;
    LoRaMacParamsDefaults.JoinAcceptDelay1 = JOIN_ACCEPT_DELAY1;
    LoRaMacParamsDefaults.JoinAcceptDelay2 = JOIN_ACCEPT_DELAY2;

    LoRaMacParamsDefaults.ChannelsNbRep = 1;
    LoRaMacParamsDefaults.Rx1DrOffset = 0;

    LoRaMacParamsDefaults.Rx2Channel = ( Rx2ChannelParams_t )RX_WND_2_CHANNEL;

    // Channel mask
#if defined( USE_BAND_433 )
    LoRaMacParamsDefaults.ChannelsMask[0] = LC( 1 ) + LC( 2 ) + LC( 3 );
#elif defined ( USE_BAND_470 )
    LoRaMacParamsDefaults.ChannelsMask[0] = 0xFFFF;
    LoRaMacParamsDefaults.ChannelsMask[1] = 0xFFFF;
    LoRaMacParamsDefaults.ChannelsMask[2] = 0xFFFF;
    LoRaMacParamsDefaults.ChannelsMask[3] = 0xFFFF;
    LoRaMacParamsDefaults.ChannelsMask[4] = 0xFFFF;
    LoRaMacParamsDefaults.ChannelsMask[5] = 0xFFFF;
#elif defined( USE_BAND_780 )
    LoRaMacParamsDefaults.ChannelsMask[0] = LC( 1 ) + LC( 2 ) + LC( 3 );
#elif defined( USE_BAND_868 )
    LoRaMacParamsDefaults.ChannelsMask[0] = LC( 1 ) + LC( 2 ) + LC( 3 );
#elif defined( USE_BAND_915 )
    LoRaMacParamsDefaults.ChannelsMask[0] = 0xFFFF;
    LoRaMacParamsDefaults.ChannelsMask[1] = 0xFFFF;
    LoRaMacParamsDefaults.ChannelsMask[2] = 0xFFFF;
    LoRaMacParamsDefaults.ChannelsMask[3] = 0xFFFF;
    LoRaMacParamsDefaults.ChannelsMask[4] = 0x00FF;
    LoRaMacParamsDefaults.ChannelsMask[5] = 0x0000;
#elif defined( USE_BAND_915_HYBRID )
    LoRaMacParamsDefaults.ChannelsMask[0] = 0x00FF;
    LoRaMacParamsDefaults.ChannelsMask[1] = 0x0000;
    LoRaMacParamsDefaults.ChannelsMask[2] = 0x0000;
    LoRaMacParamsDefaults.ChannelsMask[3] = 0x0000;
    LoRaMacParamsDefaults.ChannelsMask[4] = 0x0001;
    LoRaMacParamsDefaults.ChannelsMask[5] = 0x0000;
#else
    #error "Please define a frequency band in the compiler options."
#endif

#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    // 125 kHz channels
    for( uint8_t i = 0; i < LORA_MAX_NB_CHANNELS - 8; i++ )
    {
        Channels[i].Frequency = 902.3e6 + i * 200e3;
        Channels[i].DrRange.Value = ( DR_3 << 4 ) | DR_0;
        Channels[i].Band = 0;
    }
    // 500 kHz channels
    for( uint8_t i = LORA_MAX_NB_CHANNELS - 8; i < LORA_MAX_NB_CHANNELS; i++ )
    {
        Channels[i].Frequency = 903.0e6 + ( i - ( LORA_MAX_NB_CHANNELS - 8 ) ) * 1.6e6;
        Channels[i].DrRange.Value = ( DR_4 << 4 ) | DR_4;
        Channels[i].Band = 0;
    }
#elif defined( USE_BAND_470 )
    // 125 kHz channels
    for( uint8_t i = 0; i < LORA_MAX_NB_CHANNELS; i++ )
    {
        Channels[i].Frequency = 470.3e6 + i * 200e3;
        Channels[i].DrRange.Value = ( DR_5 << 4 ) | DR_0;
        Channels[i].Band = 0;
    }
#endif

    ResetMacParameters( );

    // Initialize timers
    // Beaconing
    BeaconState = BEACON_STATE_ACQUISITION;
    PingSlotState = PINGSLOT_STATE_SET_TIMER;

    memset1( ( uint8_t* ) &PingSlotCtx, 0, sizeof( PingSlotCtx ) );
    memset1( ( uint8_t* ) &BeaconCtx, 0, sizeof( BeaconContext_t ) );

    BeaconCtx.Cfg.Interval = BEACON_INTERVAL;
    BeaconCtx.Cfg.Reserved = BEACON_RESERVED;
    BeaconCtx.Cfg.Guard = BEACON_GUARD;
    BeaconCtx.Cfg.Window = BEACON_WINDOW;
    BeaconCtx.Cfg.WindowSlots = BEACON_WINDOW_SLOTS;
    BeaconCtx.Cfg.SymbolToDefault = BEACON_SYMBOL_TO_DEFAULT;
    BeaconCtx.Cfg.SymbolToExpansionMax = BEACON_SYMBOL_TO_EXPANSION_MAX;
    BeaconCtx.Cfg.SymbolToExpansionFactor = BEACON_SYMBOL_TO_EXPANSION_FACTOR;
    BeaconCtx.Cfg.MaxBeaconLessPeriod = MAX_BEACON_LESS_PERIOD;
    BeaconCtx.Cfg.DelayBeaconTimingAns = BEACON_DELAY_BEACON_TIMING_ANS;

    PingSlotCtx.Cfg.PingSlotWindow = PING_SLOT_WINDOW;
    PingSlotCtx.Cfg.SymbolToExpansionMax = PING_SLOT_SYMBOL_TO_EXPANSION_MAX;
    PingSlotCtx.Cfg.SymbolToExpansionFactor = PING_SLOT_SYMBOL_TO_EXPANSION_FACTOR;

    TimerInit( &BeaconTimer, OnBeaconTimerEvent );
    TimerInit( &PingSlotTimer, OnPingSlotTimerEvent );
    TimerInit( &MulticastSlotTimer, OnMulticastSlotTimerEvent );

    TimerInit( &MacStateCheckTimer, OnMacStateCheckTimerEvent );
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );

    TimerInit( &TxDelayedTimer, OnTxDelayedTimerEvent );
    TimerInit( &RxWindowTimer1, OnRxWindow1TimerEvent );
    TimerInit( &RxWindowTimer2, OnRxWindow2TimerEvent );
    TimerInit( &AckTimeoutTimer, OnAckTimeoutTimerEvent );

    // Initialize Radio driver
    RadioEvents.TxDone = OnRadioTxDone;
    RadioEvents.RxDone = OnRadioRxDone;
    RadioEvents.RxError = OnRadioRxError;
    RadioEvents.TxTimeout = OnRadioTxTimeout;
    RadioEvents.RxTimeout = OnRadioRxTimeout;
    Radio.Init( &RadioEvents );

    // Random seed initialization
    srand1( Radio.Random( ) );

    PublicNetwork = true;
    SetPublicNetwork( PublicNetwork );
    Radio.Sleep( );

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacQueryTxPossible( uint8_t size, LoRaMacTxInfo_t* txInfo )
{
    int8_t datarate = LoRaMacParamsDefaults.ChannelsDatarate;
    uint8_t fOptLen = MacCommandsBufferIndex + MacCommandsBufferToRepeatIndex;

    if( txInfo == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    AdrNextDr( AdrCtrlOn, false, &datarate );

    if( RepeaterSupport == true )
    {
        txInfo->CurrentPayloadSize = MaxPayloadOfDatarateRepeater[datarate];
    }
    else
    {
        txInfo->CurrentPayloadSize = MaxPayloadOfDatarate[datarate];
    }

    if( txInfo->CurrentPayloadSize >= fOptLen )
    {
        txInfo->MaxPossiblePayload = txInfo->CurrentPayloadSize - fOptLen;
    }
    else
    {
        return LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR;
    }

    if( ValidatePayloadLength( size, datarate, 0 ) == false )
    {
        return LORAMAC_STATUS_LENGTH_ERROR;
    }

    if( ValidatePayloadLength( size, datarate, fOptLen ) == false )
    {
        return LORAMAC_STATUS_MAC_CMD_LENGTH_ERROR;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMibGetRequestConfirm( MibRequestConfirm_t *mibGet )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;

    if( mibGet == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    switch( mibGet->Type )
    {
        case MIB_DEVICE_CLASS:
        {
            mibGet->Param.Class = LoRaMacDeviceClass;
            break;
        }
        case MIB_NETWORK_JOINED:
        {
            mibGet->Param.IsNetworkJoined = IsLoRaMacNetworkJoined;
            break;
        }
        case MIB_ADR:
        {
            mibGet->Param.AdrEnable = AdrCtrlOn;
            break;
        }
        case MIB_NET_ID:
        {
            mibGet->Param.NetID = LoRaMacNetID;
            break;
        }
        case MIB_DEV_ADDR:
        {
            mibGet->Param.DevAddr = LoRaMacDevAddr;
            break;
        }
        case MIB_NWK_SKEY:
        {
            mibGet->Param.NwkSKey = LoRaMacNwkSKey;
            break;
        }
        case MIB_APP_SKEY:
        {
            mibGet->Param.AppSKey = LoRaMacAppSKey;
            break;
        }
        case MIB_PUBLIC_NETWORK:
        {
            mibGet->Param.EnablePublicNetwork = PublicNetwork;
            break;
        }
        case MIB_REPEATER_SUPPORT:
        {
            mibGet->Param.EnableRepeaterSupport = RepeaterSupport;
            break;
        }
        case MIB_CHANNELS:
        {
            mibGet->Param.ChannelList = Channels;
            break;
        }
        case MIB_RX2_CHANNEL:
        {
            mibGet->Param.Rx2Channel = LoRaMacParams.Rx2Channel;
            break;
        }
        case MIB_CHANNELS_MASK:
        {
            mibGet->Param.ChannelsMask = LoRaMacParams.ChannelsMask;
            break;
        }
        case MIB_CHANNELS_NB_REP:
        {
            mibGet->Param.ChannelNbRep = LoRaMacParams.ChannelsNbRep;
            break;
        }
        case MIB_MAX_RX_WINDOW_DURATION:
        {
            mibGet->Param.MaxRxWindow = LoRaMacParams.MaxRxWindow;
            break;
        }
        case MIB_RECEIVE_DELAY_1:
        {
            mibGet->Param.ReceiveDelay1 = LoRaMacParams.ReceiveDelay1;
            break;
        }
        case MIB_RECEIVE_DELAY_2:
        {
            mibGet->Param.ReceiveDelay2 = LoRaMacParams.ReceiveDelay2;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_1:
        {
            mibGet->Param.JoinAcceptDelay1 = LoRaMacParams.JoinAcceptDelay1;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_2:
        {
            mibGet->Param.JoinAcceptDelay2 = LoRaMacParams.JoinAcceptDelay2;
            break;
        }
        case MIB_CHANNELS_DEFAULT_DATARATE:
        {
            mibGet->Param.ChannelsDefaultDatarate = LoRaMacParamsDefaults.ChannelsDatarate;
            break;
        }
        case MIB_CHANNELS_DATARATE:
        {
            mibGet->Param.ChannelsDatarate = LoRaMacParams.ChannelsDatarate;
            break;
        }
        case MIB_CHANNELS_DEFAULT_TX_POWER:
        {
            mibGet->Param.ChannelsDefaultTxPower = LoRaMacParamsDefaults.ChannelsTxPower;
            break;
        }
        case MIB_CHANNELS_TX_POWER:
        {
            mibGet->Param.ChannelsTxPower = LoRaMacParams.ChannelsTxPower;
            break;
        }
        case MIB_UPLINK_COUNTER:
        {
            mibGet->Param.UpLinkCounter = UpLinkCounter;
            break;
        }
        case MIB_DOWNLINK_COUNTER:
        {
            mibGet->Param.DownLinkCounter = DownLinkCounter;
            break;
        }
        case MIB_MULTICAST_CHANNEL:
        {
            mibGet->Param.MulticastList = MulticastChannels;
            break;
        }
        case MIB_BEACON_INTERVAL:
        {
            mibGet->Param.BeaconInterval = BeaconCtx.Cfg.Interval;
            break;
        }
        case MIB_BEACON_RESERVED:
        {
            mibGet->Param.BeaconReserved = BeaconCtx.Cfg.Reserved;
            break;
        }
        case MIB_BEACON_GUARD:
        {
            mibGet->Param.BeaconGuard = BeaconCtx.Cfg.Guard;
            break;
        }
        case MIB_BEACON_WINDOW:
        {
            mibGet->Param.BeaconWindow = BeaconCtx.Cfg.Window;
            break;
        }
        case MIB_BEACON_WINDOW_SLOTS:
        {
            mibGet->Param.BeaconWindowSlots = BeaconCtx.Cfg.WindowSlots;
            break;
        }
        case MIB_PING_SLOT_WINDOW:
        {
            mibGet->Param.PingSlotWindow = PingSlotCtx.Cfg.PingSlotWindow;
            break;
        }
        case MIB_BEACON_SYMBOL_TO_DEFAULT:
        {
            mibGet->Param.BeaconSymbolToDefault = BeaconCtx.Cfg.SymbolToDefault;
            break;
        }
        case MIB_BEACON_SYMBOL_TO_EXPANSION_MAX:
        {
            mibGet->Param.BeaconSymbolToExpansionMax = BeaconCtx.Cfg.SymbolToExpansionMax;
            break;
        }
        case MIB_PING_SLOT_SYMBOL_TO_EXPANSION_MAX:
        {
            mibGet->Param.PingSlotSymbolToExpansionMax = PingSlotCtx.Cfg.SymbolToExpansionMax;
            break;
        }
        case MIB_BEACON_SYMBOL_TO_EXPANSION_FACTOR:
        {
            mibGet->Param.BeaconSymbolToExpansionFactor = BeaconCtx.Cfg.SymbolToExpansionFactor;
            break;
        }
        case MIB_PING_SLOT_SYMBOL_TO_EXPANSION_FACTOR:
        {
            mibGet->Param.PingSlotSymbolToExpansionFactor = PingSlotCtx.Cfg.SymbolToExpansionFactor;
            break;
        }
        case MIB_MAX_BEACON_LESS_PERIOD:
        {
            mibGet->Param.MaxBeaconLessPeriod = BeaconCtx.Cfg.MaxBeaconLessPeriod;
            break;
        }
        default:
            status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            break;
    }

    return status;
}

LoRaMacStatus_t LoRaMacMibSetRequestConfirm( MibRequestConfirm_t *mibSet )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;

    if( mibSet == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    switch( mibSet->Type )
    {
        case MIB_NETWORK_JOINED:
        {
            IsLoRaMacNetworkJoined = mibSet->Param.IsNetworkJoined;
            break;
        }
        case MIB_ADR:
        {
            AdrCtrlOn = mibSet->Param.AdrEnable;
            break;
        }
        case MIB_NET_ID:
        {
            LoRaMacNetID = mibSet->Param.NetID;
            break;
        }
        case MIB_DEV_ADDR:
        {
            LoRaMacDevAddr = mibSet->Param.DevAddr;
            break;
        }
        case MIB_NWK_SKEY:
        {
            if( mibSet->Param.NwkSKey != NULL )
            {
                memcpy1( LoRaMacNwkSKey, mibSet->Param.NwkSKey,
                               sizeof( LoRaMacNwkSKey ) );
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_APP_SKEY:
        {
            if( mibSet->Param.AppSKey != NULL )
            {
                memcpy1( LoRaMacAppSKey, mibSet->Param.AppSKey,
                               sizeof( LoRaMacAppSKey ) );
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_PUBLIC_NETWORK:
        {
            SetPublicNetwork( mibSet->Param.EnablePublicNetwork );
            break;
        }
        case MIB_REPEATER_SUPPORT:
        {
             RepeaterSupport = mibSet->Param.EnableRepeaterSupport;
            break;
        }
        case MIB_RX2_CHANNEL:
        {
            LoRaMacParams.Rx2Channel = mibSet->Param.Rx2Channel;
            break;
        }
        case MIB_CHANNELS_MASK:
        {
            if( mibSet->Param.ChannelsMask )
            {
#if defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
                bool chanMaskState = true;

#if defined( USE_BAND_915_HYBRID )
                chanMaskState = ValidateChannelMask( mibSet->Param.ChannelsMask );
#endif
                if( chanMaskState == true )
                {
                    if( ( CountNbEnabled125kHzChannels( mibSet->Param.ChannelsMask ) < 2 ) &&
                        ( CountNbEnabled125kHzChannels( mibSet->Param.ChannelsMask ) > 0 ) )
                    {
                        status = LORAMAC_STATUS_PARAMETER_INVALID;
                    }
                    else
                    {
                        memcpy1( ( uint8_t* ) LoRaMacParams.ChannelsMask,
                                 ( uint8_t* ) mibSet->Param.ChannelsMask, sizeof( LoRaMacParams.ChannelsMask ) );
                        for ( uint8_t i = 0; i < sizeof( LoRaMacParams.ChannelsMask ) / 2; i++ )
                        {
                            // Disable channels which are no longer available
                            ChannelsMaskRemaining[i] &= LoRaMacParams.ChannelsMask[i];
                        }
                    }
                }
                else
                {
                    status = LORAMAC_STATUS_PARAMETER_INVALID;
                }
#elif defined( USE_BAND_470 )
                memcpy1( ( uint8_t* ) LoRaMacParams.ChannelsMask,
                         ( uint8_t* ) mibSet->Param.ChannelsMask, sizeof( LoRaMacParams.ChannelsMask ) );
#else
                memcpy1( ( uint8_t* ) LoRaMacParams.ChannelsMask,
                         ( uint8_t* ) mibSet->Param.ChannelsMask, 2 );
#endif
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_NB_REP:
        {
            if( ( mibSet->Param.ChannelNbRep >= 1 ) &&
                ( mibSet->Param.ChannelNbRep <= 15 ) )
            {
                LoRaMacParams.ChannelsNbRep = mibSet->Param.ChannelNbRep;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_MAX_RX_WINDOW_DURATION:
        {
            LoRaMacParams.MaxRxWindow = mibSet->Param.MaxRxWindow;
            break;
        }
        case MIB_RECEIVE_DELAY_1:
        {
            LoRaMacParams.ReceiveDelay1 = mibSet->Param.ReceiveDelay1;
            break;
        }
        case MIB_RECEIVE_DELAY_2:
        {
            LoRaMacParams.ReceiveDelay2 = mibSet->Param.ReceiveDelay2;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_1:
        {
            LoRaMacParams.JoinAcceptDelay1 = mibSet->Param.JoinAcceptDelay1;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_2:
        {
            LoRaMacParams.JoinAcceptDelay2 = mibSet->Param.JoinAcceptDelay2;
            break;
        }
        case MIB_CHANNELS_DEFAULT_DATARATE:
        {
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
            if( ValueInRange( mibSet->Param.ChannelsDefaultDatarate,
                              DR_0, DR_5 ) )
            {
                LoRaMacParamsDefaults.ChannelsDatarate = mibSet->Param.ChannelsDefaultDatarate;
            }
#else
            if( ValueInRange( mibSet->Param.ChannelsDefaultDatarate,
                              LORAMAC_TX_MIN_DATARATE, LORAMAC_TX_MAX_DATARATE ) )
            {
                LoRaMacParamsDefaults.ChannelsDatarate = mibSet->Param.ChannelsDefaultDatarate;
            }
#endif
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_DATARATE:
        {
            if( ValueInRange( mibSet->Param.ChannelsDatarate,
                              LORAMAC_TX_MIN_DATARATE, LORAMAC_TX_MAX_DATARATE ) )
            {
                LoRaMacParams.ChannelsDatarate = mibSet->Param.ChannelsDatarate;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_DEFAULT_TX_POWER:
        {
            if( ValueInRange( mibSet->Param.ChannelsDefaultTxPower,
                              LORAMAC_MIN_TX_POWER, LORAMAC_MAX_TX_POWER ) )
            {
                LoRaMacParamsDefaults.ChannelsTxPower = mibSet->Param.ChannelsDefaultTxPower;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_TX_POWER:
        {
            if( ValueInRange( mibSet->Param.ChannelsTxPower,
                              LORAMAC_MAX_TX_POWER, LORAMAC_MIN_TX_POWER ) )
            {
                LoRaMacParams.ChannelsTxPower = mibSet->Param.ChannelsTxPower;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_UPLINK_COUNTER:
        {
            UpLinkCounter = mibSet->Param.UpLinkCounter;
            break;
        }
        case MIB_DOWNLINK_COUNTER:
        {
            DownLinkCounter = mibSet->Param.DownLinkCounter;
            break;
        }
        case MIB_BEACON_INTERVAL:
        {
            BeaconCtx.Cfg.Interval = mibSet->Param.BeaconInterval;
            break;
        }
        case MIB_BEACON_RESERVED:
        {
            BeaconCtx.Cfg.Reserved = mibSet->Param.BeaconReserved;
            break;
        }
        case MIB_BEACON_GUARD:
        {
            BeaconCtx.Cfg.Guard = mibSet->Param.BeaconGuard;
            break;
        }
        case MIB_BEACON_WINDOW:
        {
            BeaconCtx.Cfg.Window = mibSet->Param.BeaconWindow;
            break;
        }
        case MIB_BEACON_WINDOW_SLOTS:
        {
            BeaconCtx.Cfg.WindowSlots = mibSet->Param.BeaconWindowSlots;
            break;
        }
        case MIB_PING_SLOT_WINDOW:
        {
            PingSlotCtx.Cfg.PingSlotWindow = mibSet->Param.PingSlotWindow;
            break;
        }
        case MIB_BEACON_SYMBOL_TO_DEFAULT:
        {
            BeaconCtx.Cfg.SymbolToDefault = mibSet->Param.BeaconSymbolToDefault;
            break;
        }
        case MIB_BEACON_SYMBOL_TO_EXPANSION_MAX:
        {
            BeaconCtx.Cfg.SymbolToExpansionMax = mibSet->Param.BeaconSymbolToExpansionMax;
            break;
        }
        case MIB_PING_SLOT_SYMBOL_TO_EXPANSION_MAX:
        {
            PingSlotCtx.Cfg.SymbolToExpansionMax = mibSet->Param.PingSlotSymbolToExpansionMax;
            break;
        }
        case MIB_BEACON_SYMBOL_TO_EXPANSION_FACTOR:
        {
            BeaconCtx.Cfg.SymbolToExpansionFactor = mibSet->Param.BeaconSymbolToExpansionFactor;
            break;
        }
        case MIB_PING_SLOT_SYMBOL_TO_EXPANSION_FACTOR:
        {
            PingSlotCtx.Cfg.SymbolToExpansionFactor = mibSet->Param.PingSlotSymbolToExpansionFactor;
            break;
        }
        case MIB_MAX_BEACON_LESS_PERIOD:
        {
            BeaconCtx.Cfg.MaxBeaconLessPeriod = mibSet->Param.MaxBeaconLessPeriod;
            break;
        }
        default:
            status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            break;
    }

    return status;
}

LoRaMacStatus_t LoRaMacChannelAdd( uint8_t id, ChannelParams_t params )
{
#if defined( USE_BAND_470 ) || defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )
    return LORAMAC_STATUS_PARAMETER_INVALID;
#else
    bool datarateInvalid = false;
    bool frequencyInvalid = false;
    uint8_t band = 0;

    // The id must not exceed LORA_MAX_NB_CHANNELS
    if( id >= LORA_MAX_NB_CHANNELS )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    // Validate if the MAC is in a correct state
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        if( ( LoRaMacState & MAC_TX_CONFIG ) != MAC_TX_CONFIG )
        {
            return LORAMAC_STATUS_BUSY;
        }
    }
    // Validate the datarate
    if( ValidateDrRange( params.DrRange, LORAMAC_TX_MIN_DATARATE, LORAMAC_TX_MAX_DATARATE ) == false )
    {
        datarateInvalid = true;
    }

#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    if( id < 3 )
    {
        if( params.Frequency != Channels[id].Frequency )
        {
            frequencyInvalid = true;
        }

        if( params.DrRange.Fields.Min > LoRaMacParamsDefaults.ChannelsDatarate )
        {
            datarateInvalid = true;
        }
        if( ValueInRange( params.DrRange.Fields.Max, DR_5, LORAMAC_TX_MAX_DATARATE ) == false )
        {
            datarateInvalid = true;
        }
    }
#endif

    // Validate the frequency
    if( ( Radio.CheckRfFrequency( params.Frequency ) == true ) && ( params.Frequency > 0 ) && ( frequencyInvalid == false ) )
    {
#if defined( USE_BAND_868 )
        if( ( params.Frequency >= 865000000 ) && ( params.Frequency <= 868000000 ) )
        {
            band = BAND_G1_0;
        }
        else if( ( params.Frequency > 868000000 ) && ( params.Frequency <= 868600000 ) )
        {
            band = BAND_G1_1;
        }
        else if( ( params.Frequency >= 868700000 ) && ( params.Frequency <= 869200000 ) )
        {
            band = BAND_G1_2;
        }
        else if( ( params.Frequency >= 869400000 ) && ( params.Frequency <= 869650000 ) )
        {
            band = BAND_G1_3;
        }
        else if( ( params.Frequency >= 869700000 ) && ( params.Frequency <= 870000000 ) )
        {
            band = BAND_G1_4;
        }
        else
        {
            frequencyInvalid = true;
        }
#endif
    }
    else
    {
        frequencyInvalid = true;
    }

    if( ( datarateInvalid == true ) && ( frequencyInvalid == true ) )
    {
        return LORAMAC_STATUS_FREQ_AND_DR_INVALID;
    }
    if( datarateInvalid == true )
    {
        return LORAMAC_STATUS_DATARATE_INVALID;
    }
    if( frequencyInvalid == true )
    {
        return LORAMAC_STATUS_FREQUENCY_INVALID;
    }

    // Every parameter is valid, activate the channel
    Channels[id] = params;
    Channels[id].Band = band;
    LoRaMacParams.ChannelsMask[0] |= ( 1 << id );

    return LORAMAC_STATUS_OK;
#endif
}

LoRaMacStatus_t LoRaMacChannelRemove( uint8_t id )
{
#if defined( USE_BAND_433 ) || defined( USE_BAND_780 ) || defined( USE_BAND_868 )
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        if( ( LoRaMacState & MAC_TX_CONFIG ) != MAC_TX_CONFIG )
        {
            return LORAMAC_STATUS_BUSY;
        }
    }

    if( ( id < 3 ) || ( id >= LORA_MAX_NB_CHANNELS ) )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    else
    {
        // Remove the channel from the list of channels
        Channels[id] = ( ChannelParams_t ){ 0, { 0 }, 0 };

        // Disable the channel as it doesn't exist anymore
        if( DisableChannelInMask( id, LoRaMacParams.ChannelsMask ) == false )
        {
            return LORAMAC_STATUS_PARAMETER_INVALID;
        }
    }
    return LORAMAC_STATUS_OK;
#elif ( defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID ) || defined( USE_BAND_470 ) )
    return LORAMAC_STATUS_PARAMETER_INVALID;
#endif
}

LoRaMacStatus_t LoRaMacMulticastChannelLink( MulticastParams_t *channelParam )
{
    if( channelParam == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    // Reset downlink counter
    channelParam->DownLinkCounter = 0;
    channelParam->Next = NULL;

    if( MulticastChannels == NULL )
    {
        // New node is the fist element
        MulticastChannels = channelParam;
    }
    else
    {
        MulticastParams_t *cur = MulticastChannels;

        // Search the last node in the list
        while( cur->Next != NULL )
        {
            cur = cur->Next;
        }
        // This function always finds the last node
        cur->Next = channelParam;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMulticastChannelUnlink( MulticastParams_t *channelParam )
{
    if( channelParam == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    if( MulticastChannels != NULL )
    {
        if( MulticastChannels == channelParam )
        {
          // First element
          MulticastChannels = channelParam->Next;
        }
        else
        {
            MulticastParams_t *cur = MulticastChannels;

            // Search the node in the list
            while( cur->Next && cur->Next != channelParam )
            {
                cur = cur->Next;
            }
            // If we found the node, remove it
            if( cur->Next )
            {
                cur->Next = channelParam->Next;
            }
        }
        channelParam->Next = NULL;
    }

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMlmeRequest( MlmeReq_t *mlmeRequest )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_SERVICE_UNKNOWN;
    LoRaMacHeader_t macHdr;

    if( mlmeRequest == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    if( MlmeConfirmQueueCnt == 0 )
    {
        memset1( ( uint8_t* ) &MlmeConfirm, 0, sizeof( MlmeConfirm ) );
    }

    MlmeConfirmQueue[MlmeConfirmQueueCnt].Status = LORAMAC_EVENT_INFO_STATUS_ERROR;

    switch( mlmeRequest->Type )
    {
        case MLME_JOIN:
        {
            if( ( LoRaMacState & MAC_TX_DELAYED ) == MAC_TX_DELAYED )
            {
                return LORAMAC_STATUS_BUSY;
            }

            if( ( mlmeRequest->Req.Join.DevEui == NULL ) ||
                ( mlmeRequest->Req.Join.AppEui == NULL ) ||
                ( mlmeRequest->Req.Join.AppKey == NULL ) )
            {
                return LORAMAC_STATUS_PARAMETER_INVALID;
            }

            LoRaMacFlags.Bits.MlmeReq = 1;

            LoRaMacDevEui = mlmeRequest->Req.Join.DevEui;
            LoRaMacAppEui = mlmeRequest->Req.Join.AppEui;
            LoRaMacAppKey = mlmeRequest->Req.Join.AppKey;

            macHdr.Value = 0;
            macHdr.Bits.MType  = FRAME_TYPE_JOIN_REQ;

            ResetMacParameters( );

            MlmeConfirmQueue[MlmeConfirmQueueCnt].MlmeRequest = mlmeRequest->Type;

            JoinRequestTrials++;
            LoRaMacParams.ChannelsDatarate = AlternateDatarate( JoinRequestTrials );

            status = Send( &macHdr, 0, NULL, 0 );
            break;
        }
        case MLME_LINK_CHECK:
        {
            LoRaMacFlags.Bits.MlmeReq = 1;
            // LoRaMac will send this command piggy-pack
            MlmeConfirmQueue[MlmeConfirmQueueCnt].MlmeRequest = mlmeRequest->Type;

            status = AddMacCommand( MOTE_MAC_LINK_CHECK_REQ, 0, 0 );
            break;
        }
        case MLME_PING_SLOT_INFO:
        {
            uint8_t value = mlmeRequest->Req.PingSlotInfo.PingSlot.Value;

            LoRaMacFlags.Bits.MlmeReq = 1;
            // LoRaMac will send this command piggy-pack
            MlmeConfirmQueue[MlmeConfirmQueueCnt].MlmeRequest = mlmeRequest->Type;

            PingSlotCtx.PingNb = 128 / ( 1 << mlmeRequest->Req.PingSlotInfo.PingSlot.Fields.Periodicity );
            PingSlotCtx.PingPeriod = BeaconCtx.Cfg.WindowSlots / PingSlotCtx.PingNb;

            PingSlotCtx.DrRange.Fields.Max = mlmeRequest->Req.PingSlotInfo.PingSlot.Fields.Datarate;
            PingSlotCtx.DrRange.Fields.Min = mlmeRequest->Req.PingSlotInfo.PingSlot.Fields.Datarate;
            PingSlotCtx.Datarate = mlmeRequest->Req.PingSlotInfo.PingSlot.Fields.Datarate;

            status = AddMacCommand( MOTE_MAC_PING_SLOT_INFO_REQ, value, 0 );
            break;
        }
        case MLME_BEACON_TIMING:
        {
            LoRaMacFlags.Bits.MlmeReq = 1;
            // LoRaMac will send this command piggy-pack
            MlmeConfirmQueue[MlmeConfirmQueueCnt].MlmeRequest = mlmeRequest->Type;

            status = AddMacCommand( MOTE_MAC_BEACON_TIMING_REQ, 0, 0 );
            break;
        }
        case MLME_SWITCH_CLASS:
        {
            LoRaMacFlags.Bits.MlmeReq = 1;
                MlmeConfirmQueue[MlmeConfirmQueueCnt].MlmeRequest = mlmeRequest->Type;


            status = SwitchClass( mlmeRequest->Req.SwitchClass.Class );
            break;
        }
        default:
            break;
    }

    if( status != LORAMAC_STATUS_OK )
    {
        NodeAckRequested = false;
        if( MlmeConfirmQueueCnt == 0 )
        {
            LoRaMacFlags.Bits.MlmeReq = 0;
        }
    }
    else
    {
        MlmeConfirmQueueCnt++;
    }

    return status;
}

LoRaMacStatus_t LoRaMacMcpsRequest( McpsReq_t *mcpsRequest )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_SERVICE_UNKNOWN;
    LoRaMacHeader_t macHdr;
    uint8_t fPort = 0;
    void *fBuffer;
    uint16_t fBufferSize;
    int8_t datarate;
    bool readyToSend = false;

    if( mcpsRequest == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( ( LoRaMacState & MAC_TX_RUNNING ) == MAC_TX_RUNNING ) ||
        ( ( LoRaMacState & MAC_TX_DELAYED ) == MAC_TX_DELAYED ) )
    {
        return LORAMAC_STATUS_BUSY;
    }

    macHdr.Value = 0;
    memset1 ( ( uint8_t* ) &McpsConfirm, 0, sizeof( McpsConfirm ) );
    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;

    switch( mcpsRequest->Type )
    {
        case MCPS_UNCONFIRMED:
        {
            readyToSend = true;
            AckTimeoutRetries = 1;

            macHdr.Bits.MType = FRAME_TYPE_DATA_UNCONFIRMED_UP;
            fPort = mcpsRequest->Req.Unconfirmed.fPort;
            fBuffer = mcpsRequest->Req.Unconfirmed.fBuffer;
            fBufferSize = mcpsRequest->Req.Unconfirmed.fBufferSize;
            datarate = mcpsRequest->Req.Unconfirmed.Datarate;
            break;
        }
        case MCPS_CONFIRMED:
        {
            readyToSend = true;
            AckTimeoutRetriesCounter = 1;
            AckTimeoutRetries = mcpsRequest->Req.Confirmed.NbTrials;

            macHdr.Bits.MType = FRAME_TYPE_DATA_CONFIRMED_UP;
            fPort = mcpsRequest->Req.Confirmed.fPort;
            fBuffer = mcpsRequest->Req.Confirmed.fBuffer;
            fBufferSize = mcpsRequest->Req.Confirmed.fBufferSize;
            datarate = mcpsRequest->Req.Confirmed.Datarate;
            break;
        }
        case MCPS_PROPRIETARY:
        {
            readyToSend = true;
            AckTimeoutRetries = 1;

            macHdr.Bits.MType = FRAME_TYPE_PROPRIETARY;
            fBuffer = mcpsRequest->Req.Proprietary.fBuffer;
            fBufferSize = mcpsRequest->Req.Proprietary.fBufferSize;
            datarate = mcpsRequest->Req.Proprietary.Datarate;
            break;
        }
        default:
            break;
    }

    if( readyToSend == true )
    {
        if( AdrCtrlOn == false )
        {
            if( ValueInRange( datarate, LORAMAC_TX_MIN_DATARATE, LORAMAC_TX_MAX_DATARATE ) == true )
            {
                LoRaMacParams.ChannelsDatarate = datarate;
            }
            else
            {
                return LORAMAC_STATUS_PARAMETER_INVALID;
            }
        }

        status = Send( &macHdr, fPort, fBuffer, fBufferSize );
        if( status == LORAMAC_STATUS_OK )
        {
            McpsConfirm.McpsRequest = mcpsRequest->Type;
            LoRaMacFlags.Bits.McpsReq = 1;
        }
        else
        {
            NodeAckRequested = false;
        }
    }

    return status;
}

void LoRaMacTestRxWindowsOn( bool enable )
{
    IsRxWindowsEnabled = enable;
}

void LoRaMacTestSetMic( uint16_t txPacketCounter )
{
    UpLinkCounter = txPacketCounter;
    IsUpLinkCounterFixed = true;
}

void LoRaMacTestSetDutyCycleOn( bool enable )
{
#if defined( USE_BAND_868 )
    DutyCycleOn = enable;
#else
    DutyCycleOn = false;
#endif
}
