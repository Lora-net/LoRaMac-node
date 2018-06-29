/*!
 * \file      LoRaMac.c
 *
 * \brief     LoRa MAC layer implementation
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
 */
#include "utilities.h"
#include "LoRaMac.h"
#include "LoRaMacClassB.h"
#include "LoRaMacCrypto.h"
#include "LoRaMacTest.h"
#include "LoRaMacConfirmQueue.h"
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

typedef struct sLoRaMacNvmCtx
{
    /*
     * LoRaMac region.
     */
    LoRaMacRegion_t Region;
    /*
     * LoRaMac default parameters
     */
    LoRaMacParams_t MacParamsDefaults;
    /*
     * Network ID ( 3 bytes )
     */
    uint32_t NetID;
    /*
     * Mote Address
     */
    uint32_t DevAddr;
    /*!
    * Multicast channel list
    */
    MulticastParams_t *MulticastChannelList;
    /*
     * Actual device class
     */
    DeviceClass_t DeviceClass;
    /*
     * Indicates if the node is connected to
     * a private or public network
     */
    bool PublicNetwork;
    /*
     * LoRaMac ADR control status
     */
    bool AdrCtrlOn;
    /*
     * Counts the number of missed ADR acknowledgements
     */
    uint32_t AdrAckCounter;
    /*
     * LoRaMac parameters
     */
    LoRaMacParams_t MacParams;
    /*
     * Uplink messages repetitions counter
     */
    uint8_t ChannelsNbTransCounter;
    /*
     * Maximum duty cycle
     * \remark Possibility to shutdown the device.
     */
    uint8_t MaxDCycle;
    /*
    * Enables/Disables duty cycle management (Test only)
    */
    bool DutyCycleOn;
    /*
     * Current channel index
     */
    uint8_t Channel;
    /*
     * Current channel index
     */
    uint8_t LastTxChannel;
    /*
     * Holds the current rx window slot
     */
    bool RepeaterSupport;
    /*
     * Buffer containing the MAC layer commands
     */
    uint8_t MacCommandsBuffer[LORA_MAC_COMMAND_MAX_LENGTH];
    /*
     * Buffer containing the MAC layer commands which must be repeated
     */
    uint8_t MacCommandsBufferToRepeat[LORA_MAC_COMMAND_MAX_LENGTH];
    /*
     * Number of trials to get a frame acknowledged
     */
    uint8_t AckTimeoutRetries;
    /*
     * Number of trials to get a frame acknowledged
     */
    uint8_t AckTimeoutRetriesCounter;
    /*
     * Indicates if the AckTimeout timer has expired or not
     */
    bool AckTimeoutRetry;
    /*
     * If the node has sent a FRAME_TYPE_DATA_CONFIRMED_UP this variable indicates
     * if the nodes needs to manage the server acknowledgement.
     */
    bool NodeAckRequested;
    /*
     * If the server has sent a FRAME_TYPE_DATA_CONFIRMED_DOWN this variable indicates
     * if the ACK bit must be set for the next transmission
     */
    bool SrvAckRequested;
    /*
     * Aggregated duty cycle management
     */
    uint16_t AggregatedDCycle;
    /*
     * End-Device network activation
     */
    bool IsLoRaMacNetworkJoined;
}LoRaMacNvmCtx_t;

typedef struct sLoRaMacCtx
{
    /*
     * Device IEEE EUI
     */
    uint8_t* DevEui;
    /*
    * App IEEE EUI
    */
    uint8_t* AppEui;
    /*
    * Length of packet in PktBuffer
    */
    uint16_t PktBufferLen;
    /*
    * Buffer containing the data to be sent or received.
    */
    uint8_t PktBuffer[LORAMAC_PHY_MAXPAYLOAD];
    /*!
    * Buffer containing the data received by the application.
    */
    uint8_t AppData[LORAMAC_PHY_MAXPAYLOAD];
    /*
    * Size of buffer containing the application data.
    */
    uint8_t AppDataSize;
    /*
    * Buffer containing the upper layer data.
    */
    uint8_t RxPayload[LORAMAC_PHY_MAXPAYLOAD];
    /*
    * Aggregated duty cycle management
    */
    TimerTime_t AggregatedLastTxDoneTime;
    TimerTime_t AggregatedTimeOff;
    /*!
    * Set to true, if the last uplink was a join request
    */
    bool LastTxIsJoinRequest;
    /*
    * Stores the time at LoRaMac initialization.
    *
    * \remark Used for the BACKOFF_DC computation.
    */
    TimerTime_t InitializationTime;
    SysTime_t LastTxSysTime;
    /*
    * LoRaMac internal state
    */
    uint32_t MacState;
    /*
    * LoRaMac upper layer event functions
    */
    LoRaMacPrimitives_t* MacPrimitives;
    /*
    * LoRaMac upper layer callback functions
    */
    LoRaMacCallback_t* MacCallbacks;
    /*
    * Radio events function pointer
    */
    RadioEvents_t RadioEvents;
    /*
    * LoRaMac duty cycle delayed Tx timer
    */
    TimerEvent_t TxDelayedTimer;
    /*
    * LoRaMac reception windows timers
    */
    TimerEvent_t RxWindowTimer1;
    TimerEvent_t RxWindowTimer2;
    /*
    * LoRaMac reception windows delay
    * \remark normal frame: RxWindowXDelay = ReceiveDelayX - RADIO_WAKEUP_TIME
    *         join frame  : RxWindowXDelay = JoinAcceptDelayX - RADIO_WAKEUP_TIME
    */
    uint32_t RxWindow1Delay;
    uint32_t RxWindow2Delay;
    /*
    * LoRaMac Rx windows configuration
    */
    RxConfigParams_t RxWindow1Config;
    RxConfigParams_t RxWindow2Config;
    /*
    * Acknowledge timeout timer. Used for packet retransmissions.
    */
    TimerEvent_t AckTimeoutTimer;
    /*
    * Last transmission time on air
    */
    TimerTime_t TxTimeOnAir;
    /*
    * Structure to hold an MCPS indication data.
    */
    McpsIndication_t McpsIndication;
    /*
    * Structure to hold MCPS confirm data.
    */
    McpsConfirm_t McpsConfirm;
    /*
    * Structure to hold MLME confirm data.
    */
    MlmeConfirm_t MlmeConfirm;
    /*
    * Structure to hold MLME indication data.
    */
    MlmeIndication_t MlmeIndication;
    /*
    * Holds the current rx window slot
    */
    LoRaMacRxSlot_t RxSlot;
    /*
    * LoRaMac tx/rx operation state
    */
    LoRaMacFlags_t MacFlags;
    /*
    * Non-volatile module context structure
    */
    LoRaMacNvmCtx_t* NvmCtx;
}LoRaMacCtx_t;

/*
 * Module context.
 */
static LoRaMacCtx_t MacCtx;

/*
 * Non-volatile module context.
 */
static LoRaMacNvmCtx_t NvmMacCtx;

/*!
 * Defines the LoRaMac radio events status
 */
typedef union uLoRaMacRadioEvents
{
    uint32_t Value;
    struct sEvents
    {
        uint32_t RxTimeout : 1;
        uint32_t RxError   : 1;
        uint32_t TxTimeout : 1;
        uint32_t RxDone    : 1;
        uint32_t TxDone    : 1;
    }Events;
}LoRaMacRadioEvents_t;

/*!
 * LoRaMac radio events status
 */
LoRaMacRadioEvents_t LoRaMacRadioEvents = { .Value = 0 };

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
static void OnRadioRxDone( uint8_t* payload, uint16_t size, int16_t rssi, int8_t snr );

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
 * \brief Check if the OnAckTimeoutTimer has do be disabled. If so, the
 *        function disables it.
 *
 * \param [IN] nodeAckRequested Set to true, if the node has requested an ACK
 * \param [IN] class The device class
 * \param [IN] ackReceived Set to true, if the node has received an ACK
 * \param [IN] ackTimeoutRetriesCounter Retries counter for confirmed uplinks
 * \param [IN] ackTimeoutRetries Maximum retries for confirmed uplinks
 */
static void CheckToDisableAckTimeout( bool nodeAckRequested, DeviceClass_t devClass, bool ackReceived,
                                      uint8_t ackTimeoutRetriesCounter, uint8_t ackTimeoutRetries );

/*!
 * \brief Function executed on AckTimeout timer event
 */
static void OnAckTimeoutTimerEvent( void );

/*!
 * \brief Initializes and opens the reception window
 *
 * \param [IN] rxContinuous Set to true, if the RX is in continuous mode
 * \param [IN] maxRxWindow Maximum RX window timeout
 */
static void RxWindowSetup( bool rxContinuous, uint32_t maxRxWindow );

/*!
 * \brief Verifies if sticky MAC commands are pending.
 *
 * \retval [true: sticky MAC commands pending, false: No MAC commands pending]
 */
static bool IsStickyMacCommandPending( void );

/*!
 * \brief Configures the events to trigger an MLME-Indication with
 *        a MLME type of MLME_SCHEDULE_UPLINK.
 */
static void SetMlmeScheduleUplinkIndication( void );

/*!
 * \brief Switches the device class
 *
 * \param [IN] deviceClass Device class to switch to
 */
static LoRaMacStatus_t SwitchClass( DeviceClass_t deviceClass );

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
 *
 * \param [IN] payload      A pointer to the payload
 * \param [IN] macIndex     The index of the payload where the MAC commands start
 * \param [IN] commandsSize The size of the MAC commands
 * \param [IN] snr          The SNR value  of the frame
 * \param [IN] rxSlot       The RX slot where the frame was received
 */
static void ProcessMacCommands( uint8_t *payload, uint8_t macIndex, uint8_t commandsSize, uint8_t snr, LoRaMacRxSlot_t rxSlot );

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
 * \param [IN] allowDelayedTx When set to true, the a frame will be delayed,
 *                            the duty cycle restriction is active
 * \retval Status of the operation
 */
static LoRaMacStatus_t ScheduleTx( bool allowDelayedTx );

/*
 * \brief Calculates the back-off time for the band of a channel.
 *
 * \param [IN] channel     The last Tx channel index
 */
static void CalculateBackOff( uint8_t channel );

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
 * \param [IN] power       RF output power to be set.
 * \retval status          Status of the operation.
 */
LoRaMacStatus_t SetTxContinuousWave1( uint16_t timeout, uint32_t frequency, uint8_t power );

/*!
 * \brief Resets MAC specific parameters to default
 */
static void ResetMacParameters( void );

/*!
 * \brief Resets MAC specific parameters to default
 *
 * \param [IN] fPort     The fPort
 *
 * \retval [false: fPort not allowed, true: fPort allowed]
 */
static bool IsFPortAllowed( uint8_t fPort );

/*!
 * \brief Opens up a continuous RX 2 window. This is used for
 *        class c devices.
 */
static void OpenContinuousRx2Window( void );









/*!
 * Structure used to store the radio Tx event data
 */
struct
{
    TimerTime_t CurTime;
}TxDoneParams;

/*!
 * Structure used to store the radio Rx event data
 */
struct
{
    TimerTime_t LastRxDone;
    uint8_t *Payload;
    uint16_t Size;
    int16_t Rssi;
    int8_t Snr;
}RxDoneParams;

static void OnRadioTxDone( void )
{
    TxDoneParams.CurTime = TimerGetCurrentTime( );
    MacCtx.LastTxSysTime = SysTimeGet( );

    LoRaMacRadioEvents.Events.TxDone = 1;
}

static void OnRadioRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
    RxDoneParams.LastRxDone = TimerGetCurrentTime( );
    RxDoneParams.Payload = payload;
    RxDoneParams.Size = size;
    RxDoneParams.Rssi = rssi;
    RxDoneParams.Snr = snr;

    LoRaMacRadioEvents.Events.RxDone = 1;
}

static void OnRadioTxTimeout( void )
{
    LoRaMacRadioEvents.Events.TxTimeout = 1;
}

static void OnRadioRxError( void )
{
    LoRaMacRadioEvents.Events.RxError = 1;
}

static void OnRadioRxTimeout( void )
{
    LoRaMacRadioEvents.Events.RxTimeout = 1;
}

static void ProcessRadioTxDone( void )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    SetBandTxDoneParams_t txDone;

    if( MacCtx.NvmCtx->DeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OpenContinuousRx2Window( );
    }

    // Setup timers
    TimerSetValue( &MacCtx.RxWindowTimer1, MacCtx.RxWindow1Delay );
    TimerStart( &MacCtx.RxWindowTimer1 );
    if( MacCtx.NvmCtx->DeviceClass != CLASS_C )
    {
        TimerSetValue( &MacCtx.RxWindowTimer2, MacCtx.RxWindow2Delay );
        TimerStart( &MacCtx.RxWindowTimer2 );
    }
    if( ( MacCtx.NvmCtx->DeviceClass == CLASS_C ) || ( MacCtx.NvmCtx->NodeAckRequested == true ) )
    {
        getPhy.Attribute = PHY_ACK_TIMEOUT;
        phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
        TimerSetValue( &MacCtx.AckTimeoutTimer, MacCtx.RxWindow2Delay + phyParam.Value );
        TimerStart( &MacCtx.AckTimeoutTimer );
    }

    // Verify if the last uplink was a join request
    if( ( MacCtx.MacFlags.Bits.MlmeReq == 1 ) && ( MacCtx.MlmeConfirm.MlmeRequest == MLME_JOIN ) )
    {
        MacCtx.LastTxIsJoinRequest = true;
    }
    else
    {
        MacCtx.LastTxIsJoinRequest = false;
    }

    // Store last Tx channel
    MacCtx.NvmCtx->LastTxChannel = MacCtx.NvmCtx->Channel;
    // Update last tx done time for the current channel
    txDone.Channel = MacCtx.NvmCtx->Channel;

    txDone.Joined = MacCtx.NvmCtx->IsLoRaMacNetworkJoined;
    txDone.LastTxDoneTime = TxDoneParams.CurTime;
    RegionSetBandTxDone( MacCtx.NvmCtx->Region, &txDone );
    // Update Aggregated last tx done time
    MacCtx.AggregatedLastTxDoneTime = TxDoneParams.CurTime;

    if( MacCtx.NvmCtx->NodeAckRequested == false )
    {
        MacCtx.McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;
        MacCtx.NvmCtx->ChannelsNbTransCounter++;
    }
}

static void PrepareRxDoneAbort( void )
{
    MacCtx.MacState |= LORAMAC_RX_ABORT;

    if( MacCtx.NvmCtx->NodeAckRequested )
    {
        OnAckTimeoutTimerEvent( );
    }

    MacCtx.MacFlags.Bits.McpsInd = 1;
    MacCtx.MacFlags.Bits.MacDone = 1;
}

