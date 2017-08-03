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

Maintainer: Miguel Luis ( Semtech ), Gregory Cristian ( Semtech ) and Daniel Jaeckle ( STACKFORCE )
*/
#include "board.h"

#include "LoRaMac.h"
#include "region/Region.h"
#include "LoRaMacCrypto.h"
#include "LoRaMacTest.h"



/*!
 * Maximum PHY layer payload size
 */
#define LORAMAC_PHY_MAXPAYLOAD                      255

/*!
 * Maximum MAC commands buffer size
 */
#define LORA_MAC_COMMAND_MAX_LENGTH                 128

/*!
 * Maximum length of the fOpts field
 */
#define LORA_MAC_COMMAND_MAX_FOPTS_LENGTH           15

/*!
 * LoRaMac duty cycle for the back-off procedure during the first hour.
 */
#define BACKOFF_DC_1_HOUR                           100

/*!
 * LoRaMac duty cycle for the back-off procedure during the next 10 hours.
 */
#define BACKOFF_DC_10_HOURS                         1000

/*!
 * LoRaMac duty cycle for the back-off procedure during the next 24 hours.
 */
#define BACKOFF_DC_24_HOURS                         10000

/*!
 * LoRaMac region.
 */
static LoRaMacRegion_t LoRaMacRegion;

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
static uint8_t LoRaMacRxPayload[LORAMAC_PHY_MAXPAYLOAD];

/*!
 * LoRaMAC frame counter. Each time a packet is sent the counter is incremented.
 * Only the 16 LSB bits are sent
 */
static uint32_t UpLinkCounter = 0;

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
 * Current channel index
 */
static uint8_t LastTxChannel;

/*!
 * Set to true, if the last uplink was a join request
 */
static bool LastTxIsJoinRequest;

/*!
 * Stores the time at LoRaMac initialization.
 *
 * \remark Used for the BACKOFF_DC computation.
 */
static TimerTime_t LoRaMacInitializationTime = 0;

/*!
 * Last channel index
 */
static uint8_t LastTxChannel = 0;

/*!
 * LoRaMac internal states
 */
enum eLoRaMacState
{
    LORAMAC_IDLE          = 0x00000000,
    LORAMAC_TX_RUNNING    = 0x00000001,
    LORAMAC_RX            = 0x00000002,
    LORAMAC_ACK_REQ       = 0x00000004,
    LORAMAC_ACK_RETRY     = 0x00000008,
    LORAMAC_TX_DELAYED    = 0x00000010,
    LORAMAC_TX_CONFIG     = 0x00000020,
    LORAMAC_RX_ABORT      = 0x00000040,
};

/*!
 * LoRaMac internal state
 */
uint32_t LoRaMacState = LORAMAC_IDLE;

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
 * LoRaMac Rx windows configuration
 */
static RxConfigParams_t RxWindow1Config;
static RxConfigParams_t RxWindow2Config;

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
static uint8_t JoinRequestTrials;

/*!
 * Maximum number of trials for the Join Request
 */
static uint8_t MaxJoinRequestTrials;

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
     * Handles the state when the beacon was missed due to an uplink
     */
    BEACON_STATE_BEACON_MISSED,
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
         * Set if the node receives beacons
         */
        uint8_t BeaconMode          : 1;
        /*!
         * Set if the node has acquired the beacon
         */
        uint8_t BeaconAcquired      : 1;
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
        /*!
         * Set if beacon timer is set to acquire a beacon
         */
        uint8_t AcquisitionTimerSet  : 1;
        /*!
         * Set if the beacon state machine will be resumed
         */
        uint8_t ResumeBeaconing      : 1;
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
 * \param [IN] rxContinuous Set to true, if the RX is in continuous mode
 * \param [IN] maxRxWindow Maximum RX window timeout
 */
static void RxWindowSetup( bool rxContinuous, uint32_t maxRxWindow );

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
 * \brief Calculates the back-off time for the band of a channel.
 *
 * \param [IN] channel     The last Tx channel index
 */
static void CalculateBackOff( uint8_t channel );

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
 * \param [IN] channel     Channel to transmit on
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t SendFrameOnChannel( uint8_t channel );

/*!
 * \brief Sets the radio in continuous transmission mode
 *
 * \remark Uses the radio parameters set on the previous transmission.
 *
 * \param [IN] timeout     Time in seconds while the radio is kept in continuous wave mode
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t SetTxContinuousWave( uint16_t timeout );

/*!
 * \brief Sets the radio in continuous transmission mode
 *
 * \remark Uses the radio parameters set on the previous transmission.
 *
 * \param [IN] timeout     Time in seconds while the radio is kept in continuous wave mode
 * \param [IN] frequency   RF frequency to be set.
 * \param [IN] power       RF ouptut power to be set.
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t SetTxContinuousWave1( uint16_t timeout, uint32_t frequency, uint8_t power );

/*!
 * \brief Resets MAC specific parameters to default
 */
static void ResetMacParameters( void );

static void OnRadioTxDone( void )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    SetBandTxDoneParams_t txDone;
    TimerTime_t curTime = TimerGetCurrentTime( );

    if( LoRaMacDeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OnRxWindow2TimerEvent( );
    }

    // Setup timers
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
            getPhy.Attribute = PHY_ACK_TIMEOUT;
            phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
            TimerSetValue( &AckTimeoutTimer, RxWindow2Delay + phyParam.Value );
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

    // Verify if the last uplink was a join request
    if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( MlmeConfirm.MlmeRequest == MLME_JOIN ) )
    {
        LastTxIsJoinRequest = true;
    }
    else
    {
        LastTxIsJoinRequest = false;
    }

    // Store last Tx channel
    LastTxChannel = Channel;
    // Update last tx done time for the current channel
    txDone.Channel = Channel;
    txDone.Joined = IsLoRaMacNetworkJoined;
    txDone.LastTxDoneTime = curTime;
    RegionSetBandTxDone( LoRaMacRegion, &txDone );
    // Update Aggregated last tx done time
    AggregatedLastTxDoneTime = curTime;

    if( NodeAckRequested == false )
    {
        McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
        ChannelsNbRepCounter++;
    }
}

static void PrepareRxDoneAbort( void )
{
    LoRaMacState |= LORAMAC_RX_ABORT;

    if( NodeAckRequested )
    {
        OnAckTimeoutTimerEvent( );
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
    ApplyCFListParams_t applyCFList;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    bool skipIndication = false;

    uint8_t index = 0;

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

    Radio.Sleep( );
    TimerStop( &RxWindowTimer2 );

    if( RxBeacon( payload, size ) == true )
    {
        return;
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
                LoRaMacParams.ReceiveDelay1 *= 1000;
                LoRaMacParams.ReceiveDelay2 = LoRaMacParams.ReceiveDelay1 + 1000;

                // Apply CF list
                applyCFList.Payload = &LoRaMacRxPayload[13];
                // Size of the regular payload is 12. Plus 1 byte MHDR and 4 bytes MIC
                applyCFList.Size = size - 17;

                RegionApplyCFList( LoRaMacRegion, &applyCFList );

                MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                IsLoRaMacNetworkJoined = true;
                LoRaMacParams.ChannelsDatarate = LoRaMacParamsDefaults.ChannelsDatarate;
            }
            break;
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
            {
                // Check if the received payload size is valid
                getPhy.UplinkDwellTime = LoRaMacParams.DownlinkDwellTime;
                getPhy.Datarate = McpsIndication.RxDatarate;
                getPhy.Attribute = PHY_MAX_PAYLOAD;

                // Get the maximum payload length
                if( RepeaterSupport == true )
                {
                    getPhy.Attribute = PHY_MAX_PAYLOAD_REPEATER;
                }
                phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
                if( MAX( 0, ( int16_t )( ( int16_t )size - ( int16_t )LORA_MAC_FRMPAYLOAD_OVERHEAD ) ) > phyParam.Value )
                {
                    McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
                    PrepareRxDoneAbort( );
                    return;
                }

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
                getPhy.Attribute = PHY_MAX_FCNT_GAP;
                phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
                if( sequenceCounterDiff >= phyParam.Value )
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
                                // In this case, the MAC layer shall accept the MAC commands
                                // which are included in the downlink retransmission.
                                // It should not provide the same frame to the application
                                // layer again.
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

                    // This must be done before parsing the payload and the MAC commands.
                    // We need to reset the MacCommandsBufferIndex here, since we need
                    // to take retransmissions and repetitions into account. Error cases
                    // will be handled in function OnMacStateCheckTimerEvent.
                    if( McpsConfirm.McpsRequest == MCPS_CONFIRMED )
                    {
                        if( fCtrl.Bits.Ack == 1 )
                        {// Reset MacCommandsBufferIndex when we have received an ACK.
                            MacCommandsBufferIndex = 0;
                        }
                    }
                    else
                    {// Reset the variable if we have received any valid frame.
                        MacCommandsBufferIndex = 0;
                    }

                    // Process payload and MAC commands
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
                    }
                    // Provide always an indication, skip the callback to the user application,
                    // in case of a confirmed downlink retransmission.
                    LoRaMacFlags.Bits.McpsInd = 1;
                    LoRaMacFlags.Bits.McpsIndSkip = skipIndication;
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
        if( RxSlot == 0 )
        {
            if( NodeAckRequested == true )
            {
                McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX1_ERROR;
            }
            MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX1_ERROR;
            if( TimerGetElapsedTime( AggregatedLastTxDoneTime ) >= RxWindow2Delay )
            {
                LoRaMacFlags.Bits.MacDone = 1;
            }
        }
        else
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

    if( RxSlot == 0 )
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
            McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT;
        }
        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT;

        if( TimerGetElapsedTime( AggregatedLastTxDoneTime ) >= RxWindow2Delay )
        {
            LoRaMacFlags.Bits.MacDone = 1;
        }
    }
    else
    {
        if( NodeAckRequested == true )
        {
            McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;
        }
        MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;

        if( LoRaMacDeviceClass != CLASS_C )
        {
            LoRaMacFlags.Bits.MacDone = 1;
        }
    }
}

