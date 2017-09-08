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

Description: LoRa MAC Class B layer implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis ( Semtech ), Gregory Cristian ( Semtech ) and Daniel Jaeckle ( STACKFORCE )
*/
#include "board.h"
#include "LoRaMac.h"
#include "region/Region.h"
#include "LoRaMacClassB.h"
#include "LoRaMacCrypto.h"

#ifdef LORAMAC_CLASSB_ENABLED
/*!
 * State of the beaconing mechanism
 */
static BeaconState_t BeaconState;

/*!
 * State of the ping slot mechanism
 */
static PingSlotState_t PingSlotState;

/*!
 * State of the multicast slot mechanism
 */
static PingSlotState_t MulticastSlotState;

/*!
 * Class B ping slot context
 */
static PingSlotContext_t PingSlotCtx;

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
 * Container for the callbacks related to class b.
 */
static LoRaMacClassBCallback_t LoRaMacClassBCallbacks;

/*!
 * Data structure which holds the parameters which needs to be set
 * in class b operation.
 */
static LoRaMacClassBParams_t LoRaMacClassBParams;


/*!
 * \brief Calculates the next ping slot time.
 *
 * \param [IN] slotOffset The ping slot offset
 * \param [IN] pingPeriod The ping period
 * \param [OUT] timeOffset Time offset of the next slot, based on current time
 *
 * \retval [true: ping slot found, false: no ping slot found]
 */
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

/*!
 * \brief Calculates CRC's of the beacon frame
 *
 * \param [IN] buffer Pointer to the data
 * \param [IN] length Length of the data
 *
 * \retval CRC
 */
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

static void GetTemperatureLevel( LoRaMacClassBCallback_t *callbacks, BeaconContext_t *beaconCtx )
{
    // Measure temperature, if available
    if( ( callbacks != NULL ) && ( callbacks->GetTemperatureLevel != NULL ) )
    {
        beaconCtx->Temperature = callbacks->GetTemperatureLevel( );
    }
}

#endif // LORAMAC_CLASSB_ENABLED