static void ProcessRadioRxDone( void )
{
    LoRaMacHeader_t macHdr;
    LoRaMacFrameCtrl_t fCtrl;
    ApplyCFListParams_t applyCFList;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;

    uint8_t *payload = RxDoneParams.Payload;
    uint16_t size = RxDoneParams.Size;
    int16_t rssi = RxDoneParams.Rssi;
    int8_t snr = RxDoneParams.Snr;

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

    MacCtx.McpsConfirm.AckReceived = false;
    MacCtx.McpsIndication.Rssi = rssi;
    MacCtx.McpsIndication.Snr = snr;
    MacCtx.McpsIndication.RxSlot = MacCtx.RxSlot;
    MacCtx.McpsIndication.Port = 0;
    MacCtx.McpsIndication.Multicast = 0;
    MacCtx.McpsIndication.FramePending = 0;
    MacCtx.McpsIndication.Buffer = NULL;
    MacCtx.McpsIndication.BufferSize = 0;
    MacCtx.McpsIndication.RxData = false;
    MacCtx.McpsIndication.AckReceived = false;
    MacCtx.McpsIndication.DownLinkCounter = 0;
    MacCtx.McpsIndication.McpsIndication = MCPS_UNCONFIRMED;

    Radio.Sleep( );
    TimerStop( &MacCtx.RxWindowTimer2 );

    // This function must be called even if we are not in class b mode yet.
    if( LoRaMacClassBRxBeacon( payload, size ) == true )
    {
        MacCtx.MlmeIndication.BeaconInfo.Rssi = rssi;
        MacCtx.MlmeIndication.BeaconInfo.Snr = snr;
        return;
    }
    // Check if we expect a ping or a multicast slot.
    if( MacCtx.NvmCtx->DeviceClass == CLASS_B )
    {
        if( LoRaMacClassBIsPingExpected( ) == true )
        {
            LoRaMacClassBSetPingSlotState( PINGSLOT_STATE_CALC_PING_OFFSET );
            LoRaMacClassBPingSlotTimerEvent( );
             MacCtx.McpsIndication.RxSlot = RX_SLOT_WIN_PING_SLOT;
        }
        else if( LoRaMacClassBIsMulticastExpected( ) == true )
        {
            LoRaMacClassBSetMulticastSlotState( PINGSLOT_STATE_CALC_PING_OFFSET );
            LoRaMacClassBMulticastSlotTimerEvent( );
            MacCtx.McpsIndication.RxSlot = RX_SLOT_WIN_MULTICAST_SLOT;
        }
    }

    macHdr.Value = payload[pktHeaderLen++];

    switch( macHdr.Bits.MType )
    {
        case FRAME_TYPE_JOIN_ACCEPT:
            if( MacCtx.NvmCtx->IsLoRaMacNetworkJoined == true )
            {
                MacCtx.McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
                PrepareRxDoneAbort( );
                return;
            }
            LoRaMacJoinDecrypt( payload + 1, size - 1, LoRaMacAppKey, MacCtx.RxPayload + 1 );

            MacCtx.RxPayload[0] = macHdr.Value;

            LoRaMacJoinComputeMic( MacCtx.RxPayload, size - LORAMAC_MFR_LEN, LoRaMacAppKey, &mic );

            micRx |= ( uint32_t )MacCtx.RxPayload[size - LORAMAC_MFR_LEN];
            micRx |= ( ( uint32_t )MacCtx.RxPayload[size - LORAMAC_MFR_LEN + 1] << 8 );
            micRx |= ( ( uint32_t )MacCtx.RxPayload[size - LORAMAC_MFR_LEN + 2] << 16 );
            micRx |= ( ( uint32_t )MacCtx.RxPayload[size - LORAMAC_MFR_LEN + 3] << 24 );

            if( LoRaMacConfirmQueueIsCmdActive( MLME_JOIN ) == true )
            {
                if( micRx == mic )
                {
                    LoRaMacJoinComputeSKeys( LoRaMacAppKey, MacCtx.RxPayload + 1, LoRaMacDevNonce, LoRaMacNwkSKey, LoRaMacAppSKey );

                    MacCtx.NvmCtx->NetID = ( uint32_t )MacCtx.RxPayload[4];
                    MacCtx.NvmCtx->NetID |= ( ( uint32_t )MacCtx.RxPayload[5] << 8 );
                    MacCtx.NvmCtx->NetID |= ( ( uint32_t )MacCtx.RxPayload[6] << 16 );

                    MacCtx.NvmCtx->DevAddr = ( uint32_t )MacCtx.RxPayload[7];
                    MacCtx.NvmCtx->DevAddr |= ( ( uint32_t )MacCtx.RxPayload[8] << 8 );
                    MacCtx.NvmCtx->DevAddr |= ( ( uint32_t )MacCtx.RxPayload[9] << 16 );
                    MacCtx.NvmCtx->DevAddr |= ( ( uint32_t )MacCtx.RxPayload[10] << 24 );

                    // DLSettings
                    MacCtx.NvmCtx->MacParams.Rx1DrOffset = ( MacCtx.RxPayload[11] >> 4 ) & 0x07;
                    MacCtx.NvmCtx->MacParams.Rx2Channel.Datarate = MacCtx.RxPayload[11] & 0x0F;

                    // RxDelay
                    MacCtx.NvmCtx->MacParams.ReceiveDelay1 = ( MacCtx.RxPayload[12] & 0x0F );
                    if( MacCtx.NvmCtx->MacParams.ReceiveDelay1 == 0 )
                    {
                        MacCtx.NvmCtx->MacParams.ReceiveDelay1 = 1;
                    }
                    MacCtx.NvmCtx->MacParams.ReceiveDelay1 *= 1000;
                    MacCtx.NvmCtx->MacParams.ReceiveDelay2 = MacCtx.NvmCtx->MacParams.ReceiveDelay1 + 1000;

                    // Apply CF list
                    applyCFList.Payload = &MacCtx.RxPayload[13];
                    // Size of the regular payload is 12. Plus 1 byte MHDR and 4 bytes MIC
                    applyCFList.Size = size - 17;

                    RegionApplyCFList( MacCtx.NvmCtx->Region, &applyCFList );

                    LoRaMacConfirmQueueSetStatus( LORAMAC_EVENT_INFO_STATUS_OK, MLME_JOIN );
                    MacCtx.NvmCtx->IsLoRaMacNetworkJoined = true;
                }
                else
                {
                    LoRaMacConfirmQueueSetStatus( LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL, MLME_JOIN );
                }
            }
            break;
        case FRAME_TYPE_DATA_CONFIRMED_DOWN:
        case FRAME_TYPE_DATA_UNCONFIRMED_DOWN:
            // Check if the received payload size is valid
            getPhy.UplinkDwellTime = MacCtx.NvmCtx->MacParams.DownlinkDwellTime;
            getPhy.Datarate = MacCtx.McpsIndication.RxDatarate;
            getPhy.Attribute = PHY_MAX_PAYLOAD;

            // Get the maximum payload length
            if( MacCtx.NvmCtx->RepeaterSupport == true )
            {
                getPhy.Attribute = PHY_MAX_PAYLOAD_REPEATER;
            }
            phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
            if( MAX( 0, ( int16_t )( ( int16_t )size - ( int16_t )LORA_MAC_FRMPAYLOAD_OVERHEAD ) ) > phyParam.Value )
            {
                MacCtx.McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
                PrepareRxDoneAbort( );
                return;
            }

            address = payload[pktHeaderLen++];
            address |= ( (uint32_t)payload[pktHeaderLen++] << 8 );
            address |= ( (uint32_t)payload[pktHeaderLen++] << 16 );
            address |= ( (uint32_t)payload[pktHeaderLen++] << 24 );

            fCtrl.Value = payload[pktHeaderLen++];

            if( address != MacCtx.NvmCtx->DevAddr )
            {
                curMulticastParams = MacCtx.NvmCtx->MulticastChannelList;
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
                    MacCtx.McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL;
                    PrepareRxDoneAbort( );
                    return;
                }
                if( ( macHdr.Bits.MType != FRAME_TYPE_DATA_UNCONFIRMED_DOWN ) ||
                    ( fCtrl.Bits.Ack == 1 ) ||
                    ( fCtrl.Bits.AdrAckReq == 1 ) )
                {
                    // Wrong multicast message format. Refer to chapter 11.2.2 of the specification
                    MacCtx.McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_MULTICAST_FAIL;
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
            phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
            if( sequenceCounterDiff >= phyParam.Value )
            {
                MacCtx.McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_TOO_MANY_FRAMES_LOSS;
                MacCtx.McpsIndication.DownLinkCounter = downLinkCounter;
                PrepareRxDoneAbort( );
                return;
            }

            if( isMicOk == true )
            {
                MacCtx.McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
                MacCtx.McpsIndication.Multicast = multicast;
                MacCtx.McpsIndication.FramePending = fCtrl.Bits.FPending;
                MacCtx.McpsIndication.Buffer = NULL;
                MacCtx.McpsIndication.BufferSize = 0;
                MacCtx.McpsIndication.DownLinkCounter = downLinkCounter;

                MacCtx.McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_OK;

                MacCtx.NvmCtx->AdrAckCounter = 0;
                MacCommandsBufferToRepeatIndex = 0;

                // Update 32 bits downlink counter
                if( multicast == 1 )
                {
                    MacCtx.McpsIndication.McpsIndication = MCPS_MULTICAST;

                    if( ( curMulticastParams->DownLinkCounter == downLinkCounter ) &&
                        ( curMulticastParams->DownLinkCounter != 0 ) )
                    {
                        MacCtx.McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED;
                        MacCtx.McpsIndication.DownLinkCounter = downLinkCounter;
                        PrepareRxDoneAbort( );
                        return;
                    }
                    curMulticastParams->DownLinkCounter = downLinkCounter;
                }
                else
                {
                    if( macHdr.Bits.MType == FRAME_TYPE_DATA_CONFIRMED_DOWN )
                    {
                        MacCtx.NvmCtx->SrvAckRequested = true;
                        MacCtx.McpsIndication.McpsIndication = MCPS_CONFIRMED;

                        if( ( DownLinkCounter == downLinkCounter ) &&
                            ( DownLinkCounter != 0 ) )
                        {
                            // Duplicated confirmed downlink. Skip indication.
                            // In this case, the MAC layer shall accept the MAC commands
                            // which are included in the downlink retransmission.
                            // It should not provide the same frame to the application
                            // layer again. The MAC layer accepts the acknowledgement.
                            MacCtx.MacFlags.Bits.McpsIndSkip = 1;
                        }
                    }
                    else
                    {
                        MacCtx.NvmCtx->SrvAckRequested = false;
                        MacCtx.McpsIndication.McpsIndication = MCPS_UNCONFIRMED;

                        if( ( DownLinkCounter == downLinkCounter ) &&
                            ( DownLinkCounter != 0 ) )
                        {
                            MacCtx.McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_DOWNLINK_REPEATED;
                            MacCtx.McpsIndication.DownLinkCounter = downLinkCounter;
                            PrepareRxDoneAbort( );
                            return;
                        }
                    }
                    DownLinkCounter = downLinkCounter;
                }

                // This must be done before parsing the payload and the MAC commands.
                // We need to reset the MacCommandsBufferIndex here, since we need
                // to take retransmissions and repetitions into account. Error cases
                // will be handled in function LoRaMacProcess.
                if( MacCtx.McpsConfirm.McpsRequest == MCPS_CONFIRMED )
                {
                    if( fCtrl.Bits.Ack == 1 )
                    {// Reset MacCommandsBufferIndex when we have received an ACK.
                        MacCommandsBufferIndex = 0;
                        // Update acknowledgement information
                        MacCtx.McpsConfirm.AckReceived = fCtrl.Bits.Ack;
                        MacCtx.McpsIndication.AckReceived = fCtrl.Bits.Ack;
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

                    MacCtx.McpsIndication.Port = port;

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
                                                   MacCtx.RxPayload );

                            // Decode frame payload MAC commands
                            ProcessMacCommands( MacCtx.RxPayload, 0, frameLen, snr, MacCtx.McpsIndication.RxSlot );
                        }
                        else
                        {
                            MacCtx.MacFlags.Bits.McpsIndSkip = 1;
                            // This is not a valid frame. Drop it and reset the ACK bits
                            MacCtx.McpsConfirm.AckReceived = false;
                            MacCtx.McpsIndication.AckReceived = false;
                        }
                    }
                    else
                    {
                        if( ( fCtrl.Bits.FOptsLen > 0 ) && ( multicast == 0 ) )
                        {
                            // Decode Options field MAC commands. Omit the fPort.
                            ProcessMacCommands( payload, 8, appPayloadStartIndex - 1, snr, MacCtx.McpsIndication.RxSlot );
                        }

                        LoRaMacPayloadDecrypt( payload + appPayloadStartIndex,
                                               frameLen,
                                               appSKey,
                                               address,
                                               DOWN_LINK,
                                               downLinkCounter,
                                               MacCtx.RxPayload );

                        MacCtx.McpsIndication.Buffer = MacCtx.RxPayload;
                        MacCtx.McpsIndication.BufferSize = frameLen;
                        MacCtx.McpsIndication.RxData = true;
                    }
                }
                else
                {
                    if( fCtrl.Bits.FOptsLen > 0 )
                    {
                        // Decode Options field MAC commands
                        ProcessMacCommands( payload, 8, appPayloadStartIndex, snr, MacCtx.McpsIndication.RxSlot );
                    }
                }

                // Provide always an indication, skip the callback to the user application,
                // in case of a confirmed downlink retransmission.
                MacCtx.MacFlags.Bits.McpsInd = 1;
            }
            else
            {
                MacCtx.McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_MIC_FAIL;

                PrepareRxDoneAbort( );
                return;
            }
            break;
        case FRAME_TYPE_PROPRIETARY:
            memcpy1( MacCtx.RxPayload, &payload[pktHeaderLen], size );

            MacCtx.McpsIndication.McpsIndication = MCPS_PROPRIETARY;
            MacCtx.McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_OK;
            MacCtx.McpsIndication.Buffer = MacCtx.RxPayload;
            MacCtx.McpsIndication.BufferSize = size - pktHeaderLen;

            MacCtx.MacFlags.Bits.McpsInd = 1;
            break;
        default:
            MacCtx.McpsIndication.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            PrepareRxDoneAbort( );
            break;
    }

    // Verify if we need to disable the AckTimeoutTimer
    CheckToDisableAckTimeout( MacCtx.NvmCtx->NodeAckRequested, MacCtx.NvmCtx->DeviceClass, MacCtx.McpsConfirm.AckReceived,
                                MacCtx.NvmCtx->AckTimeoutRetriesCounter, MacCtx.NvmCtx->AckTimeoutRetries );

    if( MacCtx.AckTimeoutTimer.IsRunning == false )
    {// Procedure is completed when the AckTimeoutTimer is not running anymore
        MacCtx.MacFlags.Bits.MacDone = 1;
    }
}