static void OnMacStateCheckTimerEvent( void )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;

    TimerStop( &MacStateCheckTimer );

    if( LoRaMacFlags.Bits.MacDone == 1 )
    {
        if( ( LoRaMacState & LORAMAC_RX_ABORT ) == LORAMAC_RX_ABORT )
        {
            LoRaMacState &= ~LORAMAC_RX_ABORT;
            LoRaMacState &= ~LORAMAC_TX_RUNNING;
        }

        if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) || ( ( LoRaMacFlags.Bits.McpsReq == 1 ) ) )
        {
            if( ( McpsConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) ||
                ( MlmeConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) )
            {
                // Stop transmit cycle due to tx timeout.
                LoRaMacState &= ~LORAMAC_TX_RUNNING;
                MacCommandsBufferIndex = 0;
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
                    LoRaMacState &= ~LORAMAC_TX_RUNNING;
                }
            }
        }

        if( ( NodeAckRequested == false ) && ( noTx == false ) )
        {
            if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) || ( ( LoRaMacFlags.Bits.McpsReq == 1 ) ) )
            {
                index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_JOIN, MlmeConfirmQueueCnt );
                if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN ) )
                {// Procedure for the join request
                    MlmeConfirm.NbRetries = JoinRequestTrials;

                    if( MlmeConfirmQueue[index].Status == LORAMAC_EVENT_INFO_STATUS_OK )
                    {// Node joined successfully
                        UpLinkCounter = 0;
                        ChannelsNbRepCounter = 0;
                        LoRaMacState &= ~LORAMAC_TX_RUNNING;
                    }
                    else
                    {
                        if( JoinRequestTrials >= MaxJoinRequestTrials )
                        {
                            LoRaMacState &= ~LORAMAC_TX_RUNNING;
                        }
                        else
                        {
                            LoRaMacFlags.Bits.MacDone = 0;
                            // Sends the same frame again
                            OnTxDelayedTimerEvent( );
                        }
                    }
                }
                else
                {// Procedure for all other frames
                    if( ( ChannelsNbRepCounter >= LoRaMacParams.ChannelsNbRep ) || ( LoRaMacFlags.Bits.McpsInd == 1 ) )
                    {
                        if( LoRaMacFlags.Bits.McpsInd == 0 )
                        {   // Maximum repetitions without downlink. Reset MacCommandsBufferIndex. Increase ADR Ack counter.
                            // Only process the case when the MAC did not receive a downlink.
                            MacCommandsBufferIndex = 0;
                            AdrAckCounter++;
                        }

                        ChannelsNbRepCounter = 0;

                        if( IsUpLinkCounterFixed == false )
                        {
                            UpLinkCounter++;
                        }

                        LoRaMacState &= ~LORAMAC_TX_RUNNING;
                    }
                    else
                    {
                        LoRaMacFlags.Bits.MacDone = 0;
                        // Sends the same frame again
                        OnTxDelayedTimerEvent( );
                    }
                }
            }
        }

        if( LoRaMacFlags.Bits.McpsInd == 1 )
        {// Procedure if we received a frame
            if( ( McpsConfirm.AckReceived == true ) || ( AckTimeoutRetriesCounter > AckTimeoutRetries ) )
            {
                AckTimeoutRetry = false;
                NodeAckRequested = false;
                if( IsUpLinkCounterFixed == false )
                {
                    UpLinkCounter++;
                }
                McpsConfirm.NbRetries = AckTimeoutRetriesCounter;

                LoRaMacState &= ~LORAMAC_TX_RUNNING;
            }
        }

        if( ( AckTimeoutRetry == true ) && ( ( LoRaMacState & LORAMAC_TX_DELAYED ) == 0 ) )
        {// Retransmissions procedure for confirmed uplinks
            AckTimeoutRetry = false;
            if( ( AckTimeoutRetriesCounter < AckTimeoutRetries ) && ( AckTimeoutRetriesCounter <= MAX_ACK_RETRIES ) )
            {
                AckTimeoutRetriesCounter++;

                if( ( AckTimeoutRetriesCounter % 2 ) == 1 )
                {
                    getPhy.Attribute = PHY_NEXT_LOWER_TX_DR;
                    getPhy.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;
                    getPhy.Datarate = LoRaMacParams.ChannelsDatarate;
                    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
                    LoRaMacParams.ChannelsDatarate = phyParam.Value;
                }
                // Try to send the frame again
                if( ScheduleTx( ) == LORAMAC_STATUS_OK )
                {
                    LoRaMacFlags.Bits.MacDone = 0;
                }
                else
                {
                    // The DR is not applicable for the payload size
                    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR;

                    MacCommandsBufferIndex = 0;
                    LoRaMacState &= ~LORAMAC_TX_RUNNING;
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
                RegionInitDefaults( LoRaMacRegion, INIT_TYPE_RESTORE );

                LoRaMacState &= ~LORAMAC_TX_RUNNING;

                MacCommandsBufferIndex = 0;
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
    if( ( LoRaMacState & LORAMAC_RX ) == LORAMAC_RX )
    {
        LoRaMacState &= ~LORAMAC_RX;
    }
    if( LoRaMacState == LORAMAC_IDLE )
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

        // Procedure done. Reset variables.
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
        if( LoRaMacDeviceClass == CLASS_C )
        {// Activate RX2 window for Class C
            OnRxWindow2TimerEvent( );
        }
        if( LoRaMacFlags.Bits.McpsIndSkip == 0 )
        {
            LoRaMacPrimitives->MacMcpsIndication( &McpsIndication );
        }
        LoRaMacFlags.Bits.McpsIndSkip = 0;
        LoRaMacFlags.Bits.McpsInd = 0;
    }
}

static void OnTxDelayedTimerEvent( void )
{
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;
    AlternateDrParams_t altDr;

    TimerStop( &TxDelayedTimer );
    LoRaMacState &= ~LORAMAC_TX_DELAYED;

    index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_JOIN, MlmeConfirmQueueCnt );

    if( ( LoRaMacFlags.Bits.MlmeReq == 1 ) && ( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN ) )
    {
        ResetMacParameters( );

        altDr.NbTrials = JoinRequestTrials + 1;
        LoRaMacParams.ChannelsDatarate = RegionAlternateDr( LoRaMacRegion, &altDr );

        macHdr.Value = 0;
        macHdr.Bits.MType = FRAME_TYPE_JOIN_REQ;

        fCtrl.Value = 0;
        fCtrl.Bits.Adr = AdrCtrlOn;

        /* In case of join request retransmissions, the stack must prepare
         * the frame again, because the network server keeps track of the random
         * LoRaMacDevNonce values to prevent reply attacks. */
        PrepareFrame( &macHdr, &fCtrl, 0, NULL, 0 );
    }

    ScheduleTx( );
}

static void OnRxWindow1TimerEvent( void )
{
    TimerStop( &RxWindowTimer1 );
    RxSlot = 0;

    RxWindow1Config.Channel = Channel;
    RxWindow1Config.DrOffset = LoRaMacParams.Rx1DrOffset;
    RxWindow1Config.DownlinkDwellTime = LoRaMacParams.DownlinkDwellTime;
    RxWindow1Config.RepeaterSupport = RepeaterSupport;
    RxWindow1Config.RxContinuous = false;
    RxWindow1Config.Window = RxSlot;

    if( LoRaMacDeviceClass == CLASS_C )
    {
        Radio.Standby( );
    }

    RegionRxConfig( LoRaMacRegion, &RxWindow1Config, ( int8_t* )&McpsIndication.RxDatarate );
    RxWindowSetup( RxWindow1Config.RxContinuous, LoRaMacParams.MaxRxWindow );
}