void LoRaMacClassBInit( LoRaMacClassBParams_t *classBParams, LoRaMacClassBCallback_t *callbacks )
{
#ifdef LORAMAC_CLASSB_ENABLED
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;

    // Init variables to default
    memset1( ( uint8_t* ) &BeaconCtx, 0, sizeof( BeaconContext_t ) );
    memset1( ( uint8_t* ) &PingSlotCtx, 0, sizeof( PingSlotCtx ) );

    // Store callbacks
    LoRaMacClassBCallbacks = *callbacks;

    // Store parameter pointers
    LoRaMacClassBParams = *classBParams;

    // Setup default temperature
    BeaconCtx.Temperature = 25.0;
    GetTemperatureLevel( &LoRaMacClassBCallbacks, &BeaconCtx );

    // Initialize timers
    TimerInit( &BeaconTimer, LoRaMacClassBBeaconTimerEvent );
    TimerInit( &PingSlotTimer, LoRaMacClassBPingSlotTimerEvent );
    TimerInit( &MulticastSlotTimer, LoRaMacClassBMulticastSlotTimerEvent );

    // Setup default states
    BeaconState = BEACON_STATE_ACQUISITION;
    PingSlotState = PINGSLOT_STATE_SET_TIMER;
    MulticastSlotState = PINGSLOT_STATE_SET_TIMER;

    // Get phy parameters
    getPhy.Attribute = PHY_BEACON_INTERVAL;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    BeaconCtx.Cfg.Interval = phyParam.Value;

    getPhy.Attribute = PHY_BEACON_RESERVED;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    BeaconCtx.Cfg.Reserved = phyParam.Value;

    getPhy.Attribute = PHY_BEACON_GUARD;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    BeaconCtx.Cfg.Guard = phyParam.Value;

    getPhy.Attribute = PHY_BEACON_WINDOW;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    BeaconCtx.Cfg.Window = phyParam.Value;

    getPhy.Attribute = PHY_BEACON_WINDOW_SLOTS;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    BeaconCtx.Cfg.WindowSlots = phyParam.Value;

    getPhy.Attribute = PHY_BEACON_SYMBOL_TO_DEFAULT;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    BeaconCtx.Cfg.SymbolToDefault = phyParam.Value;

    getPhy.Attribute = PHY_BEACON_SYMBOL_TO_EXPANSION_MAX;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    BeaconCtx.Cfg.SymbolToExpansionMax = phyParam.Value;

    getPhy.Attribute = PHY_BEACON_SYMBOL_TO_EXPANSION_FACTOR;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    BeaconCtx.Cfg.SymbolToExpansionFactor = phyParam.Value;

    getPhy.Attribute = PHY_MAX_BEACON_LESS_PERIOD;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    BeaconCtx.Cfg.MaxBeaconLessPeriod = phyParam.Value;

    getPhy.Attribute = PHY_PING_SLOT_WINDOW;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    PingSlotCtx.Cfg.PingSlotWindow = phyParam.Value;

    getPhy.Attribute = PHY_PING_SLOT_SYMBOL_TO_EXPANSION_MAX;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    PingSlotCtx.Cfg.SymbolToExpansionMax = phyParam.Value;

    getPhy.Attribute = PHY_PING_SLOT_SYMBOL_TO_EXPANSION_FACTOR;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    PingSlotCtx.Cfg.SymbolToExpansionFactor = phyParam.Value;
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBSetBeaconState( BeaconState_t beaconState )
{
#ifdef LORAMAC_CLASSB_ENABLED
    BeaconState = beaconState;
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBSetPingSlotState( PingSlotState_t pingSlotState )
{
#ifdef LORAMAC_CLASSB_ENABLED
    PingSlotState = pingSlotState;
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBBeaconTimerEvent( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
    RxBeaconSetup_t rxBeaconSetup;
    uint8_t index = 0;
    bool activateTimer = false;
    TimerTime_t beaconEventTime = 1;
    TimerTime_t currentTime = TimerGetCurrentTime( );

    TimerStop( &BeaconTimer );

    // Beacon state machine
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

                BeaconCtx.Ctrl.AcquisitionPending = 1;
                beaconEventTime = BeaconCtx.Cfg.Interval;

                rxBeaconSetup.SymbolTimeout = BeaconCtx.SymbolTimeout;
                rxBeaconSetup.RxTime = 0;
                rxBeaconSetup.DeviceAddress = *LoRaMacClassBParams.LoRaMacDevAddr;
                rxBeaconSetup.CustomFrequencyEnabled = BeaconCtx.Ctrl.CustomFreq;
                rxBeaconSetup.CustomFrequency = BeaconCtx.Frequency;
                rxBeaconSetup.BeaconTime = BeaconCtx.BeaconTime;
                rxBeaconSetup.BeaconInterval = BeaconCtx.Cfg.Interval;

                RegionRxBeaconSetup( *LoRaMacClassBParams.LoRaMacRegion, &rxBeaconSetup, &LoRaMacClassBParams.McpsIndication->RxDatarate );
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
            if( ( currentTime - BeaconCtx.LastBeaconRx ) > BeaconCtx.Cfg.MaxBeaconLessPeriod )
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
                LoRaMacClassBParams.MlmeIndication->MlmeIndication = MLME_BEACON;
                LoRaMacClassBParams.MlmeIndication->Status = LORAMAC_EVENT_INFO_STATUS_BEACON_LOST;
                LoRaMacClassBParams.LoRaMacFlags->Bits.MlmeInd = 1;

                TimerSetValue( LoRaMacClassBParams.MacStateCheckTimer, 1 );
                TimerStart( LoRaMacClassBParams.MacStateCheckTimer );
                LoRaMacClassBParams.LoRaMacFlags->Bits.MacDone = 1;
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

            if( LoRaMacClassBParams.LoRaMacFlags->Bits.MlmeReq == 1 )
            {
                // index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_BEACON_ACQUISITION, MlmeConfirmQueueCnt );
                index = LoRaMacClassBCallbacks.GetMlmeConfrimIndex( LoRaMacClassBParams.MlmeConfirmQueue, MLME_BEACON_ACQUISITION );
                if( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
                {
                    LoRaMacClassBParams.MlmeConfirmQueue[index].Status = LORAMAC_EVENT_INFO_STATUS_OK;
                    LoRaMacClassBParams.MlmeConfirm->TxTimeOnAir = 0;
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
                LoRaMacClassBParams.MlmeIndication->MlmeIndication = MLME_BEACON;
                LoRaMacClassBParams.MlmeIndication->Status = LORAMAC_EVENT_INFO_STATUS_BEACON_LOCKED;
                LoRaMacClassBParams.LoRaMacFlags->Bits.MlmeInd = 1;

                TimerSetValue( LoRaMacClassBParams.MacStateCheckTimer, 1 );
                TimerStart( LoRaMacClassBParams.MacStateCheckTimer );
                LoRaMacClassBParams.LoRaMacFlags->Bits.MacDone = 1;
            }
            BeaconCtx.Ctrl.ResumeBeaconing = 0;
            break;
        }
        case BEACON_STATE_IDLE:
        {
            activateTimer = true;
            GetTemperatureLevel( &LoRaMacClassBCallbacks, &BeaconCtx );
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

            rxBeaconSetup.SymbolTimeout = BeaconCtx.SymbolTimeout;
            rxBeaconSetup.RxTime = BeaconCtx.Cfg.Reserved;
            rxBeaconSetup.DeviceAddress = *LoRaMacClassBParams.LoRaMacDevAddr;
            rxBeaconSetup.CustomFrequencyEnabled = BeaconCtx.Ctrl.CustomFreq;
            rxBeaconSetup.CustomFrequency = BeaconCtx.Frequency;
            rxBeaconSetup.BeaconTime = BeaconCtx.BeaconTime;
            rxBeaconSetup.BeaconInterval = BeaconCtx.Cfg.Interval;

            RegionRxBeaconSetup( *LoRaMacClassBParams.LoRaMacRegion, &rxBeaconSetup, &LoRaMacClassBParams.McpsIndication->RxDatarate );
            break;
        }
        case BEACON_STATE_SWITCH_CLASS:
        {
            if( LoRaMacClassBParams.LoRaMacFlags->Bits.MlmeReq == 1 )
            {
                // index = GetMlmeConfirmIndex( MlmeConfirmQueue, MLME_BEACON_ACQUISITION, MlmeConfirmQueueCnt );
                index = LoRaMacClassBCallbacks.GetMlmeConfrimIndex( LoRaMacClassBParams.MlmeConfirmQueue, MLME_BEACON_ACQUISITION );
                if( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
                {
                    LoRaMacClassBParams.MlmeConfirmQueue[index].Status = LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND;
                }
            }
            else
            {
                LoRaMacClassBParams.MlmeIndication->MlmeIndication = MLME_SWITCH_CLASS;
                LoRaMacClassBParams.MlmeIndication->Status = LORAMAC_EVENT_INFO_STATUS_OK;
                PingSlotCtx.Ctrl.Assigned = 0;
                LoRaMacClassBParams.LoRaMacFlags->Bits.MlmeInd = 1;
            }
            BeaconState = BEACON_STATE_ACQUISITION;

            BeaconCtx.Ctrl.BeaconMode = 0;
            BeaconCtx.Ctrl.AcquisitionPending = 0;
            LoRaMacClassBParams.LoRaMacFlags->Bits.MacDone = 1;

            TimerSetValue( LoRaMacClassBParams.MacStateCheckTimer, beaconEventTime );
            TimerStart( LoRaMacClassBParams.MacStateCheckTimer );
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
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBPingSlotTimerEvent( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    RxConfigParams_t pingSlotRxConfig;
    TimerTime_t pingSlotTime = 0;

    TimerStop( &PingSlotTimer );

    switch( PingSlotState )
    {
        case PINGSLOT_STATE_CALC_PING_OFFSET:
        {
            LoRaMacBeaconComputePingOffset( BeaconCtx.BeaconTime,
                                            *LoRaMacClassBParams.LoRaMacDevAddr,
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
                getPhy.Attribute = PHY_PINGSLOT_CHANNEL_FREQ;
                getPhy.DeviceAddress = *LoRaMacClassBParams.LoRaMacDevAddr;
                phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
                frequency = phyParam.Value;
            }

            if( MulticastSlotState != PINGSLOT_STATE_RX )
            {
                if( BeaconCtx.Ctrl.BeaconAcquired == 1 )
                {
                    RegionComputeRxWindowParameters( *LoRaMacClassBParams.LoRaMacRegion,
                                                     PingSlotCtx.Datarate,
                                                     LoRaMacClassBParams.LoRaMacParams->MinRxSymbols,
                                                     LoRaMacClassBParams.LoRaMacParams->SystemMaxRxError,
                                                     &pingSlotRxConfig );
                    PingSlotCtx.SymbolTimeout = pingSlotRxConfig.WindowTimeout;
                }
                PingSlotState = PINGSLOT_STATE_RX;

                pingSlotRxConfig.Datarate = PingSlotCtx.Datarate;
                pingSlotRxConfig.DownlinkDwellTime = LoRaMacClassBParams.LoRaMacParams->DownlinkDwellTime;
                pingSlotRxConfig.RepeaterSupport = LoRaMacClassBParams.LoRaMacParams->RepeaterSupport;
                pingSlotRxConfig.Frequency = frequency;
                pingSlotRxConfig.RxContinuous = false;
                pingSlotRxConfig.Window = 1;

                RegionRxConfig( *LoRaMacClassBParams.LoRaMacRegion, &pingSlotRxConfig, ( int8_t* )&LoRaMacClassBParams.McpsIndication->RxDatarate );

                if( pingSlotRxConfig.RxContinuous == false )
                {
                    Radio.Rx( LoRaMacClassBParams.LoRaMacParams->MaxRxWindow );
                }
                else
                {
                    Radio.Rx( 0 ); // Continuous mode
                }
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
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBMulticastSlotTimerEvent( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    RxConfigParams_t multicastSlotRxConfig;
    TimerTime_t multicastSlotTime = 0;
    TimerTime_t slotTime = 0;
    MulticastParams_t *cur = LoRaMacClassBParams.MulticastChannels;

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
            cur = LoRaMacClassBParams.MulticastChannels;
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
                getPhy.Attribute = PHY_PINGSLOT_CHANNEL_FREQ;
                getPhy.DeviceAddress = PingSlotCtx.NextMulticastChannel->Address;
                phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
                frequency = phyParam.Value;
            }

            if( BeaconCtx.Ctrl.BeaconAcquired == 1 )
            {
                RegionComputeRxWindowParameters( *LoRaMacClassBParams.LoRaMacRegion,
                                                 PingSlotCtx.Datarate,
                                                 LoRaMacClassBParams.LoRaMacParams->MinRxSymbols,
                                                 LoRaMacClassBParams.LoRaMacParams->SystemMaxRxError,
                                                 &multicastSlotRxConfig );
                PingSlotCtx.SymbolTimeout = multicastSlotRxConfig.WindowTimeout;
            }
            MulticastSlotState = PINGSLOT_STATE_RX;

            multicastSlotRxConfig.Datarate = PingSlotCtx.Datarate;
            multicastSlotRxConfig.DownlinkDwellTime = LoRaMacClassBParams.LoRaMacParams->DownlinkDwellTime;
            multicastSlotRxConfig.RepeaterSupport = LoRaMacClassBParams.LoRaMacParams->RepeaterSupport;
            multicastSlotRxConfig.Frequency = frequency;
            multicastSlotRxConfig.RxContinuous = false;
            multicastSlotRxConfig.Window = 1;

            RegionRxConfig( *LoRaMacClassBParams.LoRaMacRegion, &multicastSlotRxConfig, ( int8_t* )&LoRaMacClassBParams.McpsIndication->RxDatarate );

            if( multicastSlotRxConfig.RxContinuous == false )
            {
                Radio.Rx( LoRaMacClassBParams.LoRaMacParams->MaxRxWindow );
            }
            else
            {
                Radio.Rx( 0 ); // Continuous mode
            }
            break;
        }
        default:
        {
            MulticastSlotState = PINGSLOT_STATE_SET_TIMER;
            break;
        }
    }
#endif // LORAMAC_CLASSB_ENABLED
}

bool LoRaMacClassBRxBeacon( uint8_t *payload, uint16_t size )
{
#ifdef LORAMAC_CLASSB_ENABLED
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    bool beaconReceived = false;
    uint16_t crc0 = 0;
    uint16_t crc1 = 0;
    uint16_t beaconCrc0 = 0;
    uint16_t beaconCrc1 = 0;
    uint8_t rfuOffset1 = 0;
    uint8_t rfuOffset2 = 0;

    getPhy.Attribute = PHY_BEACON_SIZE;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );

    // For beacon payload sizes > 17 we need to apply an offset
    if( phyParam.Value > 17 )
    {
        rfuOffset1 = 1;
        rfuOffset2 = 1;
    }

    // Verify if we are in the state where we expect a beacon
    if( ( BeaconState == BEACON_STATE_RX ) || ( BeaconCtx.Ctrl.AcquisitionPending == 1 ) )
    {
        if( size == phyParam.Value )
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
                LoRaMacClassBParams.MlmeIndication->BeaconInfo.Time = BeaconCtx.BeaconTime;
                beaconReceived = true;
            }

            beaconCrc1 = ( ( uint16_t ) payload[15 + rfuOffset1 + rfuOffset2] ) & 0x00FF;
            beaconCrc1 |= ( ( uint16_t ) payload[16 + rfuOffset1 + rfuOffset2] << 8 ) & 0xFF00;
            crc1 = BeaconCrc( &payload[8 + rfuOffset1], 7 + rfuOffset2 );

            // Validate the second crc of the beacon frame
            if( crc1 == beaconCrc1 )
            {
                // Beacon valid, apply data
                LoRaMacClassBParams.MlmeIndication->BeaconInfo.GwSpecific.InfoDesc = payload[8 + rfuOffset1];
                memcpy1( LoRaMacClassBParams.MlmeIndication->BeaconInfo.GwSpecific.Info, &payload[9 + rfuOffset1], 7 );
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

                LoRaMacClassBBeaconTimerEvent( );
            }
        }

        if( BeaconState == BEACON_STATE_RX )
        {
            BeaconState = BEACON_STATE_TIMEOUT;
            LoRaMacClassBBeaconTimerEvent( );
        }
        // Return always true, when we expect a beacon.
        beaconReceived = true;
    }

    return beaconReceived;
#else
    return false;
#endif // LORAMAC_CLASSB_ENABLED
}

bool LoRaMacClassBIsBeaconExpected( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
    if( ( BeaconCtx.Ctrl.AcquisitionPending == 1 ) ||
        ( BeaconState == BEACON_STATE_RX ) )
    {
        return true;
    }
    return false;
#else
    return false;
#endif // LORAMAC_CLASSB_ENABLED
}

bool LoRaMacClassBIsPingExpected( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
    if( PingSlotState == PINGSLOT_STATE_RX )
    {
        return true;
    }
    return false;
#else
    return false;
#endif // LORAMAC_CLASSB_ENABLED
}

bool LoRaMacClassBIsAcquisitionPending( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
    if( BeaconCtx.Ctrl.AcquisitionPending == 1 )
    {
        return true;
    }
    return false;
#else
    return false;
#endif // LORAMAC_CLASSB_ENABLED
}

bool LoRaMacClassBIsBeaconModeActive( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
    if( BeaconCtx.Ctrl.BeaconMode == 1 )
    {
        return true;
    }
    return false;
#else
    return false;
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBSetPingSlotInfo( uint8_t periodicity )
{
#ifdef LORAMAC_CLASSB_ENABLED
    PingSlotCtx.PingNb = 128 / ( 1 << periodicity );
    PingSlotCtx.PingPeriod = BeaconCtx.Cfg.WindowSlots / PingSlotCtx.PingNb;
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBHaltBeaconing( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
    if( ( BeaconState == BEACON_STATE_TIMEOUT ) ||
        ( BeaconState == BEACON_STATE_SWITCH_CLASS ) )
    {
        // Update the state machine before halt
        LoRaMacClassBBeaconTimerEvent( );
    }

    // Halt beacon state machine
    BeaconState = BEACON_STATE_HALT;

    // Halt ping slot state machine
    TimerStop( &BeaconTimer );

    // Halt ping slot state machine
    TimerStop( &PingSlotTimer );

    // Halt multicast ping slot state machine
    TimerStop( &MulticastSlotTimer );
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBResumeBeaconing( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
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
#endif // LORAMAC_CLASSB_ENABLED
}

LoRaMacStatus_t LoRaMacClassBSwitchClass( DeviceClass_t nextClass )
{
#ifdef LORAMAC_CLASSB_ENABLED
    if( nextClass == CLASS_B )
    {// Switch to from class a to class b
        if( ( BeaconCtx.Ctrl.BeaconMode == 1 ) && ( PingSlotCtx.Ctrl.Assigned == 1 ) )
        {
            return LORAMAC_STATUS_OK;
        }
    }
    if( nextClass == CLASS_A )
    {// Switch from class b to class a
        BeaconState = BEACON_STATE_ACQUISITION;
        return LORAMAC_STATUS_OK;
    }
    return LORAMAC_STATUS_SERVICE_UNKNOWN;
#else
    return LORAMAC_STATUS_SERVICE_UNKNOWN;
#endif // LORAMAC_CLASSB_ENABLED
}

LoRaMacStatus_t LoRaMacClassBMibGetRequestConfirm( MibRequestConfirm_t *mibGet )
{
#ifdef LORAMAC_CLASSB_ENABLED
    LoRaMacStatus_t status;

    switch( mibGet->Type )
    {
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
        {
            status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            break;
        }
    }
    return status;
#else
    return LORAMAC_STATUS_SERVICE_UNKNOWN;
#endif // LORAMAC_CLASSB_ENABLED
}

LoRaMacStatus_t LoRaMacMibClassBSetRequestConfirm( MibRequestConfirm_t *mibSet )
{
#ifdef LORAMAC_CLASSB_ENABLED
    LoRaMacStatus_t status;

    switch( mibSet->Type )
    {
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
        {
            status = LORAMAC_STATUS_SERVICE_UNKNOWN;
            break;
        }
    }
    return status;
#else
    return LORAMAC_STATUS_SERVICE_UNKNOWN;
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBPingSlotInfoAns( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
    uint8_t index = LORA_MAC_MLME_CONFIRM_QUEUE_LEN;

    index = LoRaMacClassBCallbacks.GetMlmeConfrimIndex( LoRaMacClassBParams.MlmeConfirmQueue, MLME_PING_SLOT_INFO );
    if( index < LORA_MAC_MLME_CONFIRM_QUEUE_LEN )
    {
        LoRaMacClassBParams.MlmeConfirmQueue[index].Status = LORAMAC_EVENT_INFO_STATUS_OK;
        PingSlotCtx.Ctrl.Assigned = 1;
    }
#endif // LORAMAC_CLASSB_ENABLED
}

uint8_t LoRaMacClassBPingSlotChannelReq( uint8_t datarate, uint32_t frequency )
{
#ifdef LORAMAC_CLASSB_ENABLED
    uint8_t status = 0x03;
    VerifyParams_t verify;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;

    if( frequency != 0 )
    {
        if( Radio.CheckRfFrequency( frequency ) == false )
        {
            status &= 0xFE; // Channel frequency KO
        }

        verify.DatarateParams.Datarate = datarate;
        verify.DatarateParams.DownlinkDwellTime = LoRaMacClassBParams.LoRaMacParams->DownlinkDwellTime;

        if( RegionVerify( *LoRaMacClassBParams.LoRaMacRegion, &verify, PHY_RX_DR ) == false )
        {
            status &= 0xFD; // Datarate range KO
        }

        if( status == 0x03 )
        {
            PingSlotCtx.Ctrl.CustomFreq = 1;
            PingSlotCtx.Frequency = frequency;
            PingSlotCtx.Datarate = datarate;
        }
    }
    else
    {
        getPhy.Attribute = PHY_BEACON_CHANNEL_DR;
        phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );

        PingSlotCtx.Ctrl.CustomFreq = 0;
        PingSlotCtx.Datarate = phyParam.Value;
    }
    return status;
#else
    return 0;
#endif // LORAMAC_CLASSB_ENABLED
}

bool LoRaMacClassBBeaconFreqReq( uint32_t frequency )
{
#ifdef LORAMAC_CLASSB_ENABLED
    if( frequency != 0 )
    {
        if( Radio.CheckRfFrequency( frequency ) == true )
        {
            BeaconCtx.Ctrl.CustomFreq = 1;
            BeaconCtx.Frequency = frequency;
            return true;
        }
    }
    else
    {
        BeaconCtx.Ctrl.CustomFreq = 0;
        return true;
    }
    return false;
#else
    return false;
#endif // LORAMAC_CLASSB_ENABLED
}

TimerTime_t LoRaMacClassBGetBeaconReservedTime( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
    return BeaconCtx.Cfg.Reserved;
#else
    return 0;
#endif // LORAMAC_CLASSB_ENABLED
}

TimerTime_t LoRaMacClassBGetPingSlotWinTime( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
    return PingSlotCtx.Cfg.PingSlotWindow;
#else
    return 0;
#endif // LORAMAC_CLASSB_ENABLED
}

TimerTime_t LoRaMacClassBIsUplinkCollision( TimerTime_t txTimeOnAir )
{
#ifdef LORAMAC_CLASSB_ENABLED
    TimerTime_t currentTime = TimerGetCurrentTime( );
    TimerTime_t beaconReserved = 0;

    beaconReserved = BeaconCtx.NextBeaconRx -
                     BeaconCtx.Cfg.Guard -
                     LoRaMacClassBParams.LoRaMacParams->ReceiveDelay1 -
                     LoRaMacClassBParams.LoRaMacParams->ReceiveDelay2 -
                     txTimeOnAir;

    // Check if the next beacon will be received during the next uplink.
    if( ( currentTime >= beaconReserved ) && ( currentTime < ( BeaconCtx.NextBeaconRx + BeaconCtx.Cfg.Reserved ) ) )
    {// Next beacon will be sent during the next uplink.
        return BeaconCtx.Cfg.Reserved;
    }
    return 0;
#else
    return 0;
#endif // LORAMAC_CLASSB_ENABLED
}