static void ProcessRadioTxTimeout( void )
{
    if( MacCtx.NvmCtx->DeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }
    else
    {
        OpenContinuousRx2Window( );
    }

    MacCtx.McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT;
    LoRaMacConfirmQueueSetStatusCmn( LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT );
    MacCtx.MacFlags.Bits.MacDone = 1;
}

static void ProcessRadioRxError( void )
{
    bool classBRx = false;

    if( MacCtx.NvmCtx->DeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }

    if( LoRaMacClassBIsBeaconExpected( ) == true )
    {
        LoRaMacClassBSetBeaconState( BEACON_STATE_TIMEOUT );
        LoRaMacClassBBeaconTimerEvent( );
        classBRx = true;
    }
    if( MacCtx.NvmCtx->DeviceClass == CLASS_B )
    {
        if( LoRaMacClassBIsPingExpected( ) == true )
        {
            LoRaMacClassBSetPingSlotState( PINGSLOT_STATE_CALC_PING_OFFSET );
            LoRaMacClassBPingSlotTimerEvent( );
            classBRx = true;
        }
        if( LoRaMacClassBIsMulticastExpected( ) == true )
        {
            LoRaMacClassBSetMulticastSlotState( PINGSLOT_STATE_CALC_PING_OFFSET );
            LoRaMacClassBMulticastSlotTimerEvent( );
            classBRx = true;
        }
    }

    if( classBRx == false )
    {
        if( MacCtx.RxSlot == RX_SLOT_WIN_1 )
        {
            if( MacCtx.NvmCtx->NodeAckRequested == true )
            {
                MacCtx.McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX1_ERROR;
            }
            LoRaMacConfirmQueueSetStatusCmn( LORAMAC_EVENT_INFO_STATUS_RX1_ERROR );

            if( MacCtx.NvmCtx->DeviceClass != CLASS_C )
            {
                if( TimerGetElapsedTime( MacCtx.AggregatedLastTxDoneTime ) >= MacCtx.RxWindow2Delay )
                {
                    TimerStop( &MacCtx.RxWindowTimer2 );
                    MacCtx.MacFlags.Bits.MacDone = 1;
                }
            }
        }
        else
        {
            if( MacCtx.NvmCtx->NodeAckRequested == true )
            {
                MacCtx.McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_ERROR;
            }
            LoRaMacConfirmQueueSetStatusCmn( LORAMAC_EVENT_INFO_STATUS_RX2_ERROR );

            if( MacCtx.NvmCtx->DeviceClass != CLASS_C )
            {
                MacCtx.MacFlags.Bits.MacDone = 1;
            }
        }
    }

    if( MacCtx.NvmCtx->DeviceClass == CLASS_C )
    {
        OpenContinuousRx2Window( );
    }
}

static void ProcessRadioRxTimeout( void )
{
    bool classBRx = false;

    if( MacCtx.NvmCtx->DeviceClass != CLASS_C )
    {
        Radio.Sleep( );
    }

    if( LoRaMacClassBIsBeaconExpected( ) == true )
    {
        LoRaMacClassBSetBeaconState( BEACON_STATE_TIMEOUT );
        LoRaMacClassBBeaconTimerEvent( );
        classBRx = true;
    }
    if( MacCtx.NvmCtx->DeviceClass == CLASS_B )
    {
        if( LoRaMacClassBIsPingExpected( ) == true )
        {
            LoRaMacClassBSetPingSlotState( PINGSLOT_STATE_CALC_PING_OFFSET );
            LoRaMacClassBPingSlotTimerEvent( );
            classBRx = true;
        }
        if( LoRaMacClassBIsMulticastExpected( ) == true )
        {
            LoRaMacClassBSetMulticastSlotState( PINGSLOT_STATE_CALC_PING_OFFSET );
            LoRaMacClassBMulticastSlotTimerEvent( );
            classBRx = true;
        }
    }

    if( classBRx == false )
    {
        if( MacCtx.RxSlot == RX_SLOT_WIN_1 )
        {
            if( MacCtx.NvmCtx->NodeAckRequested == true )
            {
                MacCtx.McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT;
            }
            LoRaMacConfirmQueueSetStatusCmn( LORAMAC_EVENT_INFO_STATUS_RX1_TIMEOUT );

            if( MacCtx.NvmCtx->DeviceClass != CLASS_C )
            {
                if( TimerGetElapsedTime( MacCtx.AggregatedLastTxDoneTime ) >= MacCtx.RxWindow2Delay )
                {
                    TimerStop( &MacCtx.RxWindowTimer2 );
                    MacCtx.MacFlags.Bits.MacDone = 1;
                }
            }
        }
        else
        {
            if( MacCtx.NvmCtx->NodeAckRequested == true )
            {
                MacCtx.McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT;
            }
            LoRaMacConfirmQueueSetStatusCmn( LORAMAC_EVENT_INFO_STATUS_RX2_TIMEOUT );

            if( MacCtx.NvmCtx->DeviceClass != CLASS_C )
            {
                MacCtx.MacFlags.Bits.MacDone = 1;
            }
        }
    }

    if( MacCtx.NvmCtx->DeviceClass == CLASS_C )
    {
        OpenContinuousRx2Window( );
    }
}

static void LoRaMacHandleIrqEvents( void )
{
    LoRaMacRadioEvents_t events = LoRaMacRadioEvents;
    if( events.Value != 0 )
    {
        CRITICAL_SECTION_BEGIN( );
        LoRaMacRadioEvents.Value = 0;
        CRITICAL_SECTION_END( );

        if( events.Events.TxDone == 1 )
        {
            ProcessRadioTxDone( );
        }
        if( events.Events.RxDone == 1 )
        {
            ProcessRadioRxDone( );
        }
        if( events.Events.TxTimeout == 1 )
        {
            ProcessRadioTxTimeout( );
        }
        if( events.Events.RxError == 1 )
        {
            ProcessRadioRxError( );
        }
        if( events.Events.RxTimeout == 1 )
        {
            ProcessRadioRxTimeout( );
        }
    }
}

void LoRaMacProcess( void )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    bool noTx = false;

    LoRaMacHandleIrqEvents( );
    LoRaMacClassBProcess( );

    if( MacCtx.MacFlags.Bits.MacDone == 1 )
    {
        if( ( MacCtx.MacState & LORAMAC_RX_ABORT ) == LORAMAC_RX_ABORT )
        {
            MacCtx.MacState &= ~LORAMAC_RX_ABORT;
            MacCtx.MacState &= ~LORAMAC_TX_RUNNING;
        }

        if( ( MacCtx.MacFlags.Bits.MlmeReq == 1 ) ||
            ( MacCtx.MacFlags.Bits.McpsReq == 1 ) )
        {
            // Get a status of any request and check if we have a TX timeout
            MacCtx.MlmeConfirm.Status = LoRaMacConfirmQueueGetStatusCmn( );

            if( ( MacCtx.McpsConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) ||
                ( MacCtx.MlmeConfirm.Status == LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT ) )
            {
                // Stop transmit cycle due to tx timeout.
                MacCtx.MacState &= ~LORAMAC_TX_RUNNING;
                MacCommandsBufferIndex = 0;
                MacCtx.McpsConfirm.NbRetries = MacCtx.NvmCtx->AckTimeoutRetriesCounter;
                MacCtx.McpsConfirm.AckReceived = false;
                MacCtx.McpsConfirm.TxTimeOnAir = 0;
                noTx = true;
            }

            if( ( LoRaMacConfirmQueueIsCmdActive( MLME_BEACON_ACQUISITION ) == true ) &&
                ( MacCtx.MacFlags.Bits.McpsReq == 0 ) )
            {
                if( MacCtx.MacFlags.Bits.MlmeReq == 1 )
                {
                    noTx = true;
                    MacCtx.MacState &= ~LORAMAC_TX_RUNNING;
                }
            }
        }

        if( ( MacCtx.NvmCtx->NodeAckRequested == false ) && ( noTx == false ) )
        {
            if( ( MacCtx.MacFlags.Bits.MlmeReq == 1 ) || ( ( MacCtx.MacFlags.Bits.McpsReq == 1 ) ) )
            {
                if( ( MacCtx.MacFlags.Bits.MlmeReq == 1 ) && ( LoRaMacConfirmQueueIsCmdActive( MLME_JOIN ) == true ) )
                {// Procedure for the join request
                    if( LoRaMacConfirmQueueGetStatus( MLME_JOIN ) == LORAMAC_EVENT_INFO_STATUS_OK )
                    {// Node joined successfully
                        UpLinkCounter = 0;
                        MacCtx.NvmCtx->ChannelsNbTransCounter = 0;
                    }
                    MacCtx.MacState &= ~LORAMAC_TX_RUNNING;
                }
                else
                {// Procedure for all other frames
                    if( ( MacCtx.NvmCtx->ChannelsNbTransCounter >= MacCtx.NvmCtx->MacParams.ChannelsNbTrans ) || ( MacCtx.MacFlags.Bits.McpsInd == 1 ) )
                    {
                        if( MacCtx.MacFlags.Bits.McpsInd == 0 )
                        {   // Maximum repetitions without downlink. Reset MacCommandsBufferIndex. Increase ADR Ack counter.
                            // Only process the case when the MAC did not receive a downlink.
                            MacCommandsBufferIndex = 0;
                            MacCtx.NvmCtx->AdrAckCounter++;
                        }

                        MacCtx.NvmCtx->ChannelsNbTransCounter = 0;
                        UpLinkCounter++;
                        MacCtx.MacState &= ~LORAMAC_TX_RUNNING;
                    }
                    else
                    {
                        MacCtx.MacFlags.Bits.MacDone = 0;
                        // Sends the same frame again
                        OnTxDelayedTimerEvent( );
                    }
                }
            }
        }

        if( MacCtx.MacFlags.Bits.McpsInd == 1 )
        {// Procedure if we received a frame
            if( ( MacCtx.McpsConfirm.AckReceived == true ) || ( MacCtx.NvmCtx->AckTimeoutRetriesCounter > MacCtx.NvmCtx->AckTimeoutRetries ) )
            {
                MacCtx.NvmCtx->AckTimeoutRetry = false;
                MacCtx.NvmCtx->NodeAckRequested = false;
                UpLinkCounter++;
                MacCtx.McpsConfirm.NbRetries = MacCtx.NvmCtx->AckTimeoutRetriesCounter;

                MacCtx.MacState &= ~LORAMAC_TX_RUNNING;
            }
        }

        if( ( MacCtx.NvmCtx->AckTimeoutRetry == true ) && ( ( MacCtx.MacState & LORAMAC_TX_DELAYED ) == 0 ) )
        {// Retransmissions procedure for confirmed uplinks
            MacCtx.NvmCtx->AckTimeoutRetry = false;
            if( ( MacCtx.NvmCtx->AckTimeoutRetriesCounter < MacCtx.NvmCtx->AckTimeoutRetries ) && ( MacCtx.NvmCtx->AckTimeoutRetriesCounter <= MAX_ACK_RETRIES ) )
            {
                MacCtx.NvmCtx->AckTimeoutRetriesCounter++;

                if( ( MacCtx.NvmCtx->AckTimeoutRetriesCounter % 2 ) == 1 )
                {
                    getPhy.Attribute = PHY_NEXT_LOWER_TX_DR;
                    getPhy.UplinkDwellTime = MacCtx.NvmCtx->MacParams.UplinkDwellTime;
                    getPhy.Datarate = MacCtx.NvmCtx->MacParams.ChannelsDatarate;
                    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
                    MacCtx.NvmCtx->MacParams.ChannelsDatarate = phyParam.Value;
                }
                // Try to send the frame again. Allow delayed frame transmissions
                if( ScheduleTx( true ) == LORAMAC_STATUS_OK )
                {
                    MacCtx.MacFlags.Bits.MacDone = 0;
                }
                else
                {
                    // The DR is not applicable for the payload size
                    MacCtx.McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_TX_DR_PAYLOAD_SIZE_ERROR;

                    MacCommandsBufferIndex = 0;
                    MacCtx.MacState &= ~LORAMAC_TX_RUNNING;
                    MacCtx.NvmCtx->NodeAckRequested = false;
                    MacCtx.McpsConfirm.AckReceived = false;
                    MacCtx.McpsConfirm.NbRetries = MacCtx.NvmCtx->AckTimeoutRetriesCounter;
                    MacCtx.McpsConfirm.Datarate = MacCtx.NvmCtx->MacParams.ChannelsDatarate;
                    UpLinkCounter++;
                }
            }
            else
            {
                RegionInitDefaults( MacCtx.NvmCtx->Region, INIT_TYPE_RESTORE );

                MacCtx.MacState &= ~LORAMAC_TX_RUNNING;

                MacCommandsBufferIndex = 0;
                MacCtx.NvmCtx->NodeAckRequested = false;
                MacCtx.McpsConfirm.AckReceived = false;
                MacCtx.McpsConfirm.NbRetries = MacCtx.NvmCtx->AckTimeoutRetriesCounter;
                UpLinkCounter++;
            }
        }
    }

    // Handle events
    if( MacCtx.MacState == LORAMAC_IDLE )
    {
        if( MacCtx.MacFlags.Bits.McpsReq == 1 )
        {
            MacCtx.MacFlags.Bits.McpsReq = 0;
            MacCtx.MacPrimitives->MacMcpsConfirm( &MacCtx.McpsConfirm );
        }

        if( MacCtx.MacFlags.Bits.MlmeReq == 1 )
        {
            MacCtx.MacFlags.Bits.MlmeReq = 0;
            LoRaMacConfirmQueueHandleCb( &MacCtx.MlmeConfirm );
            if( LoRaMacConfirmQueueGetCnt( ) > 0 )
            {
                MacCtx.MacFlags.Bits.MlmeReq = 1;
            }
        }

        // Handle MLME indication
        if( MacCtx.MacFlags.Bits.MlmeInd == 1 )
        {
            MacCtx.MacPrimitives->MacMlmeIndication( &MacCtx.MlmeIndication );
            MacCtx.MacFlags.Bits.MlmeInd = 0;
        }

        // Verify if sticky MAC commands are pending or not
        if( IsStickyMacCommandPending( ) == true )
        {// Setup MLME indication
            SetMlmeScheduleUplinkIndication( );
            MacCtx.MacPrimitives->MacMlmeIndication( &MacCtx.MlmeIndication );
            MacCtx.MacFlags.Bits.MlmeInd = 0;
        }

        // Procedure done. Reset variables.
        MacCtx.MacFlags.Bits.MacDone = 0;

        LoRaMacClassBResumeBeaconing( );
    }

    // Handle MCPS indication
    if( MacCtx.MacFlags.Bits.McpsInd == 1 )
    {
        MacCtx.MacFlags.Bits.McpsInd = 0;
        if( MacCtx.NvmCtx->DeviceClass == CLASS_C )
        {// Activate RX2 window for Class C
            OpenContinuousRx2Window( );
        }
        if( MacCtx.MacFlags.Bits.McpsIndSkip == 0 )
        {
            MacCtx.MacPrimitives->MacMcpsIndication( &MacCtx.McpsIndication );
        }
        MacCtx.MacFlags.Bits.McpsIndSkip = 0;
    }

}