static void OnRxWindow2TimerEvent( void )
{
    TimerStop( &RxWindowTimer2 );

    RxWindow2Config.Channel = Channel;
    RxWindow2Config.Frequency = LoRaMacParams.Rx2Channel.Frequency;
    RxWindow2Config.DownlinkDwellTime = LoRaMacParams.DownlinkDwellTime;
    RxWindow2Config.RepeaterSupport = RepeaterSupport;
    RxWindow2Config.Window = 1;

    if( LoRaMacDeviceClass != CLASS_C )
    {
        RxWindow2Config.RxContinuous = false;
    }
    else
    {
        RxWindow2Config.RxContinuous = true;
    }

    if( RegionRxConfig( LoRaMacRegion, &RxWindow2Config, ( int8_t* )&McpsIndication.RxDatarate ) == true )
    {
        RxWindowSetup( RxWindow2Config.RxContinuous, LoRaMacParams.MaxRxWindow );
        RxSlot = RxWindow2Config.Window;
    }
}

static void OnAckTimeoutTimerEvent( void )
{
    TimerStop( &AckTimeoutTimer );

    if( NodeAckRequested == true )
    {
        AckTimeoutRetry = true;
        LoRaMacState &= ~LORAMAC_ACK_REQ;
    }
    if( LoRaMacDeviceClass == CLASS_C )
    {
        LoRaMacFlags.Bits.MacDone = 1;
    }
}

static void RxWindowSetup( bool rxContinuous, uint32_t maxRxWindow )
{
    if( rxContinuous == false )
    {
        Radio.Rx( maxRxWindow );
    }
    else
    {
        Radio.Rx( 0 ); // Continuous mode
    }
}

static LoRaMacStatus_t SwitchClass( DeviceClass_t deviceClass )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_PARAMETER_INVALID;

    switch( LoRaMacDeviceClass )
    {
        case CLASS_A:
        {
            if( deviceClass == CLASS_B )
            {
                if( ( BeaconCtx.Ctrl.BeaconMode == 1 ) && ( PingSlotCtx.Ctrl.Assigned == 1 ) )
                {
                    LoRaMacDeviceClass = deviceClass;
                    status = LORAMAC_STATUS_OK;
                }
            }

            if( deviceClass == CLASS_C )
            {
                LoRaMacDeviceClass = deviceClass;

                // Set the NodeAckRequested indicator to default
                NodeAckRequested = false;
                OnRxWindow2TimerEvent( );

                status = LORAMAC_STATUS_OK;
            }
            break;
        }
        case CLASS_B:
        {
            if( deviceClass == CLASS_A )
            {
                BeaconState = BEACON_STATE_ACQUISITION;
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
                Radio.Sleep();
                BeaconState = BEACON_STATE_SWITCH_CLASS;
            }
            else
            {
                // Default symbol timeouts
                BeaconCtx.SymbolTimeout = BeaconCtx.Cfg.SymbolToDefault;
                PingSlotCtx.SymbolTimeout = BeaconCtx.Cfg.SymbolToDefault;

                if( BeaconCtx.Ctrl.BeaconDelaySet == 1 )
                {
                    if( BeaconCtx.BeaconTimingDelay > 0 )
                    {
                        if( BeaconCtx.NextBeaconRx > currentTime )
                        {
                            BeaconCtx.Ctrl.AcquisitionTimerSet = 1;
                            beaconEventTime = TimerTempCompensation( BeaconCtx.NextBeaconRx - currentTime, BeaconCtx.Temperature );
                        }
                        else
                        {
                            BeaconCtx.Ctrl.BeaconDelaySet = 0;
                            BeaconCtx.Ctrl.AcquisitionPending = 1;
                            BeaconCtx.Ctrl.AcquisitionTimerSet = 0;
                            beaconEventTime = BeaconCtx.Cfg.Interval;
                            RxBeaconSetup( BeaconCtx.SymbolTimeout, 0 );
                        }
                        BeaconCtx.NextBeaconRx = 0;
                        BeaconCtx.BeaconTimingDelay = 0;
                    }
                    else
                    {
                        BeaconCtx.Ctrl.BeaconDelaySet = 0;
                        BeaconCtx.Ctrl.AcquisitionPending = 0;
                        BeaconCtx.Ctrl.AcquisitionTimerSet = 1;
                        beaconEventTime = BeaconCtx.Cfg.DelayBeaconTimingAns;
                        RxBeaconSetup( BeaconCtx.SymbolTimeout, 0 );
                    }
                }
                else
                {
                    BeaconCtx.Ctrl.AcquisitionPending = 1;
                    beaconEventTime = BeaconCtx.Cfg.Interval;
                    if( BeaconCtx.Ctrl.AcquisitionTimerSet == 0 )
                    {
                        RxBeaconSetup( BeaconCtx.SymbolTimeout, 0 );
                    }
                    BeaconCtx.Ctrl.AcquisitionTimerSet = 0;
                }
            }
            break;
        }
        case BEACON_STATE_TIMEOUT:
        {
            // Store listen time
            BeaconCtx.ListenTime = currentTime - BeaconCtx.NextBeaconRx;
            // Setup next state
            BeaconState = BEACON_STATE_BEACON_MISSED;
            // no break here
        }
        case BEACON_STATE_BEACON_MISSED:
        {
            // We have to update the beacon time, since we missed a beacon
            BeaconCtx.BeaconTime += ( BeaconCtx.Cfg.Interval / 1000 );

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

                if( PingSlotCtx.Ctrl.Assigned == 1 )
                {
                    PingSlotState = PINGSLOT_STATE_CALC_PING_OFFSET;
                    TimerSetValue( &PingSlotTimer, 1 );
                    TimerStart( &PingSlotTimer );

                    MulticastSlotState = PINGSLOT_STATE_CALC_PING_OFFSET;
                    TimerSetValue( &MulticastSlotTimer, 1 );
                    TimerStart( &MulticastSlotTimer );
                }
            }
            BeaconCtx.Ctrl.BeaconAcquired = 0;

            if( BeaconCtx.Ctrl.ResumeBeaconing == 0 )
            {
                MlmeIndication.MlmeIndication = MLME_BEACON;
                MlmeIndication.Status = LORAMAC_EVENT_INFO_STATUS_BEACON_LOST;
                LoRaMacFlags.Bits.MlmeInd = 1;

                TimerSetValue( &MacStateCheckTimer, 1 );
                TimerStart( &MacStateCheckTimer );
                LoRaMacFlags.Bits.MacDone = 1;
            }
            BeaconCtx.Ctrl.ResumeBeaconing = 0;
            break;
        }
        case BEACON_STATE_LOCKED:
        {
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

            if( LoRaMacFlags.Bits.MlmeReq == 1 )
            {
                index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_BEACON_ACQUISITION, MlmeConfirmQueueCnt );
                if( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
                {
                    MlmeConfirmQueue[index].Status = LORAMAC_EVENT_INFO_STATUS_OK;
                    MlmeConfirm.TxTimeOnAir = 0;
                }
            }

            if( PingSlotCtx.Ctrl.Assigned == 1 )
            {
                PingSlotState = PINGSLOT_STATE_CALC_PING_OFFSET;
                TimerSetValue( &PingSlotTimer, 1 );
                TimerStart( &PingSlotTimer );

                MulticastSlotState = PINGSLOT_STATE_CALC_PING_OFFSET;
                TimerSetValue( &MulticastSlotTimer, 1 );
                TimerStart( &MulticastSlotTimer );
            }
            BeaconCtx.Ctrl.AcquisitionPending = 0;

            if( BeaconCtx.Ctrl.ResumeBeaconing == 0 )
            {
                MlmeIndication.MlmeIndication = MLME_BEACON;
                MlmeIndication.Status = LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED;
                LoRaMacFlags.Bits.MlmeInd = 1;

                TimerSetValue( &MacStateCheckTimer, 1 );
                TimerStart( &MacStateCheckTimer );
                LoRaMacFlags.Bits.MacDone = 1;
            }
            BeaconCtx.Ctrl.ResumeBeaconing = 0;
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
            currentTime = TimerGetCurrentTime( );

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
            if( LoRaMacFlags.Bits.MlmeReq == 1 )
            {
                index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_BEACON_ACQUISITION, MlmeConfirmQueueCnt );
                if( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
                {
                    MlmeConfirmQueue[index].Status = LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND;
                }
            }
            else
            {
                MlmeIndication.MlmeIndication = MLME_SWITCH_CLASS;
                MlmeIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                PingSlotCtx.Ctrl.Assigned = 0;
                LoRaMacFlags.Bits.MlmeInd = 1;
            }
            BeaconState = BEACON_STATE_ACQUISITION;

            BeaconCtx.Ctrl.BeaconMode = 0;
            BeaconCtx.Ctrl.AcquisitionPending = 0;
            BeaconCtx.Ctrl.AcquisitionTimerSet = 0;
            LoRaMacFlags.Bits.MacDone = 1;

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
                if( BeaconCtx.Ctrl.BeaconAcquired == 1 )
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

            if( BeaconCtx.Ctrl.BeaconAcquired == 1 )
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
                       1, 0, 10, timeout, true, BEACON_SIZE, false, 0, 0, false, rxContinuous );

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

    // BeaconState = BEACON_STATE_REACQUISITION;
    if( ( BeaconState == BEACON_STATE_RX ) || ( BeaconCtx.Ctrl.AcquisitionPending == 1 ) )
    {
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
                MlmeIndication.BeaconInfo.Time = BeaconCtx.BeaconTime;
                beaconReceived = true;
            }

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
                BeaconCtx.Ctrl.BeaconAcquired = 1;
                BeaconCtx.Ctrl.BeaconMode = 1;
                BeaconCtx.SymbolTimeout = BeaconCtx.Cfg.SymbolToDefault;
                BeaconState = BEACON_STATE_LOCKED;

                OnBeaconTimerEvent( );
            }
        }

        if( BeaconState == BEACON_STATE_RX )
        {
            BeaconState = BEACON_STATE_TIMEOUT;
            OnBeaconTimerEvent( );
        }
        // Return always true, when we expect a beacon.
        beaconReceived = true;
    }

    return beaconReceived;
}

static uint16_t BeaconCrc( uint8_t *buffer, uint16_t length )
{
    // The CRC calculation follows CCITT
    const uint16_t polynom = 0x1021;
    // CRC initial value
    uint16_t crc = 0x0000;

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
    if( ( BeaconCtx.Ctrl.AcquisitionPending == 1 ) ||
        ( BeaconCtx.Ctrl.AcquisitionTimerSet == 1 ) ||
        ( BeaconState == BEACON_STATE_RX ) )
    {
        return true;
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
        if( ( BeaconState == BEACON_STATE_TIMEOUT ) ||
            ( BeaconState == BEACON_STATE_SWITCH_CLASS ) )
        {
            // Update the state machine before halt
            OnBeaconTimerEvent( );
        }

        // Halt beacon state machine
        BeaconState = BEACON_STATE_HALT;

        // Halt ping slot state machine
        TimerStop( &BeaconTimer );

        // Halt ping slot state machine
        TimerStop( &PingSlotTimer );

        // Halt multicast ping slot state machine
        TimerStop( &MulticastSlotTimer );
    }
}

static void ResumeBeaconing( void )
{
    if( LoRaMacDeviceClass == CLASS_B )
    {
        if( BeaconState == BEACON_STATE_HALT )
        {
            BeaconCtx.Ctrl.ResumeBeaconing = 1;

            // Set default state
            BeaconState = BEACON_STATE_LOCKED;

            if( BeaconCtx.Ctrl.BeaconAcquired == 0 )
            {
                // Set the default state for beacon less operation
                BeaconState = BEACON_STATE_REACQUISITION;
            }
            TimerSetValue( &BeaconTimer, 1 );
            TimerStart( &BeaconTimer );
        }
    }
}