static void OnTxDelayedTimerEvent( void )
{
    TimerStop( &MacCtx.TxDelayedTimer );
    MacCtx.MacState &= ~LORAMAC_TX_DELAYED;

    // Schedule frame, allow delayed frame transmissions
    ScheduleTx( true );
}

static void OnRxWindow1TimerEvent( void )
{
    TimerStop( &MacCtx.RxWindowTimer1 );
    MacCtx.RxSlot = RX_SLOT_WIN_1;

    MacCtx.RxWindow1Config.Channel = MacCtx.NvmCtx->Channel;
    MacCtx.RxWindow1Config.DrOffset = MacCtx.NvmCtx->MacParams.Rx1DrOffset;
    MacCtx.RxWindow1Config.DownlinkDwellTime = MacCtx.NvmCtx->MacParams.DownlinkDwellTime;
    MacCtx.RxWindow1Config.RepeaterSupport = MacCtx.NvmCtx->RepeaterSupport;
    MacCtx.RxWindow1Config.RxContinuous = false;
    MacCtx.RxWindow1Config.RxSlot = MacCtx.RxSlot;

    if( MacCtx.NvmCtx->DeviceClass == CLASS_C )
    {
        Radio.Standby( );
    }

    RegionRxConfig( MacCtx.NvmCtx->Region, &MacCtx.RxWindow1Config, ( int8_t* )&MacCtx.McpsIndication.RxDatarate );
    RxWindowSetup( MacCtx.RxWindow1Config.RxContinuous, MacCtx.NvmCtx->MacParams.MaxRxWindow );
}

static void OnRxWindow2TimerEvent( void )
{
    TimerStop( &MacCtx.RxWindowTimer2 );

    MacCtx.RxWindow2Config.Channel = MacCtx.NvmCtx->Channel;
    MacCtx.RxWindow2Config.Frequency = MacCtx.NvmCtx->MacParams.Rx2Channel.Frequency;
    MacCtx.RxWindow2Config.DownlinkDwellTime = MacCtx.NvmCtx->MacParams.DownlinkDwellTime;
    MacCtx.RxWindow2Config.RepeaterSupport = MacCtx.NvmCtx->RepeaterSupport;
    MacCtx.RxWindow2Config.RxSlot = RX_SLOT_WIN_2;

    if( MacCtx.NvmCtx->DeviceClass != CLASS_C )
    {
        MacCtx.RxWindow2Config.RxContinuous = false;
    }
    else
    {
        // Setup continuous listening for class c
        MacCtx.RxWindow2Config.RxContinuous = true;
    }

    if( RegionRxConfig( MacCtx.NvmCtx->Region, &MacCtx.RxWindow2Config, ( int8_t* )&MacCtx.McpsIndication.RxDatarate ) == true )
    {
        RxWindowSetup( MacCtx.RxWindow2Config.RxContinuous, MacCtx.NvmCtx->MacParams.MaxRxWindow );
        MacCtx.RxSlot = RX_SLOT_WIN_2;
    }
}

static void CheckToDisableAckTimeout( bool nodeAckRequested, DeviceClass_t devClass, bool ackReceived,
                                      uint8_t ackTimeoutRetriesCounter, uint8_t ackTimeoutRetries )
{
    // There are three cases where we need to stop the AckTimeoutTimer:
    if( nodeAckRequested == false )
    {
        if( devClass == CLASS_C )
        {// FIRST CASE
            // We have performed an unconfirmed uplink in class c mode
            // and have received a downlink in RX1 or RX2.
            TimerStop( &MacCtx.AckTimeoutTimer );
        }
    }
    else
    {
        if( ackReceived == 1 )
        {// SECOND CASE
            // We have performed a confirmed uplink and have received a
            // downlink with a valid ACK.
            TimerStop( &MacCtx.AckTimeoutTimer );
        }
        else
        {// THIRD CASE
            if( ackTimeoutRetriesCounter > ackTimeoutRetries )
            {
                // We have performed a confirmed uplink and have not
                // received a downlink with a valid ACK. In this case
                // we need to verify if the maximum retries have been
                // elapsed. If so, stop the timer.
                TimerStop( &MacCtx.AckTimeoutTimer );
            }
        }
    }
}