static bool ValidatePayloadLength( uint8_t lenN, int8_t datarate, uint8_t fOptsLen )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    uint16_t maxN = 0;
    uint16_t payloadSize = 0;

    // Setup PHY request
    getPhy.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;
    getPhy.Datarate = datarate;
    getPhy.Attribute = PHY_MAX_PAYLOAD;

    // Get the maximum payload length
    if( RepeaterSupport == true )
    {
        getPhy.Attribute = PHY_MAX_PAYLOAD_REPEATER;
    }
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    maxN = phyParam.Value;

    // Calculate the resulting payload size
    payloadSize = ( lenN + fOptsLen );

    // Validation of the application payload size
    if( ( payloadSize <= maxN ) && ( payloadSize <= LORAMAC_PHY_MAXPAYLOAD ) )
    {
        return true;
    }
    return false;
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
        case MOTE_MAC_TX_PARAM_SETUP_ANS:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DL_CHANNEL_ANS:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Uplink frequency exists, Channel frequency OK
                MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
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
            // STICKY
            case MOTE_MAC_DL_CHANNEL_ANS:
            case MOTE_MAC_RX_PARAM_SETUP_ANS:
            { // 1 byte payload
                cmdBufOut[cmdCount++] = cmdBufIn[i++];
                cmdBufOut[cmdCount++] = cmdBufIn[i];
                break;
            }
            case MOTE_MAC_RX_TIMING_SETUP_ANS:
            { // 0 byte payload
                cmdBufOut[cmdCount++] = cmdBufIn[i];
                break;
            }
            // NON-STICKY
            case MOTE_MAC_DEV_STATUS_ANS:
            { // 2 bytes payload
                i += 2;
                break;
            }
            case MOTE_MAC_LINK_ADR_ANS:
            case MOTE_MAC_NEW_CHANNEL_ANS:
            { // 1 byte payload
                i++;
                break;
            }
            case MOTE_MAC_TX_PARAM_SETUP_ANS:
            case MOTE_MAC_DUTY_CYCLE_ANS:
            case MOTE_MAC_LINK_CHECK_REQ:
            { // 0 byte payload
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
    uint8_t status = 0;

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
                    LinkAdrReqParams_t linkAdrReq;
                    int8_t linkAdrDatarate = DR_0;
                    int8_t linkAdrTxPower = TX_POWER_0;
                    uint8_t linkAdrNbRep = 0;
                    uint8_t linkAdrNbBytesParsed = 0;

                    // Fill parameter structure
                    linkAdrReq.Payload = &payload[macIndex - 1];
                    linkAdrReq.PayloadSize = commandsSize - ( macIndex - 1 );
                    linkAdrReq.AdrEnabled = AdrCtrlOn;
                    linkAdrReq.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;
                    linkAdrReq.CurrentDatarate = LoRaMacParams.ChannelsDatarate;
                    linkAdrReq.CurrentTxPower = LoRaMacParams.ChannelsTxPower;
                    linkAdrReq.CurrentNbRep = LoRaMacParams.ChannelsNbRep;

                    // Process the ADR requests
                    status = RegionLinkAdrReq( LoRaMacRegion, &linkAdrReq, &linkAdrDatarate,
                                               &linkAdrTxPower, &linkAdrNbRep, &linkAdrNbBytesParsed );

                    if( ( status & 0x07 ) == 0x07 )
                    {
                        LoRaMacParams.ChannelsDatarate = linkAdrDatarate;
                        LoRaMacParams.ChannelsTxPower = linkAdrTxPower;
                        LoRaMacParams.ChannelsNbRep = linkAdrNbRep;
                    }

                    // Add the answers to the buffer
                    for( uint8_t i = 0; i < ( linkAdrNbBytesParsed / 5 ); i++ )
                    {
                        AddMacCommand( MOTE_MAC_LINK_ADR_ANS, status, 0 );
                    }
                    // Update MAC index
                    macIndex += linkAdrNbBytesParsed - 1;
                }
                break;
            case SRV_MAC_DUTY_CYCLE_REQ:
                MaxDCycle = payload[macIndex++];
                AggregatedDCycle = 1 << MaxDCycle;
                AddMacCommand( MOTE_MAC_DUTY_CYCLE_ANS, 0, 0 );
                break;
            case SRV_MAC_RX_PARAM_SETUP_REQ:
                {
                    RxParamSetupReqParams_t rxParamSetupReq;
                    status = 0x07;

                    rxParamSetupReq.DrOffset = ( payload[macIndex] >> 4 ) & 0x07;
                    rxParamSetupReq.Datarate = payload[macIndex] & 0x0F;
                    macIndex++;

                    rxParamSetupReq.Frequency =  ( uint32_t )payload[macIndex++];
                    rxParamSetupReq.Frequency |= ( uint32_t )payload[macIndex++] << 8;
                    rxParamSetupReq.Frequency |= ( uint32_t )payload[macIndex++] << 16;
                    rxParamSetupReq.Frequency *= 100;

                    // Perform request on region
                    status = RegionRxParamSetupReq( LoRaMacRegion, &rxParamSetupReq );

                    if( ( status & 0x07 ) == 0x07 )
                    {
                        LoRaMacParams.Rx2Channel.Datarate = rxParamSetupReq.Datarate;
                        LoRaMacParams.Rx2Channel.Frequency = rxParamSetupReq.Frequency;
                        LoRaMacParams.Rx1DrOffset = rxParamSetupReq.DrOffset;
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
                    NewChannelReqParams_t newChannelReq;
                    ChannelParams_t chParam;
                    status = 0x03;

                    newChannelReq.ChannelId = payload[macIndex++];
                    newChannelReq.NewChannel = &chParam;

                    chParam.Frequency = ( uint32_t )payload[macIndex++];
                    chParam.Frequency |= ( uint32_t )payload[macIndex++] << 8;
                    chParam.Frequency |= ( uint32_t )payload[macIndex++] << 16;
                    chParam.Frequency *= 100;
                    chParam.Rx1Frequency = 0;
                    chParam.DrRange.Value = payload[macIndex++];

                    status = RegionNewChannelReq( LoRaMacRegion, &newChannelReq );

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
                    LoRaMacParams.ReceiveDelay1 = delay * 1000;
                    LoRaMacParams.ReceiveDelay2 = LoRaMacParams.ReceiveDelay1 + 1000;
                    AddMacCommand( MOTE_MAC_RX_TIMING_SETUP_ANS, 0, 0 );
                }
                break;
            case SRV_MAC_TX_PARAM_SETUP_REQ:
                {
                    TxParamSetupReqParams_t txParamSetupReq;
                    uint8_t eirpDwellTime = payload[macIndex++];

                    txParamSetupReq.UplinkDwellTime = 0;
                    txParamSetupReq.DownlinkDwellTime = 0;

                    if( ( eirpDwellTime & 0x20 ) == 0x20 )
                    {
                        txParamSetupReq.DownlinkDwellTime = 1;
                    }
                    if( ( eirpDwellTime & 0x10 ) == 0x10 )
                    {
                        txParamSetupReq.UplinkDwellTime = 1;
                    }
                    txParamSetupReq.MaxEirp = eirpDwellTime & 0x0F;

                    // Check the status for correctness
                    if( RegionTxParamSetupReq( LoRaMacRegion, &txParamSetupReq ) != -1 )
                    {
                        // Accept command
                        LoRaMacParams.UplinkDwellTime = txParamSetupReq.UplinkDwellTime;
                        LoRaMacParams.DownlinkDwellTime = txParamSetupReq.DownlinkDwellTime;
                        LoRaMacParams.MaxEirp = LoRaMacMaxEirpTable[txParamSetupReq.MaxEirp];
                        // Add command response
                        AddMacCommand( MOTE_MAC_TX_PARAM_SETUP_ANS, 0, 0 );
                    }
                }
                break;
            case SRV_MAC_DL_CHANNEL_REQ:
                {
                    DlChannelReqParams_t dlChannelReq;
                    status = 0x03;

                    dlChannelReq.ChannelId = payload[macIndex++];
                    dlChannelReq.Rx1Frequency = ( uint32_t )payload[macIndex++];
                    dlChannelReq.Rx1Frequency |= ( uint32_t )payload[macIndex++] << 8;
                    dlChannelReq.Rx1Frequency |= ( uint32_t )payload[macIndex++] << 16;
                    dlChannelReq.Rx1Frequency *= 100;

                    status = RegionDlChannelReq( LoRaMacRegion, &dlChannelReq );

                    AddMacCommand( MOTE_MAC_DL_CHANNEL_ANS, status, 0 );
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

static LoRaMacStatus_t ScheduleTx( void )
{
    TimerTime_t dutyCycleTimeOff = 0;
    NextChanParams_t nextChan;

    // Check if the device is off
    if( MaxDCycle == 255 )
    {
        return LORAMAC_STATUS_DEVICE_OFF;
    }
    if( MaxDCycle == 0 )
    {
        AggregatedTimeOff = 0;
    }

    // Update Backoff
    CalculateBackOff( LastTxChannel );

    nextChan.AggrTimeOff = AggregatedTimeOff;
    nextChan.Datarate = LoRaMacParams.ChannelsDatarate;
    nextChan.DutyCycleEnabled = DutyCycleOn;
    nextChan.Joined = IsLoRaMacNetworkJoined;
    nextChan.LastAggrTx = AggregatedLastTxDoneTime;

    // Select channel
    while( RegionNextChannel( LoRaMacRegion, &nextChan, &Channel, &dutyCycleTimeOff, &AggregatedTimeOff ) == false )
    {
        // Set the default datarate
        LoRaMacParams.ChannelsDatarate = LoRaMacParamsDefaults.ChannelsDatarate;
        // Update datarate in the function parameters
        nextChan.Datarate = LoRaMacParams.ChannelsDatarate;
    }

    // Compute Rx1 windows parameters
    RegionComputeRxWindowParameters( LoRaMacRegion,
                                     RegionApplyDrOffset( LoRaMacRegion, LoRaMacParams.DownlinkDwellTime, LoRaMacParams.ChannelsDatarate, LoRaMacParams.Rx1DrOffset ),
                                     LoRaMacParams.MinRxSymbols,
                                     LoRaMacParams.SystemMaxRxError,
                                     &RxWindow1Config );
    // Compute Rx2 windows parameters
    RegionComputeRxWindowParameters( LoRaMacRegion,
                                     LoRaMacParams.Rx2Channel.Datarate,
                                     LoRaMacParams.MinRxSymbols,
                                     LoRaMacParams.SystemMaxRxError,
                                     &RxWindow2Config );

    if( IsLoRaMacNetworkJoined == false )
    {
        RxWindow1Delay = LoRaMacParams.JoinAcceptDelay1 + RxWindow1Config.WindowOffset;
        RxWindow2Delay = LoRaMacParams.JoinAcceptDelay2 + RxWindow2Config.WindowOffset;
    }
    else
    {
        if( ValidatePayloadLength( LoRaMacTxPayloadLen, LoRaMacParams.ChannelsDatarate, MacCommandsBufferIndex ) == false )
        {
            return LORAMAC_STATUS_LENGTH_ERROR;
        }
        RxWindow1Delay = LoRaMacParams.ReceiveDelay1 + RxWindow1Config.WindowOffset;
        RxWindow2Delay = LoRaMacParams.ReceiveDelay2 + RxWindow2Config.WindowOffset;
    }

    // Schedule transmission of frame
    if( dutyCycleTimeOff == 0 )
    {
        // Try to send now
        return SendFrameOnChannel( Channel );
    }

    timeOff = MAX( dutyCycleTimeOff, mutexTimeLock );

    if( timeOff > 0 )
    {
        // Send later - prepare timer
        LoRaMacState |= LORAMAC_TX_DELAYED;
        TimerSetValue( &TxDelayedTimer, timeOff );
        TimerStart( &TxDelayedTimer );
    }
    return LORAMAC_STATUS_OK;
}

static void CalculateBackOff( uint8_t channel )
{
    CalcBackOffParams_t calcBackOff;

    calcBackOff.Joined = IsLoRaMacNetworkJoined;
    calcBackOff.DutyCycleEnabled = DutyCycleOn;
    calcBackOff.Channel = channel;
    calcBackOff.ElapsedTime = TimerGetElapsedTime( LoRaMacInitializationTime );
    calcBackOff.TxTimeOnAir = TxTimeOnAir;
    calcBackOff.LastTxIsJoinRequest = LastTxIsJoinRequest;

    // Update regional back-off
    RegionCalcBackOff( LoRaMacRegion, &calcBackOff );

    // Update aggregated time-off
    AggregatedTimeOff = AggregatedTimeOff + ( TxTimeOnAir * AggregatedDCycle - TxTimeOnAir );
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
    UpLinkCounter = 0;
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
    LoRaMacParams.Rx1DrOffset = LoRaMacParamsDefaults.Rx1DrOffset;
    LoRaMacParams.Rx2Channel = LoRaMacParamsDefaults.Rx2Channel;
    LoRaMacParams.UplinkDwellTime = LoRaMacParamsDefaults.UplinkDwellTime;
    LoRaMacParams.DownlinkDwellTime = LoRaMacParamsDefaults.DownlinkDwellTime;
    LoRaMacParams.MaxEirp = LoRaMacParamsDefaults.MaxEirp;
    LoRaMacParams.AntennaGain = LoRaMacParamsDefaults.AntennaGain;

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
    Channel = 0;
    LastTxChannel = Channel;
}

LoRaMacStatus_t PrepareFrame( LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    AdrNextParams_t adrNext;
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
            //Intentional fallthrough
        case FRAME_TYPE_DATA_UNCONFIRMED_UP:
            if( IsLoRaMacNetworkJoined == false )
            {
                return LORAMAC_STATUS_NO_NETWORK_JOINED; // No network has been joined yet
            }

            // Adr next request
            adrNext.UpdateChanMask = true;
            adrNext.AdrEnabled = fCtrl->Bits.Adr;
            adrNext.AdrAckCounter = AdrAckCounter;
            adrNext.Datarate = LoRaMacParams.ChannelsDatarate;
            adrNext.TxPower = LoRaMacParams.ChannelsTxPower;
            adrNext.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;

            fCtrl->Bits.AdrAckReq = RegionAdrNext( LoRaMacRegion, &adrNext,
                                                   &LoRaMacParams.ChannelsDatarate, &LoRaMacParams.ChannelsTxPower, &AdrAckCounter );

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
                if( MacCommandsInNextTx == true )
                {
                    if( MacCommandsBufferIndex <= LORA_MAC_COMMAND_MAX_FOPTS_LENGTH )
                    {
                        fCtrl->Bits.FOptsLen += MacCommandsBufferIndex;

                        // Update FCtrl field with new value of OptionsLength
                        LoRaMacBuffer[0x05] = fCtrl->Value;
                        for( i = 0; i < MacCommandsBufferIndex; i++ )
                        {
                            LoRaMacBuffer[pktHeaderLen++] = MacCommandsBuffer[i];
                        }
                    }
                    else
                    {
                        LoRaMacTxPayloadLen = MacCommandsBufferIndex;
                        payload = MacCommandsBuffer;
                        framePort = 0;
                    }
                }
            }
            else
            {
                if( ( MacCommandsBufferIndex > 0 ) && ( MacCommandsInNextTx == true ) )
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

            if( ( payload != NULL ) && ( LoRaMacTxPayloadLen > 0 ) )
            {
                LoRaMacBuffer[pktHeaderLen++] = framePort;

                if( framePort == 0 )
                {
                    // Reset buffer index as the mac commands are being sent on port 0
                    MacCommandsBufferIndex = 0;
                    LoRaMacPayloadEncrypt( (uint8_t* ) payload, LoRaMacTxPayloadLen, LoRaMacNwkSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &LoRaMacBuffer[pktHeaderLen] );
                }
                else
                {
                    LoRaMacPayloadEncrypt( (uint8_t* ) payload, LoRaMacTxPayloadLen, LoRaMacAppSKey, LoRaMacDevAddr, UP_LINK, UpLinkCounter, &LoRaMacBuffer[pktHeaderLen] );
                }
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

LoRaMacStatus_t SendFrameOnChannel( uint8_t channel )
{
    TxConfigParams_t txConfig;
    int8_t txPower = 0;

    txConfig.Channel = channel;
    txConfig.Datarate = LoRaMacParams.ChannelsDatarate;
    txConfig.TxPower = LoRaMacParams.ChannelsTxPower;
    txConfig.MaxEirp = LoRaMacParams.MaxEirp;
    txConfig.AntennaGain = LoRaMacParams.AntennaGain;
    txConfig.PktLen = LoRaMacBufferPktLen;

    RegionTxConfig( LoRaMacRegion, &txConfig, &txPower, &TxTimeOnAir );

    MlmeConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    McpsConfirm.Datarate = LoRaMacParams.ChannelsDatarate;
    McpsConfirm.TxPower = txPower;

    // Store the time on air
    McpsConfirm.TxTimeOnAir = TxTimeOnAir;
    MlmeConfirm.TxTimeOnAir = TxTimeOnAir;

    if( ( LoRaMacDeviceClass == CLASS_B ) || ( BeaconCtx.Ctrl.BeaconMode == 1 ) )
    {
        TimerTime_t currentTime = TimerGetCurrentTime( );

        if( ( currentTime >= ( BeaconCtx.NextBeaconRx - BeaconCtx.Cfg.Guard - LoRaMacParams.ReceiveDelay1 - LoRaMacParams.ReceiveDelay2 - TxTimeOnAir ) ) &&
            ( currentTime < ( BeaconCtx.NextBeaconRx + BeaconCtx.Cfg.Reserved ) ) )
        {
            return BeaconCtx.Cfg.Reserved;
        }
    }
    HaltBeaconing( );

    // Starts the MAC layer status check timer
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
    TimerStart( &MacStateCheckTimer );

    if( IsLoRaMacNetworkJoined == false )
    {
        JoinRequestTrials++;
    }

    LoRaMacState |= LORAMAC_TX_RUNNING;

    // Send now
    Radio.Send( LoRaMacBuffer, LoRaMacBufferPktLen );

    LoRaMacState |= LORAMAC_TX_RUNNING;

    return 0;
}

LoRaMacStatus_t SetTxContinuousWave( uint16_t timeout )
{
    ContinuousWaveParams_t continuousWave;

    continuousWave.Channel = Channel;
    continuousWave.Datarate = LoRaMacParams.ChannelsDatarate;
    continuousWave.TxPower = LoRaMacParams.ChannelsTxPower;
    continuousWave.MaxEirp = LoRaMacParams.MaxEirp;
    continuousWave.AntennaGain = LoRaMacParams.AntennaGain;
    continuousWave.Timeout = timeout;

    RegionSetContinuousWave( LoRaMacRegion, &continuousWave );

    // Starts the MAC layer status check timer
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
    TimerStart( &MacStateCheckTimer );

    LoRaMacState |= LORAMAC_TX_RUNNING;

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t SetTxContinuousWave1( uint16_t timeout, uint32_t frequency, uint8_t power )
{
    Radio.SetTxContinuousWave( frequency, power, timeout );

    // Starts the MAC layer status check timer
    TimerSetValue( &MacStateCheckTimer, MAC_STATE_CHECK_TIMEOUT );
    TimerStart( &MacStateCheckTimer );

    LoRaMacState |= LORAMAC_TX_RUNNING;

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacInitialization( LoRaMacPrimitives_t *primitives, LoRaMacCallback_t *callbacks, LoRaMacRegion_t region )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;

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
    // Verify if the region is supported
    if( RegionIsActive( region ) == false )
    {
        return LORAMAC_STATUS_REGION_NOT_SUPPORTED;
    }

    LoRaMacPrimitives = primitives;
    LoRaMacCallbacks = callbacks;
    LoRaMacRegion = region;

    LoRaMacFlags.Value = 0;

    LoRaMacDeviceClass = CLASS_A;
    LoRaMacState = LORAMAC_IDLE;

    JoinRequestTrials = 0;
    MaxJoinRequestTrials = 1;
    RepeaterSupport = false;

    // Reset duty cycle times
    AggregatedLastTxDoneTime = 0;
    AggregatedTimeOff = 0;

    // Reset to defaults
    getPhy.Attribute = PHY_DUTY_CYCLE;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    DutyCycleOn = ( bool ) phyParam.Value;

    getPhy.Attribute = PHY_DEF_TX_POWER;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.ChannelsTxPower = phyParam.Value;

    getPhy.Attribute = PHY_DEF_TX_DR;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.ChannelsDatarate = phyParam.Value;

    getPhy.Attribute = PHY_MAX_RX_WINDOW;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.MaxRxWindow = phyParam.Value;

    getPhy.Attribute = PHY_RECEIVE_DELAY1;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.ReceiveDelay1 = phyParam.Value;

    getPhy.Attribute = PHY_RECEIVE_DELAY2;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.ReceiveDelay2 = phyParam.Value;

    getPhy.Attribute = PHY_JOIN_ACCEPT_DELAY1;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.JoinAcceptDelay1 = phyParam.Value;

    getPhy.Attribute = PHY_JOIN_ACCEPT_DELAY2;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.JoinAcceptDelay2 = phyParam.Value;

    getPhy.Attribute = PHY_DEF_DR1_OFFSET;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.Rx1DrOffset = phyParam.Value;

    getPhy.Attribute = PHY_DEF_RX2_FREQUENCY;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.Rx2Channel.Frequency = phyParam.Value;

    getPhy.Attribute = PHY_DEF_RX2_DR;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.Rx2Channel.Datarate = phyParam.Value;

    getPhy.Attribute = PHY_DEF_UPLINK_DWELL_TIME;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.UplinkDwellTime = phyParam.Value;

    getPhy.Attribute = PHY_DEF_DOWNLINK_DWELL_TIME;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.DownlinkDwellTime = phyParam.Value;

    getPhy.Attribute = PHY_DEF_MAX_EIRP;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.MaxEirp = phyParam.fValue;

    getPhy.Attribute = PHY_DEF_ANTENNA_GAIN;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    LoRaMacParamsDefaults.AntennaGain = phyParam.fValue;

    RegionInitDefaults( LoRaMacRegion, INIT_TYPE_INIT );

    // Init parameters which are not set in function ResetMacParameters
    LoRaMacParamsDefaults.ChannelsNbRep = 1;
    LoRaMacParamsDefaults.SystemMaxRxError = 10;
    LoRaMacParamsDefaults.MinRxSymbols = 6;

    LoRaMacParams.SystemMaxRxError = LoRaMacParamsDefaults.SystemMaxRxError;
    LoRaMacParams.MinRxSymbols = LoRaMacParamsDefaults.MinRxSymbols;
    LoRaMacParams.MaxRxWindow = LoRaMacParamsDefaults.MaxRxWindow;
    LoRaMacParams.ReceiveDelay1 = LoRaMacParamsDefaults.ReceiveDelay1;
    LoRaMacParams.ReceiveDelay2 = LoRaMacParamsDefaults.ReceiveDelay2;
    LoRaMacParams.JoinAcceptDelay1 = LoRaMacParamsDefaults.JoinAcceptDelay1;
    LoRaMacParams.JoinAcceptDelay2 = LoRaMacParamsDefaults.JoinAcceptDelay2;
    LoRaMacParams.ChannelsNbRep = LoRaMacParamsDefaults.ChannelsNbRep;

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

    // Default temperature
    BeaconCtx.Temperature = 25.0;
    // Measure temperature
    if( ( LoRaMacCallbacks != NULL ) && ( LoRaMacCallbacks->GetTemperatureLevel != NULL ) )
    {
        BeaconCtx.Temperature = LoRaMacCallbacks->GetTemperatureLevel( );
    }

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

    // Store the current initialization time
    LoRaMacInitializationTime = TimerGetCurrentTime( );

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
    Radio.SetPublicNetwork( PublicNetwork );
    Radio.Sleep( );

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacQueryTxPossible( uint8_t size, LoRaMacTxInfo_t* txInfo )
{
    AdrNextParams_t adrNext;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    int8_t datarate = LoRaMacParamsDefaults.ChannelsDatarate;
    int8_t txPower = LoRaMacParamsDefaults.ChannelsTxPower;
    uint8_t fOptLen = MacCommandsBufferIndex + MacCommandsBufferToRepeatIndex;

    if( txInfo == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    // Setup ADR request
    adrNext.UpdateChanMask = false;
    adrNext.AdrEnabled = AdrCtrlOn;
    adrNext.AdrAckCounter = AdrAckCounter;
    adrNext.Datarate = LoRaMacParams.ChannelsDatarate;
    adrNext.TxPower = LoRaMacParams.ChannelsTxPower;
    adrNext.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;

    // We call the function for information purposes only. We don't want to
    // apply the datarate, the tx power and the ADR ack counter.
    RegionAdrNext( LoRaMacRegion, &adrNext, &datarate, &txPower, &AdrAckCounter );

    // Setup PHY request
    getPhy.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;
    getPhy.Datarate = datarate;
    getPhy.Attribute = PHY_MAX_PAYLOAD;

    // Change request in case repeater is supported
    if( RepeaterSupport == true )
    {
        getPhy.Attribute = PHY_MAX_PAYLOAD_REPEATER;
    }
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    txInfo->CurrentPayloadSize = phyParam.Value;

    // Verify if the fOpts fit into the maximum payload
    if( txInfo->CurrentPayloadSize >= fOptLen )
    {
        txInfo->MaxPossiblePayload = txInfo->CurrentPayloadSize - fOptLen;
    }
    else
    {
        txInfo->MaxPossiblePayload = txInfo->CurrentPayloadSize;
        // The fOpts don't fit into the maximum payload. Omit the MAC commands to
        // ensure that another uplink is possible.
        fOptLen = 0;
        MacCommandsBufferIndex = 0;
        MacCommandsBufferToRepeatIndex = 0;
    }

    // Verify if the fOpts and the payload fit into the maximum payload
    if( ValidatePayloadLength( size, datarate, fOptLen ) == false )
    {
        return LORAMAC_STATUS_LENGTH_ERROR;
    }
    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMibGetRequestConfirm( MibRequestConfirm_t *mibGet )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_OK;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;

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
            getPhy.Attribute = PHY_CHANNELS;
            phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );

            mibGet->Param.ChannelList = phyParam.Channels;
            break;
        }
        case MIB_RX2_CHANNEL:
        {
            mibGet->Param.Rx2Channel = LoRaMacParams.Rx2Channel;
            break;
        }
        case MIB_RX2_DEFAULT_CHANNEL:
        {
            mibGet->Param.Rx2Channel = LoRaMacParamsDefaults.Rx2Channel;
            break;
        }
        case MIB_CHANNELS_DEFAULT_MASK:
        {
            getPhy.Attribute = PHY_CHANNELS_DEFAULT_MASK;
            phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );

            mibGet->Param.ChannelsDefaultMask = phyParam.ChannelsMask;
            break;
        }
        case MIB_CHANNELS_MASK:
        {
            getPhy.Attribute = PHY_CHANNELS_MASK;
            phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );

            mibGet->Param.ChannelsMask = phyParam.ChannelsMask;
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
        case MIB_SYSTEM_MAX_RX_ERROR:
        {
            mibGet->Param.SystemMaxRxError = LoRaMacParams.SystemMaxRxError;
            break;
        }
        case MIB_MIN_RX_SYMBOLS:
        {
            mibGet->Param.MinRxSymbols = LoRaMacParams.MinRxSymbols;
            break;
        }
        case MIB_ANTENNA_GAIN:
        {
            mibGet->Param.AntennaGain = LoRaMacParams.AntennaGain;
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
    ChanMaskSetParams_t chanMaskSet;
    VerifyParams_t verify;

    if( mibSet == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    switch( mibSet->Type )
    {
        case MIB_DEVICE_CLASS:
        {
            status = SwitchClass( mibSet->Param.Class );
            break;
        }
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
            PublicNetwork = mibSet->Param.EnablePublicNetwork;
            Radio.SetPublicNetwork( PublicNetwork );
            break;
        }
        case MIB_REPEATER_SUPPORT:
        {
             RepeaterSupport = mibSet->Param.EnableRepeaterSupport;
            break;
        }
        case MIB_RX2_CHANNEL:
        {
            verify.DatarateParams.Datarate = mibSet->Param.Rx2Channel.Datarate;
            verify.DatarateParams.DownlinkDwellTime = LoRaMacParams.DownlinkDwellTime;

            if( RegionVerify( LoRaMacRegion, &verify, PHY_RX_DR ) == true )
            {
                LoRaMacParams.Rx2Channel = mibSet->Param.Rx2Channel;

                if( ( LoRaMacDeviceClass == CLASS_C ) && ( IsLoRaMacNetworkJoined == true ) )
                {
                    // Compute Rx2 windows parameters
                    RegionComputeRxWindowParameters( LoRaMacRegion,
                                                     LoRaMacParams.Rx2Channel.Datarate,
                                                     LoRaMacParams.MinRxSymbols,
                                                     LoRaMacParams.SystemMaxRxError,
                                                     &RxWindow2Config );

                    RxWindow2Config.Channel = Channel;
                    RxWindow2Config.Frequency = LoRaMacParams.Rx2Channel.Frequency;
                    RxWindow2Config.DownlinkDwellTime = LoRaMacParams.DownlinkDwellTime;
                    RxWindow2Config.RepeaterSupport = RepeaterSupport;
                    RxWindow2Config.Window = 1;
                    RxWindow2Config.RxContinuous = true;

                    if( RegionRxConfig( LoRaMacRegion, &RxWindow2Config, ( int8_t* )&McpsIndication.RxDatarate ) == true )
                    {
                        RxWindowSetup( RxWindow2Config.RxContinuous, LoRaMacParams.MaxRxWindow );
                        RxSlot = RxWindow2Config.Window;
                    }
                    else
                    {
                        status = LORAMAC_STATUS_PARAMETER_INVALID;
                    }
                }
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_RX2_DEFAULT_CHANNEL:
        {
            verify.DatarateParams.Datarate = mibSet->Param.Rx2Channel.Datarate;
            verify.DatarateParams.DownlinkDwellTime = LoRaMacParams.DownlinkDwellTime;

            if( RegionVerify( LoRaMacRegion, &verify, PHY_RX_DR ) == true )
            {
                LoRaMacParamsDefaults.Rx2Channel = mibSet->Param.Rx2DefaultChannel;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_DEFAULT_MASK:
        {
            chanMaskSet.ChannelsMaskIn = mibSet->Param.ChannelsMask;
            chanMaskSet.ChannelsMaskType = CHANNELS_DEFAULT_MASK;

            if( RegionChanMaskSet( LoRaMacRegion, &chanMaskSet ) == false )
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_MASK:
        {
            chanMaskSet.ChannelsMaskIn = mibSet->Param.ChannelsMask;
            chanMaskSet.ChannelsMaskType = CHANNELS_MASK;

            if( RegionChanMaskSet( LoRaMacRegion, &chanMaskSet ) == false )
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
            verify.DatarateParams.Datarate = mibSet->Param.ChannelsDefaultDatarate;

            if( RegionVerify( LoRaMacRegion, &verify, PHY_DEF_TX_DR ) == true )
            {
                LoRaMacParamsDefaults.ChannelsDatarate = verify.DatarateParams.Datarate;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_DATARATE:
        {
            verify.DatarateParams.Datarate = mibSet->Param.ChannelsDatarate;

            if( RegionVerify( LoRaMacRegion, &verify, PHY_TX_DR ) == true )
            {
                LoRaMacParams.ChannelsDatarate = verify.DatarateParams.Datarate;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_DEFAULT_TX_POWER:
        {
            verify.TxPower = mibSet->Param.ChannelsDefaultTxPower;

            if( RegionVerify( LoRaMacRegion, &verify, PHY_DEF_TX_POWER ) == true )
            {
                LoRaMacParamsDefaults.ChannelsTxPower = verify.TxPower;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_TX_POWER:
        {
            verify.TxPower = mibSet->Param.ChannelsTxPower;

            if( RegionVerify( LoRaMacRegion, &verify, PHY_TX_POWER ) == true )
            {
                LoRaMacParams.ChannelsTxPower = verify.TxPower;
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
        case MIB_SYSTEM_MAX_RX_ERROR:
        {
            LoRaMacParams.SystemMaxRxError = LoRaMacParamsDefaults.SystemMaxRxError = mibSet->Param.SystemMaxRxError;
            break;
        }
        case MIB_MIN_RX_SYMBOLS:
        {
            LoRaMacParams.MinRxSymbols = LoRaMacParamsDefaults.MinRxSymbols = mibSet->Param.MinRxSymbols;
            break;
        }
        case MIB_ANTENNA_GAIN:
        {
            LoRaMacParams.AntennaGain = mibSet->Param.AntennaGain;
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
    ChannelAddParams_t channelAdd;

    // Validate if the MAC is in a correct state
    if( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
    {
        if( ( LoRaMacState & LORAMAC_TX_CONFIG ) != LORAMAC_TX_CONFIG )
        {
            return LORAMAC_STATUS_BUSY;
        }
    }

    channelAdd.NewChannel = &params;
    channelAdd.ChannelId = id;

    return RegionChannelAdd( LoRaMacRegion, &channelAdd );
}

LoRaMacStatus_t LoRaMacChannelRemove( uint8_t id )
{
    ChannelRemoveParams_t channelRemove;

    if( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
    {
        if( ( LoRaMacState & LORAMAC_TX_CONFIG ) != LORAMAC_TX_CONFIG )
        {
            return LORAMAC_STATUS_BUSY;
        }
    }

    channelRemove.ChannelId = id;

    if( RegionChannelsRemove( LoRaMacRegion, &channelRemove ) == false )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacMulticastChannelLink( MulticastParams_t *channelParam )
{
    if( channelParam == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
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
    if( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
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
    AlternateDrParams_t altDr;
    VerifyParams_t verify;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;

    if( mlmeRequest == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
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
            if( ( LoRaMacState & LORAMAC_TX_DELAYED ) == LORAMAC_TX_DELAYED )
            {
                return LORAMAC_STATUS_BUSY;
            }

            if( ( mlmeRequest->Req.Join.DevEui == NULL ) ||
                ( mlmeRequest->Req.Join.AppEui == NULL ) ||
                ( mlmeRequest->Req.Join.AppKey == NULL ) ||
                ( mlmeRequest->Req.Join.NbTrials == 0 ) )
            {
                return LORAMAC_STATUS_PARAMETER_INVALID;
            }

            // Verify the parameter NbTrials for the join procedure
            verify.NbJoinTrials = mlmeRequest->Req.Join.NbTrials;

            if( RegionVerify( LoRaMacRegion, &verify, PHY_NB_JOIN_TRIALS ) == false )
            {
                // Value not supported, get default
                getPhy.Attribute = PHY_DEF_NB_JOIN_TRIALS;
                phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
                mlmeRequest->Req.Join.NbTrials = ( uint8_t ) phyParam.Value;
            }

            LoRaMacFlags.Bits.MlmeReq = 1;
            MlmeConfirm.MlmeRequest = mlmeRequest->Type;

            LoRaMacDevEui = mlmeRequest->Req.Join.DevEui;
            LoRaMacAppEui = mlmeRequest->Req.Join.AppEui;
            LoRaMacAppKey = mlmeRequest->Req.Join.AppKey;
            MaxJoinRequestTrials = mlmeRequest->Req.Join.NbTrials;

            // Reset variable JoinRequestTrials
            JoinRequestTrials = 0;

            // Setup header information
            macHdr.Value = 0;
            macHdr.Bits.MType  = FRAME_TYPE_JOIN_REQ;

            ResetMacParameters( );

            altDr.NbTrials = JoinRequestTrials + 1;

            LoRaMacParams.ChannelsDatarate = RegionAlternateDr( LoRaMacRegion, &altDr );

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
        case MLME_TXCW:
        {
            MlmeConfirm.MlmeRequest = mlmeRequest->Type;
            LoRaMacFlags.Bits.MlmeReq = 1;
            status = SetTxContinuousWave( mlmeRequest->Req.TxCw.Timeout );
            break;
        }
        case MLME_TXCW_1:
        {
            MlmeConfirm.MlmeRequest = mlmeRequest->Type;
            LoRaMacFlags.Bits.MlmeReq = 1;
            status = SetTxContinuousWave1( mlmeRequest->Req.TxCw.Timeout, mlmeRequest->Req.TxCw.Frequency, mlmeRequest->Req.TxCw.Power );
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
        case MLME_BEACON_ACQUISITION:
        {
            if( ( BeaconCtx.Ctrl.AcquisitionPending == 0 ) && ( BeaconCtx.Ctrl.AcquisitionTimerSet == 0 ) )
            {
                LoRaMacFlags.Bits.MlmeReq = 1;
                MlmeConfirmQueue[MlmeConfirmQueueCnt].MlmeRequest = mlmeRequest->Type;
                BeaconState = BEACON_STATE_ACQUISITION;

                // Start class B algorithm
                OnBeaconTimerEvent( );

                status = LORAMAC_STATUS_OK;
            }
            else
            {
                status = LORAMAC_STATUS_BUSY;
            }
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
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    LoRaMacStatus_t status = LORAMAC_STATUS_SERVICE_UNKNOWN;
    LoRaMacHeader_t macHdr;
    VerifyParams_t verify;
    uint8_t fPort = 0;
    void *fBuffer;
    uint16_t fBufferSize;
    int8_t datarate;
    bool readyToSend = false;

    if( mcpsRequest == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( ( ( LoRaMacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING ) ||
        ( ( LoRaMacState & LORAMAC_TX_DELAYED ) == LORAMAC_TX_DELAYED ) )
    {
        return LORAMAC_STATUS_BUSY;
    }

    macHdr.Value = 0;
    memset1 ( ( uint8_t* ) &McpsConfirm, 0, sizeof( McpsConfirm ) );
    McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;

    // AckTimeoutRetriesCounter must be reset every time a new request (unconfirmed or confirmed) is performed.
    AckTimeoutRetriesCounter = 1;

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

    // Get the minimum possible datarate
    getPhy.Attribute = PHY_MIN_TX_DR;
    getPhy.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;
    phyParam = RegionGetPhyParam( LoRaMacRegion, &getPhy );
    // Apply the minimum possible datarate.
    // Some regions have limitations for the minimum datarate.
    datarate = MAX( datarate, phyParam.Value );

    if( readyToSend == true )
    {
        if( AdrCtrlOn == false )
        {
            verify.DatarateParams.Datarate = datarate;
            verify.DatarateParams.UplinkDwellTime = LoRaMacParams.UplinkDwellTime;

            if( RegionVerify( LoRaMacRegion, &verify, PHY_TX_DR ) == true )
            {
                LoRaMacParams.ChannelsDatarate = verify.DatarateParams.Datarate;
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
    VerifyParams_t verify;

    verify.DutyCycle = enable;

    if( RegionVerify( LoRaMacRegion, &verify, PHY_DUTY_CYCLE ) == true )
    {
        DutyCycleOn = enable;
    }
}

void LoRaMacTestSetChannel( uint8_t channel )
{
    Channel = channel;
}