static void OnAckTimeoutTimerEvent( void )
{
    TimerStop( &MacCtx.AckTimeoutTimer );

    if( MacCtx.NvmCtx->NodeAckRequested == true )
    {
        MacCtx.NvmCtx->AckTimeoutRetry = true;
        MacCtx.MacState &= ~LORAMAC_ACK_REQ;
    }
    if( MacCtx.NvmCtx->DeviceClass == CLASS_C )
    {
        MacCtx.MacFlags.Bits.MacDone = 1;
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

    switch( MacCtx.NvmCtx->DeviceClass )
    {
        case CLASS_A:
        {
            if( deviceClass == CLASS_B )
            {
                status = LoRaMacClassBSwitchClass( deviceClass );
                if( status == LORAMAC_STATUS_OK )
                {
                    MacCtx.NvmCtx->DeviceClass = deviceClass;
                }
            }

            if( deviceClass == CLASS_C )
            {
                MacCtx.NvmCtx->DeviceClass = deviceClass;

                // Set the NodeAckRequested indicator to default
                MacCtx.NvmCtx->NodeAckRequested = false;
                // Set the radio into sleep mode in case we are still in RX mode
                Radio.Sleep( );
                // Compute Rx2 windows parameters in case the RX2 datarate has changed
                RegionComputeRxWindowParameters( MacCtx.NvmCtx->Region,
                                                 MacCtx.NvmCtx->MacParams.Rx2Channel.Datarate,
                                                 MacCtx.NvmCtx->MacParams.MinRxSymbols,
                                                 MacCtx.NvmCtx->MacParams.SystemMaxRxError,
                                                 &MacCtx.RxWindow2Config );
                OpenContinuousRx2Window( );


                status = LORAMAC_STATUS_OK;
            }
            break;
        }
        case CLASS_B:
        {
            status = LoRaMacClassBSwitchClass( deviceClass );
            if( status == LORAMAC_STATUS_OK )
            {
                MacCtx.NvmCtx->DeviceClass = deviceClass;
            }
            break;
        }
        case CLASS_C:
        {
            if( deviceClass == CLASS_A )
            {
                MacCtx.NvmCtx->DeviceClass = deviceClass;

                // Set the radio into sleep to setup a defined state
                Radio.Sleep( );

                status = LORAMAC_STATUS_OK;
            }
            break;
        }
    }

    return status;
}

static bool ValidatePayloadLength( uint8_t lenN, int8_t datarate, uint8_t fOptsLen )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    uint16_t maxN = 0;
    uint16_t payloadSize = 0;

    // Setup PHY request
    getPhy.UplinkDwellTime = MacCtx.NvmCtx->MacParams.UplinkDwellTime;
    getPhy.Datarate = datarate;
    getPhy.Attribute = PHY_MAX_PAYLOAD;

    // Get the maximum payload length
    if( MacCtx.NvmCtx->RepeaterSupport == true )
    {
        getPhy.Attribute = PHY_MAX_PAYLOAD_REPEATER;
    }
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
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

static bool IsStickyMacCommandPending( void )
{
    if( MacCommandsBufferToRepeatIndex > 0 )
    {
        // Sticky MAC commands pending
        return true;
    }
    return false;
}

static void SetMlmeScheduleUplinkIndication( void )
{
    MacCtx.MlmeIndication.MlmeIndication = MLME_SCHEDULE_UPLINK;
    MacCtx.MacFlags.Bits.MlmeInd = 1;
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
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this command
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_LINK_ADR_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Margin
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DUTY_CYCLE_ANS:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_RX_PARAM_SETUP_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate ACK, Channel ACK
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                // This is a sticky MAC command answer. Setup indication
                SetMlmeScheduleUplinkIndication( );
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DEV_STATUS_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 2 ) )
            {
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // 1st byte Battery
                // 2nd byte Margin
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = p2;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_NEW_CHANNEL_ANS:
            if( MacCommandsBufferIndex < ( bufLen - 1 ) )
            {
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate range OK, Channel frequency OK
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_RX_TIMING_SETUP_ANS:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                // This is a sticky MAC command answer. Setup indication
                SetMlmeScheduleUplinkIndication( );
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_TX_PARAM_SETUP_ANS:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DL_CHANNEL_ANS:
            if( MacCommandsBufferIndex < bufLen )
            {
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Uplink frequency exists, Channel frequency OK
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = p1;

                // This is a sticky MAC command answer. Setup indication
                SetMlmeScheduleUplinkIndication( );

                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_DEVICE_TIME_REQ:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
               status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_PING_SLOT_INFO_REQ:
            if( MacCommandsBufferIndex < ( LORA_MAC_COMMAND_MAX_LENGTH - 1 ) )
            {
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Periodicity and Datarate
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_PING_SLOT_FREQ_ANS:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Datarate range OK, Channel frequency OK
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_BEACON_TIMING_REQ:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // No payload for this answer
                status = LORAMAC_STATUS_OK;
            }
            break;
        case MOTE_MAC_BEACON_FREQ_ANS:
            if( MacCommandsBufferIndex < LORA_MAC_COMMAND_MAX_LENGTH )
            {
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = cmd;
                // Status: Channel frequency OK
                MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex++] = p1;
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

static void ProcessMacCommands( uint8_t *payload, uint8_t macIndex, uint8_t commandsSize, uint8_t snr, LoRaMacRxSlot_t rxSlot )
{
    uint8_t status = 0;

    while( macIndex < commandsSize )
    {
        // Decode Frame MAC commands
        switch( payload[macIndex++] )
        {
            case SRV_MAC_LINK_CHECK_ANS:
            {
                if( LoRaMacConfirmQueueIsCmdActive( MLME_LINK_CHECK ) == true )
                {
                    LoRaMacConfirmQueueSetStatus( LORAMAC_EVENT_INFO_STATUS_OK, MLME_LINK_CHECK );
                    MacCtx.MlmeConfirm.DemodMargin = payload[macIndex++];
                    MacCtx.MlmeConfirm.NbGateways = payload[macIndex++];
                }
                break;
            }
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
                linkAdrReq.AdrEnabled = MacCtx.NvmCtx->AdrCtrlOn;
                linkAdrReq.UplinkDwellTime = MacCtx.NvmCtx->MacParams.UplinkDwellTime;
                linkAdrReq.CurrentDatarate = MacCtx.NvmCtx->MacParams.ChannelsDatarate;
                linkAdrReq.CurrentTxPower = MacCtx.NvmCtx->MacParams.ChannelsTxPower;
                linkAdrReq.CurrentNbRep = MacCtx.NvmCtx->MacParams.ChannelsNbTrans;

                // Process the ADR requests
                status = RegionLinkAdrReq( MacCtx.NvmCtx->Region, &linkAdrReq, &linkAdrDatarate,
                                           &linkAdrTxPower, &linkAdrNbRep, &linkAdrNbBytesParsed );

                if( ( status & 0x07 ) == 0x07 )
                {
                    MacCtx.NvmCtx->MacParams.ChannelsDatarate = linkAdrDatarate;
                    MacCtx.NvmCtx->MacParams.ChannelsTxPower = linkAdrTxPower;
                    MacCtx.NvmCtx->MacParams.ChannelsNbTrans = linkAdrNbRep;
                }

                // Add the answers to the buffer
                for( uint8_t i = 0; i < ( linkAdrNbBytesParsed / 5 ); i++ )
                {
                    AddMacCommand( MOTE_MAC_LINK_ADR_ANS, status, 0 );
                }
                // Update MAC index
                macIndex += linkAdrNbBytesParsed - 1;
                break;
            }
            case SRV_MAC_DUTY_CYCLE_REQ:
            {
                MacCtx.NvmCtx->MaxDCycle = payload[macIndex++];
                MacCtx.NvmCtx->AggregatedDCycle = 1 << MacCtx.NvmCtx->MaxDCycle;
                AddMacCommand( MOTE_MAC_DUTY_CYCLE_ANS, 0, 0 );
                break;
            }
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
                status = RegionRxParamSetupReq( MacCtx.NvmCtx->Region, &rxParamSetupReq );

                if( ( status & 0x07 ) == 0x07 )
                {
                    MacCtx.NvmCtx->MacParams.Rx2Channel.Datarate = rxParamSetupReq.Datarate;
                    MacCtx.NvmCtx->MacParams.Rx2Channel.Frequency = rxParamSetupReq.Frequency;
                    MacCtx.NvmCtx->MacParams.Rx1DrOffset = rxParamSetupReq.DrOffset;
                }
                AddMacCommand( MOTE_MAC_RX_PARAM_SETUP_ANS, status, 0 );
                break;
            }
            case SRV_MAC_DEV_STATUS_REQ:
            {
                uint8_t batteryLevel = BAT_LEVEL_NO_MEASURE;
                if( ( MacCtx.MacCallbacks != NULL ) && ( MacCtx.MacCallbacks->GetBatteryLevel != NULL ) )
                {
                    batteryLevel = MacCtx.MacCallbacks->GetBatteryLevel( );
                }
                AddMacCommand( MOTE_MAC_DEV_STATUS_ANS, batteryLevel, snr & 0x3F);
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

                status = RegionNewChannelReq( MacCtx.NvmCtx->Region, &newChannelReq );

                AddMacCommand( MOTE_MAC_NEW_CHANNEL_ANS, status, 0 );
                break;
            }
            case SRV_MAC_RX_TIMING_SETUP_REQ:
            {
                uint8_t delay = payload[macIndex++] & 0x0F;

                if( delay == 0 )
                {
                    delay++;
                }
                MacCtx.NvmCtx->MacParams.ReceiveDelay1 = delay * 1000;
                MacCtx.NvmCtx->MacParams.ReceiveDelay2 = MacCtx.NvmCtx->MacParams.ReceiveDelay1 + 1000;
                AddMacCommand( MOTE_MAC_RX_TIMING_SETUP_ANS, 0, 0 );
                break;
            }
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
                if( RegionTxParamSetupReq( MacCtx.NvmCtx->Region, &txParamSetupReq ) != -1 )
                {
                    // Accept command
                    MacCtx.NvmCtx->MacParams.UplinkDwellTime = txParamSetupReq.UplinkDwellTime;
                    MacCtx.NvmCtx->MacParams.DownlinkDwellTime = txParamSetupReq.DownlinkDwellTime;
                    MacCtx.NvmCtx->MacParams.MaxEirp = LoRaMacMaxEirpTable[txParamSetupReq.MaxEirp];
                    // Add command response
                    AddMacCommand( MOTE_MAC_TX_PARAM_SETUP_ANS, 0, 0 );
                }
                break;
            }
            case SRV_MAC_DL_CHANNEL_REQ:
            {
                DlChannelReqParams_t dlChannelReq;
                status = 0x03;

                dlChannelReq.ChannelId = payload[macIndex++];
                dlChannelReq.Rx1Frequency = ( uint32_t )payload[macIndex++];
                dlChannelReq.Rx1Frequency |= ( uint32_t )payload[macIndex++] << 8;
                dlChannelReq.Rx1Frequency |= ( uint32_t )payload[macIndex++] << 16;
                dlChannelReq.Rx1Frequency *= 100;

                status = RegionDlChannelReq( MacCtx.NvmCtx->Region, &dlChannelReq );

                AddMacCommand( MOTE_MAC_DL_CHANNEL_ANS, status, 0 );
                break;
            }
            case SRV_MAC_DEVICE_TIME_ANS:
            {
                SysTime_t gpsEpochTime = { 0 };
                SysTime_t sysTime = { 0 };
                SysTime_t sysTimeCurrent = { 0 };

                gpsEpochTime.Seconds = ( uint32_t )payload[macIndex++];
                gpsEpochTime.Seconds |= ( uint32_t )payload[macIndex++] << 8;
                gpsEpochTime.Seconds |= ( uint32_t )payload[macIndex++] << 16;
                gpsEpochTime.Seconds |= ( uint32_t )payload[macIndex++] << 24;
                gpsEpochTime.SubSeconds = payload[macIndex++];

                // Convert the fractional second received in ms
                // round( pow( 0.5, 8.0 ) * 1000 ) = 3.90625
                gpsEpochTime.SubSeconds = ( int16_t )( ( ( int32_t )gpsEpochTime.SubSeconds * 1000 ) >> 8 );

                // Copy received GPS Epoch time into system time
                sysTime = gpsEpochTime;
                // Add Unix to Gps epcoh offset. The system time is based on Unix time.
                sysTime.Seconds += UNIX_GPS_EPOCH_OFFSET;

                // Compensate time difference between Tx Done time and now
                sysTimeCurrent = SysTimeGet( );
                sysTime = SysTimeAdd( sysTimeCurrent, SysTimeSub( sysTime, MacCtx.LastTxSysTime ) );

                // Apply the new system time.
                SysTimeSet( sysTime );
                LoRaMacClassBDeviceTimeAns( );
                break;
            }
            case SRV_MAC_PING_SLOT_INFO_ANS:
            {
                // According to the specification, it is not allowed to process this answer in
                // a ping or multicast slot
                if( ( rxSlot != RX_SLOT_WIN_PING_SLOT ) && ( rxSlot != RX_SLOT_WIN_MULTICAST_SLOT ) )
                {
                    LoRaMacClassBPingSlotInfoAns( );
                }
                break;
            }
            case SRV_MAC_PING_SLOT_CHANNEL_REQ:
            {
                uint8_t status = 0x03;
                uint32_t frequency = 0;
                uint8_t datarate;

                frequency = ( uint32_t )payload[macIndex++];
                frequency |= ( uint32_t )payload[macIndex++] << 8;
                frequency |= ( uint32_t )payload[macIndex++] << 16;
                frequency *= 100;
                datarate = payload[macIndex++] & 0x0F;

                status = LoRaMacClassBPingSlotChannelReq( datarate, frequency );
                AddMacCommand( MOTE_MAC_PING_SLOT_FREQ_ANS, status, 0 );
                break;
            }
            case SRV_MAC_BEACON_TIMING_ANS:
            {
                uint16_t beaconTimingDelay = 0;
                uint8_t beaconTimingChannel = 0;

                beaconTimingDelay = ( uint16_t )payload[macIndex++];
                beaconTimingDelay |= ( uint16_t )payload[macIndex++] << 8;
                beaconTimingChannel = payload[macIndex++];

                LoRaMacClassBBeaconTimingAns( beaconTimingDelay, beaconTimingChannel, RxDoneParams.LastRxDone );
                break;
            }
            case SRV_MAC_BEACON_FREQ_REQ:
                {
                    uint32_t frequency = 0;

                    frequency = ( uint32_t )payload[macIndex++];
                    frequency |= ( uint32_t )payload[macIndex++] << 8;
                    frequency |= ( uint32_t )payload[macIndex++] << 16;
                    frequency *= 100;

                    if( LoRaMacClassBBeaconFreqReq( frequency ) == true )
                    {
                        AddMacCommand( MOTE_MAC_BEACON_FREQ_ANS, 1, 0 );
                    }
                    else
                    {
                        AddMacCommand( MOTE_MAC_BEACON_FREQ_ANS, 0, 0 );
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
    if( MacCtx.NvmCtx->DeviceClass == CLASS_B )
    {
        fCtrl.Bits.FPending      = 1;
    }
    else
    {
        fCtrl.Bits.FPending      = 0;
    }
    fCtrl.Bits.Ack           = false;
    fCtrl.Bits.AdrAckReq     = false;
    fCtrl.Bits.Adr           = MacCtx.NvmCtx->AdrCtrlOn;

    // Prepare the frame
    status = PrepareFrame( macHdr, &fCtrl, fPort, fBuffer, fBufferSize );

    // Validate status
    if( status != LORAMAC_STATUS_OK )
    {
        return status;
    }

    // Reset confirm parameters
    MacCtx.McpsConfirm.NbRetries = 0;
    MacCtx.McpsConfirm.AckReceived = false;
    MacCtx.McpsConfirm.UpLinkCounter = UpLinkCounter;

    // Schedule frame, do not allow delayed transmissions
    status = ScheduleTx( false );

    return status;
}

static LoRaMacStatus_t ScheduleTx( bool allowDelayedTx )
{
    LoRaMacStatus_t status = LORAMAC_STATUS_PARAMETER_INVALID;
    TimerTime_t dutyCycleTimeOff = 0;
    NextChanParams_t nextChan;

    // Check if the device is off
    if( MacCtx.NvmCtx->MaxDCycle == 255 )
    {
        return LORAMAC_STATUS_DEVICE_OFF;
    }
    if( MacCtx.NvmCtx->MaxDCycle == 0 )
    {
        MacCtx.AggregatedTimeOff = 0;
    }

    // Update Backoff
    CalculateBackOff( MacCtx.NvmCtx->LastTxChannel );

    nextChan.AggrTimeOff = MacCtx.AggregatedTimeOff;
    nextChan.Datarate = MacCtx.NvmCtx->MacParams.ChannelsDatarate;
    nextChan.DutyCycleEnabled = MacCtx.NvmCtx->DutyCycleOn;
    nextChan.Joined = MacCtx.NvmCtx->IsLoRaMacNetworkJoined;
    nextChan.LastAggrTx = MacCtx.AggregatedLastTxDoneTime;

    // Select channel
    status = RegionNextChannel( MacCtx.NvmCtx->Region, &nextChan, &MacCtx.NvmCtx->Channel, &dutyCycleTimeOff, &MacCtx.AggregatedTimeOff );

    if( status != LORAMAC_STATUS_OK )
    {
        if( ( status == LORAMAC_STATUS_DUTYCYCLE_RESTRICTED ) && 
            ( allowDelayedTx == true ) )
        {
            // Allow delayed transmissions. We have to allow it in case
            // the MAC must retransmit a frame with the frame repetitions
            if( dutyCycleTimeOff != 0 )
            {// Send later - prepare timer
                MacCtx.MacState |= LORAMAC_TX_DELAYED;
                TimerSetValue( &MacCtx.TxDelayedTimer, dutyCycleTimeOff );
                TimerStart( &MacCtx.TxDelayedTimer );
            }
            return LORAMAC_STATUS_OK;
        }
        else
        {// State where the MAC cannot send a frame
            return status;
        }
    }

    // Compute Rx1 windows parameters
    RegionComputeRxWindowParameters( MacCtx.NvmCtx->Region,
                                     RegionApplyDrOffset( MacCtx.NvmCtx->Region, MacCtx.NvmCtx->MacParams.DownlinkDwellTime, MacCtx.NvmCtx->MacParams.ChannelsDatarate, MacCtx.NvmCtx->MacParams.Rx1DrOffset ),
                                     MacCtx.NvmCtx->MacParams.MinRxSymbols,
                                     MacCtx.NvmCtx->MacParams.SystemMaxRxError,
                                     &MacCtx.RxWindow1Config );
    // Compute Rx2 windows parameters
    RegionComputeRxWindowParameters( MacCtx.NvmCtx->Region,
                                     MacCtx.NvmCtx->MacParams.Rx2Channel.Datarate,
                                     MacCtx.NvmCtx->MacParams.MinRxSymbols,
                                     MacCtx.NvmCtx->MacParams.SystemMaxRxError,
                                     &MacCtx.RxWindow2Config );

    if( MacCtx.NvmCtx->IsLoRaMacNetworkJoined == false )
    {
        MacCtx.RxWindow1Delay = MacCtx.NvmCtx->MacParams.JoinAcceptDelay1 + MacCtx.RxWindow1Config.WindowOffset;
        MacCtx.RxWindow2Delay = MacCtx.NvmCtx->MacParams.JoinAcceptDelay2 + MacCtx.RxWindow2Config.WindowOffset;
    }
    else
    {
        if( ValidatePayloadLength( MacCtx.AppDataSize, MacCtx.NvmCtx->MacParams.ChannelsDatarate, MacCommandsBufferIndex ) == false )
        {
            return LORAMAC_STATUS_LENGTH_ERROR;
        }
        MacCtx.RxWindow1Delay = MacCtx.NvmCtx->MacParams.ReceiveDelay1 + MacCtx.RxWindow1Config.WindowOffset;
        MacCtx.RxWindow2Delay = MacCtx.NvmCtx->MacParams.ReceiveDelay2 + MacCtx.RxWindow2Config.WindowOffset;
    }

    // Try to send now
    return SendFrameOnChannel( MacCtx.NvmCtx->Channel );
}

static void CalculateBackOff( uint8_t channel )
{
    CalcBackOffParams_t calcBackOff;

    calcBackOff.Joined = MacCtx.NvmCtx->IsLoRaMacNetworkJoined;
    calcBackOff.DutyCycleEnabled = MacCtx.NvmCtx->DutyCycleOn;
    calcBackOff.Channel = channel;
    calcBackOff.ElapsedTime = TimerGetElapsedTime( MacCtx.InitializationTime );
    calcBackOff.TxTimeOnAir = MacCtx.TxTimeOnAir;
    calcBackOff.LastTxIsJoinRequest = MacCtx.LastTxIsJoinRequest;

    // Update regional back-off
    RegionCalcBackOff( MacCtx.NvmCtx->Region, &calcBackOff );

    // Update aggregated time-off. This must be an assignment and no incremental
    // update as we do only calculate the time-off based on the last transmission
    MacCtx.AggregatedTimeOff = ( MacCtx.TxTimeOnAir * MacCtx.NvmCtx->AggregatedDCycle - MacCtx.TxTimeOnAir );
}

static void ResetMacParameters( void )
{
    MacCtx.NvmCtx->IsLoRaMacNetworkJoined = false;

    // Counters
    UpLinkCounter = 0;
    DownLinkCounter = 0;
    MacCtx.NvmCtx->AdrAckCounter = 0;

    MacCtx.NvmCtx->ChannelsNbTransCounter = 0;

    MacCtx.NvmCtx->AckTimeoutRetries = 1;
    MacCtx.NvmCtx->AckTimeoutRetriesCounter = 1;
    MacCtx.NvmCtx->AckTimeoutRetry = false;

    MacCtx.NvmCtx->MaxDCycle = 0;
    MacCtx.NvmCtx->AggregatedDCycle = 1;

    MacCommandsBufferIndex = 0;
    MacCommandsBufferToRepeatIndex = 0;

    MacCtx.NvmCtx->MacParams.ChannelsTxPower = MacCtx.NvmCtx->MacParamsDefaults.ChannelsTxPower;
    MacCtx.NvmCtx->MacParams.ChannelsDatarate = MacCtx.NvmCtx->MacParamsDefaults.ChannelsDatarate;
    MacCtx.NvmCtx->MacParams.Rx1DrOffset = MacCtx.NvmCtx->MacParamsDefaults.Rx1DrOffset;
    MacCtx.NvmCtx->MacParams.Rx2Channel = MacCtx.NvmCtx->MacParamsDefaults.Rx2Channel;
    MacCtx.NvmCtx->MacParams.UplinkDwellTime = MacCtx.NvmCtx->MacParamsDefaults.UplinkDwellTime;
    MacCtx.NvmCtx->MacParams.DownlinkDwellTime = MacCtx.NvmCtx->MacParamsDefaults.DownlinkDwellTime;
    MacCtx.NvmCtx->MacParams.MaxEirp = MacCtx.NvmCtx->MacParamsDefaults.MaxEirp;
    MacCtx.NvmCtx->MacParams.AntennaGain = MacCtx.NvmCtx->MacParamsDefaults.AntennaGain;

    // Reset to application defaults
    RegionInitDefaults( MacCtx.NvmCtx->Region, INIT_TYPE_APP_DEFAULTS );

    MacCtx.NvmCtx->NodeAckRequested = false;
    MacCtx.NvmCtx->SrvAckRequested = false;
    MacCommandsInNextTx = false;

    // Reset Multicast downlink counters
    MulticastParams_t *cur = MacCtx.NvmCtx->MulticastChannelList;
    while( cur != NULL )
    {
        cur->DownLinkCounter = 0;
        cur = cur->Next;
    }

    // Initialize channel index.
    MacCtx.NvmCtx->Channel = 0;
    MacCtx.NvmCtx->LastTxChannel = MacCtx.NvmCtx->Channel;
}

static bool IsFPortAllowed( uint8_t fPort )
{
    if( fPort > 224 )
    {
        return false;
    }
    return true;
}

static void OpenContinuousRx2Window( void )
{
    OnRxWindow2TimerEvent( );
    MacCtx.RxSlot = RX_SLOT_WIN_CLASS_C;
}

LoRaMacStatus_t PrepareFrame( LoRaMacHeader_t *macHdr, LoRaMacFrameCtrl_t *fCtrl, uint8_t fPort, void *fBuffer, uint16_t fBufferSize )
{
    MacCtx.PktBufferLen = 0;
    MacCtx.NvmCtx->NodeAckRequested = false;
    AdrNextParams_t adrNext;
    uint16_t i;
    uint8_t pktHeaderLen = 0;
    uint32_t mic = 0;
    const void* payload = fBuffer;
    uint8_t framePort = fPort;

    MacCtx.PktBufferLen = 0;

    MacCtx.NvmCtx->NodeAckRequested = false;

    if( fBuffer == NULL )
    {
        fBufferSize = 0;
    }

    MacCtx.AppDataSize = fBufferSize;

    MacCtx.PktBuffer[pktHeaderLen++] = macHdr->Value;

    switch( macHdr->Bits.MType )
    {
        case FRAME_TYPE_JOIN_REQ:
            MacCtx.PktBufferLen = pktHeaderLen;

            memcpyr( MacCtx.PktBuffer + MacCtx.PktBufferLen, MacCtx.AppEui, 8 );
            MacCtx.PktBufferLen += 8;
            memcpyr( MacCtx.PktBuffer + MacCtx.PktBufferLen, MacCtx.DevEui, 8 );
            MacCtx.PktBufferLen += 8;

            LoRaMacDevNonce = Radio.Random( );

            MacCtx.PktBuffer[MacCtx.PktBufferLen++] = LoRaMacDevNonce & 0xFF;
            MacCtx.PktBuffer[MacCtx.PktBufferLen++] = ( LoRaMacDevNonce >> 8 ) & 0xFF;

            LoRaMacJoinComputeMic( MacCtx.PktBuffer, MacCtx.PktBufferLen & 0xFF, LoRaMacAppKey, &mic );

            MacCtx.PktBuffer[MacCtx.PktBufferLen++] = mic & 0xFF;
            MacCtx.PktBuffer[MacCtx.PktBufferLen++] = ( mic >> 8 ) & 0xFF;
            MacCtx.PktBuffer[MacCtx.PktBufferLen++] = ( mic >> 16 ) & 0xFF;
            MacCtx.PktBuffer[MacCtx.PktBufferLen++] = ( mic >> 24 ) & 0xFF;

            break;
        case FRAME_TYPE_DATA_CONFIRMED_UP:
            MacCtx.NvmCtx->NodeAckRequested = true;
            //Intentional fallthrough
        case FRAME_TYPE_DATA_UNCONFIRMED_UP:
            if( MacCtx.NvmCtx->IsLoRaMacNetworkJoined == false )
            {
                return LORAMAC_STATUS_NO_NETWORK_JOINED; // No network has been joined yet
            }

            // Adr next request
            adrNext.UpdateChanMask = true;
            adrNext.AdrEnabled = fCtrl->Bits.Adr;
            adrNext.AdrAckCounter = MacCtx.NvmCtx->AdrAckCounter;
            adrNext.Datarate = MacCtx.NvmCtx->MacParams.ChannelsDatarate;
            adrNext.TxPower = MacCtx.NvmCtx->MacParams.ChannelsTxPower;
            adrNext.UplinkDwellTime = MacCtx.NvmCtx->MacParams.UplinkDwellTime;

            fCtrl->Bits.AdrAckReq = RegionAdrNext( MacCtx.NvmCtx->Region, &adrNext,
                                                   &MacCtx.NvmCtx->MacParams.ChannelsDatarate, &MacCtx.NvmCtx->MacParams.ChannelsTxPower, &MacCtx.NvmCtx->AdrAckCounter );

            if( MacCtx.NvmCtx->SrvAckRequested == true )
            {
                MacCtx.NvmCtx->SrvAckRequested = false;
                fCtrl->Bits.Ack = 1;
            }

            MacCtx.PktBuffer[pktHeaderLen++] = ( MacCtx.NvmCtx->DevAddr ) & 0xFF;
            MacCtx.PktBuffer[pktHeaderLen++] = ( MacCtx.NvmCtx->DevAddr >> 8 ) & 0xFF;
            MacCtx.PktBuffer[pktHeaderLen++] = ( MacCtx.NvmCtx->DevAddr >> 16 ) & 0xFF;
            MacCtx.PktBuffer[pktHeaderLen++] = ( MacCtx.NvmCtx->DevAddr >> 24 ) & 0xFF;

            MacCtx.PktBuffer[pktHeaderLen++] = fCtrl->Value;

            MacCtx.PktBuffer[pktHeaderLen++] = UpLinkCounter & 0xFF;
            MacCtx.PktBuffer[pktHeaderLen++] = ( UpLinkCounter >> 8 ) & 0xFF;

            // Copy the MAC commands which must be re-send into the MAC command buffer
            memcpy1( &MacCtx.NvmCtx->MacCommandsBuffer[MacCommandsBufferIndex], MacCtx.NvmCtx->MacCommandsBufferToRepeat, MacCommandsBufferToRepeatIndex );
            MacCommandsBufferIndex += MacCommandsBufferToRepeatIndex;

            if( ( payload != NULL ) && ( MacCtx.AppDataSize > 0 ) )
            {
                if( MacCommandsInNextTx == true )
                {
                    if( MacCommandsBufferIndex <= LORA_MAC_COMMAND_MAX_FOPTS_LENGTH )
                    {
                        fCtrl->Bits.FOptsLen += MacCommandsBufferIndex;

                        // Update FCtrl field with new value of OptionsLength
                        MacCtx.PktBuffer[0x05] = fCtrl->Value;
                        for( i = 0; i < MacCommandsBufferIndex; i++ )
                        {
                            MacCtx.PktBuffer[pktHeaderLen++] = MacCtx.NvmCtx->MacCommandsBuffer[i];
                        }
                    }
                    else
                    {
                        MacCtx.AppDataSize = MacCommandsBufferIndex;
                        payload = MacCtx.NvmCtx->MacCommandsBuffer;
                        framePort = 0;
                    }
                }
            }
            else
            {
                if( ( MacCommandsBufferIndex > 0 ) && ( MacCommandsInNextTx == true ) )
                {
                    MacCtx.AppDataSize = MacCommandsBufferIndex;
                    payload = MacCtx.NvmCtx->MacCommandsBuffer;
                    framePort = 0;
                }
            }
            MacCommandsInNextTx = false;
            // Store MAC commands which must be re-send in case the device does not receive a downlink anymore
            MacCommandsBufferToRepeatIndex = ParseMacCommandsToRepeat( MacCtx.NvmCtx->MacCommandsBuffer, MacCommandsBufferIndex, MacCtx.NvmCtx->MacCommandsBufferToRepeat );
            if( MacCommandsBufferToRepeatIndex > 0 )
            {
                MacCommandsInNextTx = true;
            }

            if( ( payload != NULL ) && ( MacCtx.AppDataSize > 0 ) )
            {
                MacCtx.PktBuffer[pktHeaderLen++] = framePort;

                if( framePort == 0 )
                {
                    // Reset buffer index as the mac commands are being sent on port 0
                    MacCommandsBufferIndex = 0;
                    LoRaMacPayloadEncrypt( (uint8_t* ) payload, MacCtx.AppDataSize, LoRaMacNwkSKey, MacCtx.NvmCtx->DevAddr, UP_LINK, UpLinkCounter, &MacCtx.PktBuffer[pktHeaderLen] );
                }
                else
                {
                    LoRaMacPayloadEncrypt( (uint8_t* ) payload, MacCtx.AppDataSize, LoRaMacAppSKey, MacCtx.NvmCtx->DevAddr, UP_LINK, UpLinkCounter, &MacCtx.PktBuffer[pktHeaderLen] );
                }
            }
            MacCtx.PktBufferLen = pktHeaderLen + MacCtx.AppDataSize;

            LoRaMacComputeMic( MacCtx.PktBuffer, MacCtx.PktBufferLen, LoRaMacNwkSKey, MacCtx.NvmCtx->DevAddr, UP_LINK, UpLinkCounter, &mic );

            MacCtx.PktBuffer[MacCtx.PktBufferLen + 0] = mic & 0xFF;
            MacCtx.PktBuffer[MacCtx.PktBufferLen + 1] = ( mic >> 8 ) & 0xFF;
            MacCtx.PktBuffer[MacCtx.PktBufferLen + 2] = ( mic >> 16 ) & 0xFF;
            MacCtx.PktBuffer[MacCtx.PktBufferLen + 3] = ( mic >> 24 ) & 0xFF;

            MacCtx.PktBufferLen += LORAMAC_MFR_LEN;

            break;
        case FRAME_TYPE_PROPRIETARY:
            if( ( fBuffer != NULL ) && ( MacCtx.AppDataSize > 0 ) )
            {
                memcpy1( MacCtx.PktBuffer + pktHeaderLen, ( uint8_t* ) fBuffer, MacCtx.AppDataSize );
                MacCtx.PktBufferLen = pktHeaderLen + MacCtx.AppDataSize;
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
    txConfig.Datarate = MacCtx.NvmCtx->MacParams.ChannelsDatarate;
    txConfig.TxPower = MacCtx.NvmCtx->MacParams.ChannelsTxPower;
    txConfig.MaxEirp = MacCtx.NvmCtx->MacParams.MaxEirp;
    txConfig.AntennaGain = MacCtx.NvmCtx->MacParams.AntennaGain;
    txConfig.PktLen = MacCtx.PktBufferLen;


    if( LoRaMacClassBIsBeaconExpected( ) == true )
    {
        return LORAMAC_STATUS_BUSY_BEACON_RESERVED_TIME;
    }

    if( MacCtx.NvmCtx->DeviceClass == CLASS_B )
    {
        if( LoRaMacClassBIsPingExpected( ) == true )
        {
            return LORAMAC_STATUS_BUSY_PING_SLOT_WINDOW_TIME;
        }
        else if( LoRaMacClassBIsMulticastExpected( ) == true )
        {
            return LORAMAC_STATUS_BUSY_PING_SLOT_WINDOW_TIME;
        }
        else
        {
            LoRaMacClassBStopRxSlots( );
        }
    }
    RegionTxConfig( MacCtx.NvmCtx->Region, &txConfig, &txPower, &MacCtx.TxTimeOnAir );

    LoRaMacConfirmQueueSetStatusCmn( LORAMAC_EVENT_INFO_STATUS_ERROR );
    MacCtx.McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
    MacCtx.McpsConfirm.Datarate = MacCtx.NvmCtx->MacParams.ChannelsDatarate;
    MacCtx.McpsConfirm.TxPower = txPower;
    MacCtx.McpsConfirm.Channel = channel;

    // Store the time on air
    MacCtx.McpsConfirm.TxTimeOnAir = MacCtx.TxTimeOnAir;
    MacCtx.MlmeConfirm.TxTimeOnAir = MacCtx.TxTimeOnAir;

    if( LoRaMacClassBIsBeaconModeActive( ) == true )
    {
        // Currently, the Time-On-Air can only be computed when the radion is configured with
        // the TX configuration
        TimerTime_t collisionTime = LoRaMacClassBIsUplinkCollision( MacCtx.TxTimeOnAir );

        if( collisionTime > 0 )
        {
            return LORAMAC_STATUS_BUSY_UPLINK_COLLISION;
        }
    }

    LoRaMacClassBHaltBeaconing( );

    MacCtx.MacState |= LORAMAC_TX_RUNNING;

    // Send now
    Radio.Send( MacCtx.PktBuffer, MacCtx.PktBufferLen );

    MacCtx.MacState |= LORAMAC_TX_RUNNING;

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t SetTxContinuousWave( uint16_t timeout )
{
    ContinuousWaveParams_t continuousWave;

    continuousWave.Channel = MacCtx.NvmCtx->Channel;
    continuousWave.Datarate = MacCtx.NvmCtx->MacParams.ChannelsDatarate;
    continuousWave.TxPower = MacCtx.NvmCtx->MacParams.ChannelsTxPower;
    continuousWave.MaxEirp = MacCtx.NvmCtx->MacParams.MaxEirp;
    continuousWave.AntennaGain = MacCtx.NvmCtx->MacParams.AntennaGain;
    continuousWave.Timeout = timeout;

    RegionSetContinuousWave( MacCtx.NvmCtx->Region, &continuousWave );

    MacCtx.MacState |= LORAMAC_TX_RUNNING;

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t SetTxContinuousWave1( uint16_t timeout, uint32_t frequency, uint8_t power )
{
    Radio.SetTxContinuousWave( frequency, power, timeout );



    MacCtx.MacState |= LORAMAC_TX_RUNNING;

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacInitialization( LoRaMacPrimitives_t *primitives, LoRaMacCallback_t *callbacks, LoRaMacRegion_t region )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    LoRaMacClassBCallback_t classBCallbacks;
    LoRaMacClassBParams_t classBParams;

    if( ( primitives == NULL ) ||
        ( callbacks == NULL ) )
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

    // Confirm queue reset
    LoRaMacConfirmQueueInit( primitives );

    // Initialize the module context with zeros
    memset1( ( uint8_t* ) &NvmMacCtx, 0x00, sizeof( LoRaMacNvmCtx_t ) );
    memset1( ( uint8_t* ) &MacCtx, 0x00, sizeof( LoRaMacCtx_t ) );
    MacCtx.NvmCtx = &NvmMacCtx;

    // Set non zero variables to its default value
    MacCtx.NvmCtx->AckTimeoutRetriesCounter = 1;
    MacCtx.NvmCtx->AckTimeoutRetries = 1;
    MacCtx.NvmCtx->Region = region;
    MacCtx.NvmCtx->DeviceClass = CLASS_A;
    MacCtx.NvmCtx->RepeaterSupport = false;

    // Reset to defaults
    getPhy.Attribute = PHY_DUTY_CYCLE;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->DutyCycleOn = ( bool ) phyParam.Value;

    getPhy.Attribute = PHY_DEF_TX_POWER;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.ChannelsTxPower = phyParam.Value;

    getPhy.Attribute = PHY_DEF_TX_DR;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.ChannelsDatarate = phyParam.Value;

    getPhy.Attribute = PHY_MAX_RX_WINDOW;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.MaxRxWindow = phyParam.Value;

    getPhy.Attribute = PHY_RECEIVE_DELAY1;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.ReceiveDelay1 = phyParam.Value;

    getPhy.Attribute = PHY_RECEIVE_DELAY2;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.ReceiveDelay2 = phyParam.Value;

    getPhy.Attribute = PHY_JOIN_ACCEPT_DELAY1;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.JoinAcceptDelay1 = phyParam.Value;

    getPhy.Attribute = PHY_JOIN_ACCEPT_DELAY2;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.JoinAcceptDelay2 = phyParam.Value;

    getPhy.Attribute = PHY_DEF_DR1_OFFSET;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.Rx1DrOffset = phyParam.Value;

    getPhy.Attribute = PHY_DEF_RX2_FREQUENCY;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.Rx2Channel.Frequency = phyParam.Value;

    getPhy.Attribute = PHY_DEF_RX2_DR;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.Rx2Channel.Datarate = phyParam.Value;

    getPhy.Attribute = PHY_DEF_UPLINK_DWELL_TIME;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.UplinkDwellTime = phyParam.Value;

    getPhy.Attribute = PHY_DEF_DOWNLINK_DWELL_TIME;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.DownlinkDwellTime = phyParam.Value;

    getPhy.Attribute = PHY_DEF_MAX_EIRP;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.MaxEirp = phyParam.fValue;

    getPhy.Attribute = PHY_DEF_ANTENNA_GAIN;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    MacCtx.NvmCtx->MacParamsDefaults.AntennaGain = phyParam.fValue;

    // Init parameters which are not set in function ResetMacParameters
    MacCtx.NvmCtx->MacParamsDefaults.ChannelsNbTrans = 1;
    MacCtx.NvmCtx->MacParamsDefaults.SystemMaxRxError = 10;
    MacCtx.NvmCtx->MacParamsDefaults.MinRxSymbols = 6;

    MacCtx.NvmCtx->MacParams.SystemMaxRxError = MacCtx.NvmCtx->MacParamsDefaults.SystemMaxRxError;
    MacCtx.NvmCtx->MacParams.MinRxSymbols = MacCtx.NvmCtx->MacParamsDefaults.MinRxSymbols;
    MacCtx.NvmCtx->MacParams.MaxRxWindow = MacCtx.NvmCtx->MacParamsDefaults.MaxRxWindow;
    MacCtx.NvmCtx->MacParams.ReceiveDelay1 = MacCtx.NvmCtx->MacParamsDefaults.ReceiveDelay1;
    MacCtx.NvmCtx->MacParams.ReceiveDelay2 = MacCtx.NvmCtx->MacParamsDefaults.ReceiveDelay2;
    MacCtx.NvmCtx->MacParams.JoinAcceptDelay1 = MacCtx.NvmCtx->MacParamsDefaults.JoinAcceptDelay1;
    MacCtx.NvmCtx->MacParams.JoinAcceptDelay2 = MacCtx.NvmCtx->MacParamsDefaults.JoinAcceptDelay2;
    MacCtx.NvmCtx->MacParams.ChannelsNbTrans = MacCtx.NvmCtx->MacParamsDefaults.ChannelsNbTrans;

    ResetMacParameters( );

    MacCtx.NvmCtx->PublicNetwork = true;

    MacCtx.MacPrimitives = primitives;
    MacCtx.MacCallbacks = callbacks;
    MacCtx.MacFlags.Value = 0;
    MacCtx.MacState = LORAMAC_IDLE;

    // Reset duty cycle times
    MacCtx.AggregatedLastTxDoneTime = 0;
    MacCtx.AggregatedTimeOff = 0;

    // Initialize timers
    TimerInit( &MacCtx.TxDelayedTimer, OnTxDelayedTimerEvent );
    TimerInit( &MacCtx.RxWindowTimer1, OnRxWindow1TimerEvent );
    TimerInit( &MacCtx.RxWindowTimer2, OnRxWindow2TimerEvent );
    TimerInit( &MacCtx.AckTimeoutTimer, OnAckTimeoutTimerEvent );

    // Store the current initialization time
    MacCtx.InitializationTime = TimerGetCurrentTime( );

    // Initialize Radio driver
    MacCtx.RadioEvents.TxDone = OnRadioTxDone;
    MacCtx.RadioEvents.RxDone = OnRadioRxDone;
    MacCtx.RadioEvents.RxError = OnRadioRxError;
    MacCtx.RadioEvents.TxTimeout = OnRadioTxTimeout;
    MacCtx.RadioEvents.RxTimeout = OnRadioRxTimeout;
    Radio.Init( &MacCtx.RadioEvents );

    RegionInitDefaults( MacCtx.NvmCtx->Region, INIT_TYPE_INIT );

    // Random seed initialization
    srand1( Radio.Random( ) );

    Radio.SetPublicNetwork( MacCtx.NvmCtx->PublicNetwork );
    Radio.Sleep( );

    // Initialize class b
    // Apply callback
    classBCallbacks.GetTemperatureLevel = NULL;
    if( callbacks != NULL )
    {
        classBCallbacks.GetTemperatureLevel = callbacks->GetTemperatureLevel;
    }

    // Must all be static. Don't use local references.
    classBParams.MlmeIndication = &MacCtx.MlmeIndication;
    classBParams.McpsIndication = &MacCtx.McpsIndication;
    classBParams.MlmeConfirm = &MacCtx.MlmeConfirm;
    classBParams.LoRaMacFlags = &MacCtx.MacFlags;
    classBParams.LoRaMacDevAddr = &MacCtx.NvmCtx->DevAddr;
    classBParams.LoRaMacRegion = &MacCtx.NvmCtx->Region;
    classBParams.LoRaMacParams = &MacCtx.NvmCtx->MacParams;
    classBParams.MulticastChannels = &MacCtx.NvmCtx->MulticastChannelList;

    LoRaMacClassBInit( &classBParams, &classBCallbacks );

    return LORAMAC_STATUS_OK;
}

LoRaMacStatus_t LoRaMacQueryTxPossible( uint8_t size, LoRaMacTxInfo_t* txInfo )
{
    AdrNextParams_t adrNext;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    int8_t datarate = MacCtx.NvmCtx->MacParamsDefaults.ChannelsDatarate;
    int8_t txPower = MacCtx.NvmCtx->MacParamsDefaults.ChannelsTxPower;
    uint8_t fOptLen = MacCommandsBufferIndex + MacCommandsBufferToRepeatIndex;

    if( txInfo == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    // Setup ADR request
    adrNext.UpdateChanMask = false;
    adrNext.AdrEnabled = MacCtx.NvmCtx->AdrCtrlOn;
    adrNext.AdrAckCounter = MacCtx.NvmCtx->AdrAckCounter;
    adrNext.Datarate = MacCtx.NvmCtx->MacParams.ChannelsDatarate;
    adrNext.TxPower = MacCtx.NvmCtx->MacParams.ChannelsTxPower;
    adrNext.UplinkDwellTime = MacCtx.NvmCtx->MacParams.UplinkDwellTime;

    // We call the function for information purposes only. We don't want to
    // apply the datarate, the tx power and the ADR ack counter.
    RegionAdrNext( MacCtx.NvmCtx->Region, &adrNext, &datarate, &txPower, &MacCtx.NvmCtx->AdrAckCounter );

    // Setup PHY request
    getPhy.UplinkDwellTime = MacCtx.NvmCtx->MacParams.UplinkDwellTime;
    getPhy.Datarate = datarate;
    getPhy.Attribute = PHY_MAX_PAYLOAD;

    // Change request in case repeater is supported
    if( MacCtx.NvmCtx->RepeaterSupport == true )
    {
        getPhy.Attribute = PHY_MAX_PAYLOAD_REPEATER;
    }
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
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
            mibGet->Param.Class = MacCtx.NvmCtx->DeviceClass;
            break;
        }
        case MIB_NETWORK_JOINED:
        {
            mibGet->Param.IsNetworkJoined = MacCtx.NvmCtx->IsLoRaMacNetworkJoined;
            break;
        }
        case MIB_ADR:
        {
            mibGet->Param.AdrEnable = MacCtx.NvmCtx->AdrCtrlOn;
            break;
        }
        case MIB_NET_ID:
        {
            mibGet->Param.NetID = MacCtx.NvmCtx->NetID;
            break;
        }
        case MIB_DEV_ADDR:
        {
            mibGet->Param.DevAddr = MacCtx.NvmCtx->DevAddr;
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
            mibGet->Param.EnablePublicNetwork = MacCtx.NvmCtx->PublicNetwork;
            break;
        }
        case MIB_REPEATER_SUPPORT:
        {
            mibGet->Param.EnableRepeaterSupport = MacCtx.NvmCtx->RepeaterSupport;
            break;
        }
        case MIB_CHANNELS:
        {
            getPhy.Attribute = PHY_CHANNELS;
            phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );

            mibGet->Param.ChannelList = phyParam.Channels;
            break;
        }
        case MIB_RX2_CHANNEL:
        {
            mibGet->Param.Rx2Channel = MacCtx.NvmCtx->MacParams.Rx2Channel;
            break;
        }
        case MIB_RX2_DEFAULT_CHANNEL:
        {
            mibGet->Param.Rx2Channel = MacCtx.NvmCtx->MacParamsDefaults.Rx2Channel;
            break;
        }
        case MIB_CHANNELS_DEFAULT_MASK:
        {
            getPhy.Attribute = PHY_CHANNELS_DEFAULT_MASK;
            phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );

            mibGet->Param.ChannelsDefaultMask = phyParam.ChannelsMask;
            break;
        }
        case MIB_CHANNELS_MASK:
        {
            getPhy.Attribute = PHY_CHANNELS_MASK;
            phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );

            mibGet->Param.ChannelsMask = phyParam.ChannelsMask;
            break;
        }
        case MIB_CHANNELS_NB_TRANS:
        {
            mibGet->Param.ChannelsNbTrans = MacCtx.NvmCtx->MacParams.ChannelsNbTrans;
            break;
        }
        case MIB_MAX_RX_WINDOW_DURATION:
        {
            mibGet->Param.MaxRxWindow = MacCtx.NvmCtx->MacParams.MaxRxWindow;
            break;
        }
        case MIB_RECEIVE_DELAY_1:
        {
            mibGet->Param.ReceiveDelay1 = MacCtx.NvmCtx->MacParams.ReceiveDelay1;
            break;
        }
        case MIB_RECEIVE_DELAY_2:
        {
            mibGet->Param.ReceiveDelay2 = MacCtx.NvmCtx->MacParams.ReceiveDelay2;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_1:
        {
            mibGet->Param.JoinAcceptDelay1 = MacCtx.NvmCtx->MacParams.JoinAcceptDelay1;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_2:
        {
            mibGet->Param.JoinAcceptDelay2 = MacCtx.NvmCtx->MacParams.JoinAcceptDelay2;
            break;
        }
        case MIB_CHANNELS_DEFAULT_DATARATE:
        {
            mibGet->Param.ChannelsDefaultDatarate = MacCtx.NvmCtx->MacParamsDefaults.ChannelsDatarate;
            break;
        }
        case MIB_CHANNELS_DATARATE:
        {
            mibGet->Param.ChannelsDatarate = MacCtx.NvmCtx->MacParams.ChannelsDatarate;
            break;
        }
        case MIB_CHANNELS_DEFAULT_TX_POWER:
        {
            mibGet->Param.ChannelsDefaultTxPower = MacCtx.NvmCtx->MacParamsDefaults.ChannelsTxPower;
            break;
        }
        case MIB_CHANNELS_TX_POWER:
        {
            mibGet->Param.ChannelsTxPower = MacCtx.NvmCtx->MacParams.ChannelsTxPower;
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
            mibGet->Param.MulticastList = MacCtx.NvmCtx->MulticastChannelList;
            break;
        }
        case MIB_SYSTEM_MAX_RX_ERROR:
        {
            mibGet->Param.SystemMaxRxError = MacCtx.NvmCtx->MacParams.SystemMaxRxError;
            break;
        }
        case MIB_MIN_RX_SYMBOLS:
        {
            mibGet->Param.MinRxSymbols = MacCtx.NvmCtx->MacParams.MinRxSymbols;
            break;
        }
        case MIB_ANTENNA_GAIN:
        {
            mibGet->Param.AntennaGain = MacCtx.NvmCtx->MacParams.AntennaGain;
            break;
        }
        case MIB_DEFAULT_ANTENNA_GAIN:
        {
            mibGet->Param.DefaultAntennaGain = MacCtx.NvmCtx->MacParamsDefaults.AntennaGain;
            break;
        }
        default:
        {
            if( MacCtx.NvmCtx->DeviceClass == CLASS_B )
            {
                status = LoRaMacClassBMibGetRequestConfirm( mibGet );
            }
            else
            {
                status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            }
            break;
        }
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
    if( ( MacCtx.MacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
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
            MacCtx.NvmCtx->IsLoRaMacNetworkJoined = mibSet->Param.IsNetworkJoined;
            break;
        }
        case MIB_ADR:
        {
            MacCtx.NvmCtx->AdrCtrlOn = mibSet->Param.AdrEnable;
            break;
        }
        case MIB_NET_ID:
        {
            MacCtx.NvmCtx->NetID = mibSet->Param.NetID;
            break;
        }
        case MIB_DEV_ADDR:
        {
            MacCtx.NvmCtx->DevAddr = mibSet->Param.DevAddr;
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
            MacCtx.NvmCtx->PublicNetwork = mibSet->Param.EnablePublicNetwork;
            Radio.SetPublicNetwork( MacCtx.NvmCtx->PublicNetwork );
            break;
        }
        case MIB_REPEATER_SUPPORT:
        {
             MacCtx.NvmCtx->RepeaterSupport = mibSet->Param.EnableRepeaterSupport;
            break;
        }
        case MIB_RX2_CHANNEL:
        {
            verify.DatarateParams.Datarate = mibSet->Param.Rx2Channel.Datarate;
            verify.DatarateParams.DownlinkDwellTime = MacCtx.NvmCtx->MacParams.DownlinkDwellTime;

            if( RegionVerify( MacCtx.NvmCtx->Region, &verify, PHY_RX_DR ) == true )
            {
                MacCtx.NvmCtx->MacParams.Rx2Channel = mibSet->Param.Rx2Channel;

                if( ( MacCtx.NvmCtx->DeviceClass == CLASS_C ) && ( MacCtx.NvmCtx->IsLoRaMacNetworkJoined == true ) )
                {
                    // We can only compute the RX window parameters directly, if we are already
                    // in class c mode and joined. We cannot setup an RX window in case of any other
                    // class type.
                    // Set the radio into sleep mode in case we are still in RX mode
                    Radio.Sleep( );
                    // Compute Rx2 windows parameters
                    RegionComputeRxWindowParameters( MacCtx.NvmCtx->Region,
                                                     MacCtx.NvmCtx->MacParams.Rx2Channel.Datarate,
                                                     MacCtx.NvmCtx->MacParams.MinRxSymbols,
                                                     MacCtx.NvmCtx->MacParams.SystemMaxRxError,
                                                     &MacCtx.RxWindow2Config );
                    OpenContinuousRx2Window( );
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
            verify.DatarateParams.DownlinkDwellTime = MacCtx.NvmCtx->MacParams.DownlinkDwellTime;

            if( RegionVerify( MacCtx.NvmCtx->Region, &verify, PHY_RX_DR ) == true )
            {
                MacCtx.NvmCtx->MacParamsDefaults.Rx2Channel = mibSet->Param.Rx2DefaultChannel;
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

            if( RegionChanMaskSet( MacCtx.NvmCtx->Region, &chanMaskSet ) == false )
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_MASK:
        {
            chanMaskSet.ChannelsMaskIn = mibSet->Param.ChannelsMask;
            chanMaskSet.ChannelsMaskType = CHANNELS_MASK;

            if( RegionChanMaskSet( MacCtx.NvmCtx->Region, &chanMaskSet ) == false )
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_CHANNELS_NB_TRANS:
        {
            if( ( mibSet->Param.ChannelsNbTrans >= 1 ) &&
                ( mibSet->Param.ChannelsNbTrans <= 15 ) )
            {
                MacCtx.NvmCtx->MacParams.ChannelsNbTrans = mibSet->Param.ChannelsNbTrans;
            }
            else
            {
                status = LORAMAC_STATUS_PARAMETER_INVALID;
            }
            break;
        }
        case MIB_MAX_RX_WINDOW_DURATION:
        {
            MacCtx.NvmCtx->MacParams.MaxRxWindow = mibSet->Param.MaxRxWindow;
            break;
        }
        case MIB_RECEIVE_DELAY_1:
        {
            MacCtx.NvmCtx->MacParams.ReceiveDelay1 = mibSet->Param.ReceiveDelay1;
            break;
        }
        case MIB_RECEIVE_DELAY_2:
        {
            MacCtx.NvmCtx->MacParams.ReceiveDelay2 = mibSet->Param.ReceiveDelay2;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_1:
        {
            MacCtx.NvmCtx->MacParams.JoinAcceptDelay1 = mibSet->Param.JoinAcceptDelay1;
            break;
        }
        case MIB_JOIN_ACCEPT_DELAY_2:
        {
            MacCtx.NvmCtx->MacParams.JoinAcceptDelay2 = mibSet->Param.JoinAcceptDelay2;
            break;
        }
        case MIB_CHANNELS_DEFAULT_DATARATE:
        {
            verify.DatarateParams.Datarate = mibSet->Param.ChannelsDefaultDatarate;

            if( RegionVerify( MacCtx.NvmCtx->Region, &verify, PHY_DEF_TX_DR ) == true )
            {
                MacCtx.NvmCtx->MacParamsDefaults.ChannelsDatarate = verify.DatarateParams.Datarate;
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
            verify.DatarateParams.UplinkDwellTime = MacCtx.NvmCtx->MacParams.UplinkDwellTime;

            if( RegionVerify( MacCtx.NvmCtx->Region, &verify, PHY_TX_DR ) == true )
            {
                MacCtx.NvmCtx->MacParams.ChannelsDatarate = verify.DatarateParams.Datarate;
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

            if( RegionVerify( MacCtx.NvmCtx->Region, &verify, PHY_DEF_TX_POWER ) == true )
            {
                MacCtx.NvmCtx->MacParamsDefaults.ChannelsTxPower = verify.TxPower;
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

            if( RegionVerify( MacCtx.NvmCtx->Region, &verify, PHY_TX_POWER ) == true )
            {
                MacCtx.NvmCtx->MacParams.ChannelsTxPower = verify.TxPower;
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
            MacCtx.NvmCtx->MacParams.SystemMaxRxError = MacCtx.NvmCtx->MacParamsDefaults.SystemMaxRxError = mibSet->Param.SystemMaxRxError;
            break;
        }
        case MIB_MIN_RX_SYMBOLS:
        {
            MacCtx.NvmCtx->MacParams.MinRxSymbols = MacCtx.NvmCtx->MacParamsDefaults.MinRxSymbols = mibSet->Param.MinRxSymbols;
            break;
        }
        case MIB_ANTENNA_GAIN:
        {
            MacCtx.NvmCtx->MacParams.AntennaGain = mibSet->Param.AntennaGain;
            break;
        }
        case MIB_DEFAULT_ANTENNA_GAIN:
        {
            MacCtx.NvmCtx->MacParamsDefaults.AntennaGain = mibSet->Param.DefaultAntennaGain;
            break;
        }
        default:
        {
            if( MacCtx.NvmCtx->DeviceClass == CLASS_B )
            {
                status = LoRaMacMibClassBSetRequestConfirm( mibSet );
            }
            else
            {
                status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            }
            break;
        }
    }

    return status;
}

LoRaMacStatus_t LoRaMacChannelAdd( uint8_t id, ChannelParams_t params )
{
    ChannelAddParams_t channelAdd;

    // Validate if the MAC is in a correct state
    if( ( MacCtx.MacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
    {
        if( ( MacCtx.MacState & LORAMAC_TX_CONFIG ) != LORAMAC_TX_CONFIG )
        {
            return LORAMAC_STATUS_BUSY;
        }
    }

    channelAdd.NewChannel = &params;
    channelAdd.ChannelId = id;

    return RegionChannelAdd( MacCtx.NvmCtx->Region, &channelAdd );
}

LoRaMacStatus_t LoRaMacChannelRemove( uint8_t id )
{
    ChannelRemoveParams_t channelRemove;

    if( ( MacCtx.MacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
    {
        if( ( MacCtx.MacState & LORAMAC_TX_CONFIG ) != LORAMAC_TX_CONFIG )
        {
            return LORAMAC_STATUS_BUSY;
        }
    }

    channelRemove.ChannelId = id;

    if( RegionChannelsRemove( MacCtx.NvmCtx->Region, &channelRemove ) == false )
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
    if( ( MacCtx.MacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    // Calculate class b parameters
    LoRaMacClassBSetMulticastPeriodicity( channelParam );

    // Reset downlink counter
    channelParam->DownLinkCounter = 0;
    channelParam->Next = NULL;

    if( MacCtx.NvmCtx->MulticastChannelList == NULL )
    {
        // New node is the fist element
        MacCtx.NvmCtx->MulticastChannelList = channelParam;
    }
    else
    {
        MulticastParams_t *cur = MacCtx.NvmCtx->MulticastChannelList;

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
    if( ( MacCtx.MacState & LORAMAC_TX_RUNNING ) == LORAMAC_TX_RUNNING )
    {
        return LORAMAC_STATUS_BUSY;
    }

    if( MacCtx.NvmCtx->MulticastChannelList != NULL )
    {
        if( MacCtx.NvmCtx->MulticastChannelList == channelParam )
        {
          // First element
          MacCtx.NvmCtx->MulticastChannelList = channelParam->Next;
        }
        else
        {
            MulticastParams_t *cur = MacCtx.NvmCtx->MulticastChannelList;

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
    MlmeConfirmQueue_t queueElement;

    if( mlmeRequest == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( MacCtx.MacState != LORAMAC_IDLE )
    {
        return LORAMAC_STATUS_BUSY;
    }
    if( LoRaMacConfirmQueueIsFull( ) == true )
    {
        return LORAMAC_STATUS_BUSY;
    }

    // Switch requests
    switch( mlmeRequest->Type )
    {
        case MLME_JOIN:
        {
            if( ( mlmeRequest->Req.Join.DevEui == NULL ) ||
                ( mlmeRequest->Req.Join.AppEui == NULL ) ||
                ( mlmeRequest->Req.Join.AppKey == NULL ) )
            {
                return LORAMAC_STATUS_PARAMETER_INVALID;
            }

            ResetMacParameters( );

            MacCtx.MacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;

            MacCtx.DevEui = mlmeRequest->Req.Join.DevEui;
            MacCtx.AppEui = mlmeRequest->Req.Join.AppEui;
            LoRaMacAppKey = mlmeRequest->Req.Join.AppKey;
            MacCtx.NvmCtx->MacParams.ChannelsDatarate = RegionAlternateDr( MacCtx.NvmCtx->Region, mlmeRequest->Req.Join.Datarate );;

            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );

            // Setup header information
            macHdr.Value = 0;
            macHdr.Bits.MType  = FRAME_TYPE_JOIN_REQ;

            status = Send( &macHdr, 0, NULL, 0 );
            break;
        }
        case MLME_LINK_CHECK:
        {
            // Apply the request
            MacCtx.MacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );

            // LoRaMac will send this command piggy-pack
            status = AddMacCommand( MOTE_MAC_LINK_CHECK_REQ, 0, 0 );
            break;
        }
        case MLME_TXCW:
        {
            // Apply the request
            MacCtx.MacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );

            status = SetTxContinuousWave( mlmeRequest->Req.TxCw.Timeout );
            break;
        }
        case MLME_TXCW_1:
        {
            // Apply the request
            MacCtx.MacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );

            status = SetTxContinuousWave1( mlmeRequest->Req.TxCw.Timeout, mlmeRequest->Req.TxCw.Frequency, mlmeRequest->Req.TxCw.Power );
            break;
        }
        case MLME_DEVICE_TIME:
        {
            // Apply the request
            MacCtx.MacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );

            // LoRaMac will send this command piggy-pack
            status = AddMacCommand( MOTE_MAC_DEVICE_TIME_REQ, 0, 0 );
            break;
        }
        case MLME_PING_SLOT_INFO:
        {
            uint8_t value = mlmeRequest->Req.PingSlotInfo.PingSlot.Value;

            // Apply the request
            MacCtx.MacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );

            // LoRaMac will send this command piggy-pack
            LoRaMacClassBSetPingSlotInfo( mlmeRequest->Req.PingSlotInfo.PingSlot.Fields.Periodicity );

            status = AddMacCommand( MOTE_MAC_PING_SLOT_INFO_REQ, value, 0 );
            break;
        }
        case MLME_BEACON_TIMING:
        {
            // Apply the request
            MacCtx.MacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = false;
            LoRaMacConfirmQueueAdd( &queueElement );

            // LoRaMac will send this command piggy-pack
            status = AddMacCommand( MOTE_MAC_BEACON_TIMING_REQ, 0, 0 );
            break;
        }
        case MLME_BEACON_ACQUISITION:
        {
            // Apply the request
            MacCtx.MacFlags.Bits.MlmeReq = 1;
            queueElement.Request = mlmeRequest->Type;
            queueElement.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;
            queueElement.RestrictCommonReadyToHandle = true;
            LoRaMacConfirmQueueAdd( &queueElement );

            if( LoRaMacClassBIsAcquisitionInProgress( ) == false )
            {
                // Start class B algorithm
                LoRaMacClassBSetBeaconState( BEACON_STATE_ACQUISITION );
                LoRaMacClassBBeaconTimerEvent( );

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
        MacCtx.NvmCtx->NodeAckRequested = false;
        LoRaMacConfirmQueueRemoveLast( );
        if( LoRaMacConfirmQueueGetCnt( ) == 0 )
        {
            MacCtx.MacFlags.Bits.MlmeReq = 0;
        }
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
    int8_t datarate = DR_0;
    bool readyToSend = false;

    if( mcpsRequest == NULL )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }
    if( MacCtx.MacState != LORAMAC_IDLE )
    {
        return LORAMAC_STATUS_BUSY;
    }

    macHdr.Value = 0;
    memset1 ( ( uint8_t* ) &MacCtx.McpsConfirm, 0, sizeof( MacCtx.McpsConfirm ) );
    MacCtx.McpsConfirm.Status = LORAMAC_EVENT_INFO_STATUS_ERROR;

    // AckTimeoutRetriesCounter must be reset every time a new request (unconfirmed or confirmed) is performed.
    MacCtx.NvmCtx->AckTimeoutRetriesCounter = 1;

    switch( mcpsRequest->Type )
    {
        case MCPS_UNCONFIRMED:
        {
            readyToSend = true;
            MacCtx.NvmCtx->AckTimeoutRetries = 1;

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
            MacCtx.NvmCtx->AckTimeoutRetries = mcpsRequest->Req.Confirmed.NbTrials;

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
            MacCtx.NvmCtx->AckTimeoutRetries = 1;

            macHdr.Bits.MType = FRAME_TYPE_PROPRIETARY;
            fBuffer = mcpsRequest->Req.Proprietary.fBuffer;
            fBufferSize = mcpsRequest->Req.Proprietary.fBufferSize;
            datarate = mcpsRequest->Req.Proprietary.Datarate;
            break;
        }
        default:
            break;
    }

    // Filter fPorts
    if( IsFPortAllowed( fPort ) == false )
    {
        return LORAMAC_STATUS_PARAMETER_INVALID;
    }

    // Get the minimum possible datarate
    getPhy.Attribute = PHY_MIN_TX_DR;
    getPhy.UplinkDwellTime = MacCtx.NvmCtx->MacParams.UplinkDwellTime;
    phyParam = RegionGetPhyParam( MacCtx.NvmCtx->Region, &getPhy );
    // Apply the minimum possible datarate.
    // Some regions have limitations for the minimum datarate.
    datarate = MAX( datarate, phyParam.Value );

    if( readyToSend == true )
    {
        if( MacCtx.NvmCtx->AdrCtrlOn == false )
        {
            verify.DatarateParams.Datarate = datarate;
            verify.DatarateParams.UplinkDwellTime = MacCtx.NvmCtx->MacParams.UplinkDwellTime;

            if( RegionVerify( MacCtx.NvmCtx->Region, &verify, PHY_TX_DR ) == true )
            {
                MacCtx.NvmCtx->MacParams.ChannelsDatarate = verify.DatarateParams.Datarate;
            }
            else
            {
                return LORAMAC_STATUS_PARAMETER_INVALID;
            }
        }

        status = Send( &macHdr, fPort, fBuffer, fBufferSize );
        if( status == LORAMAC_STATUS_OK )
        {
            MacCtx.McpsConfirm.McpsRequest = mcpsRequest->Type;
            MacCtx.MacFlags.Bits.McpsReq = 1;
        }
        else
        {
            MacCtx.NvmCtx->NodeAckRequested = false;
        }
    }

    return status;
}

void LoRaMacTestSetDutyCycleOn( bool enable )
{
    VerifyParams_t verify;

    verify.DutyCycle = enable;

    if( RegionVerify( MacCtx.NvmCtx->Region, &verify, PHY_DUTY_CYCLE ) == true )
    {
        MacCtx.NvmCtx->DutyCycleOn = enable;
    }
}
