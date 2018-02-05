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
#include <math.h>
#include "utilities.h"
#include "LoRaMac.h"
#include "LoRaMacClassB.h"
#include "LoRaMacCrypto.h"
#include "LoRaMacConfirmQueue.h"

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
 * \brief Calculates the downlink frequency for a given channel.
 *
 * \param [IN] channel The channel according to the channel plan.
 *
 * \retval The downlink frequency
 */
static uint32_t CalcDownlinkFrequency( uint8_t channel )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    uint32_t frequency = 0;
    uint32_t stepwidth = 0;

    getPhy.Attribute = PHY_BEACON_CHANNEL_FREQ;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    frequency = phyParam.Value;

    getPhy.Attribute = PHY_BEACON_CHANNEL_STEPWIDTH;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    stepwidth = phyParam.Value;

    // Calculate the frequency
    return frequency + ( channel * stepwidth );
}

/*!
 * \brief Calculates the downlink channel for the beacon and for
 *        ping slot downlinks.
 *
 * \param [IN] devAddr The address of the device
 *
 * \param [IN] beaconTime The beacon time of the beacon.
 *
 * \param [IN] beaconInterval The beacon interval
 *
 * \retval The downlink channel
 */
static uint32_t CalcDownlinkChannelAndFrequency( uint32_t devAddr, TimerTime_t beaconTime, TimerTime_t beaconInterval )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    uint32_t channel = 0;
    uint8_t nbChannels = 0;
    uint32_t frequency = 0;

    getPhy.Attribute = PHY_BEACON_NB_CHANNELS;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    nbChannels = (uint8_t) phyParam.Value;

    if( nbChannels > 1 )
    {
        // Calculate the channel for the next downlink
        channel = devAddr + ( beaconTime / ( beaconInterval / 1000 ) );
        channel = channel % nbChannels;
    }

    // Calculate the frequency for the next downlink
    frequency = CalcDownlinkFrequency( channel );

    // Calculate the frequency for the next downlink
    return frequency;
}

/*!
 * \brief Calculates the correct frequency and opens up the beacon reception window.
 *
 * \param [IN] rxTime The reception time which should be setup
 *
 * \param [IN] activateDefaultChannel Set to true, if the function shall setup the default channel
 */
static void RxBeaconSetup( TimerTime_t rxTime, bool activateDefaultChannel )
{
    RxBeaconSetup_t rxBeaconSetup;
    uint32_t frequency = 0;
    RxConfigParams_t beaconRxConfig;
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;
    uint16_t windowTimeout = BeaconCtx.SymbolTimeout;

    if( activateDefaultChannel == true )
    {
        // This is the default frequency in case we don't know when the next
        // beacon will be transmitted. We select channel 0 as default.
        frequency = CalcDownlinkFrequency( 0 );
    }
    else
    {
        // This is the frequency according to the channel plan
        frequency = CalcDownlinkChannelAndFrequency( 0, BeaconCtx.BeaconTime + ( BeaconCtx.Cfg.Interval / 1000 ), BeaconCtx.Cfg.Interval );
    }

    if( BeaconCtx.Ctrl.CustomFreq == 1 )
    {
        // Set the frequency from the BeaconFreqReq
        frequency = BeaconCtx.Frequency;
    }

    if( BeaconCtx.Ctrl.BeaconChannelSet == 1 )
    {
        // Set the frequency which was provided by BeaconTimingAns MAC command
        BeaconCtx.Ctrl.BeaconChannelSet = 0;
        frequency = CalcDownlinkFrequency( BeaconCtx.BeaconTimingChannel );
    }

    if( ( BeaconCtx.Ctrl.BeaconAcquired == 1 ) || ( BeaconCtx.Ctrl.AcquisitionPending == 1 ) )
    {
        // Apply the symbol timeout only if we have acquired the beacon
        // Otherwise, take the window enlargement into account
        // Read beacon datarate
        getPhy.Attribute = PHY_BEACON_CHANNEL_DR;
        phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );

        // Calculate downlink symbols
        RegionComputeRxWindowParameters( *LoRaMacClassBParams.LoRaMacRegion,
                                        ( int8_t )phyParam.Value, // datarate
                                        LoRaMacClassBParams.LoRaMacParams->MinRxSymbols,
                                        LoRaMacClassBParams.LoRaMacParams->SystemMaxRxError,
                                        &beaconRxConfig );
        windowTimeout = beaconRxConfig.WindowTimeout;
    }

    rxBeaconSetup.SymbolTimeout = windowTimeout;
    rxBeaconSetup.RxTime = rxTime;
    rxBeaconSetup.Frequency = frequency;

    RegionRxBeaconSetup( *LoRaMacClassBParams.LoRaMacRegion, &rxBeaconSetup, &LoRaMacClassBParams.McpsIndication->RxDatarate );
}

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
            slotTime -= Radio.GetWakeupTime( );
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

static void InitClassBDefaults( void )
{
    GetPhyParams_t getPhy;
    PhyParam_t phyParam;

    // Init variables to default
    memset1( ( uint8_t* ) &BeaconCtx, 0, sizeof( BeaconContext_t ) );
    memset1( ( uint8_t* ) &PingSlotCtx, 0, sizeof( PingSlotContext_t ) );

    // Setup default temperature
    BeaconCtx.Temperature = 25.0;
    GetTemperatureLevel( &LoRaMacClassBCallbacks, &BeaconCtx );

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

    getPhy.Attribute = PHY_BEACON_DELAY_BEACON_TIMING_ANS;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    BeaconCtx.Cfg.DelayBeaconTimingAns = phyParam.Value;

    getPhy.Attribute = PHY_PING_SLOT_WINDOW;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    PingSlotCtx.Cfg.PingSlotWindow = phyParam.Value;

    getPhy.Attribute = PHY_PING_SLOT_SYMBOL_TO_EXPANSION_MAX;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    PingSlotCtx.Cfg.SymbolToExpansionMax = phyParam.Value;

    getPhy.Attribute = PHY_PING_SLOT_SYMBOL_TO_EXPANSION_FACTOR;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );
    PingSlotCtx.Cfg.SymbolToExpansionFactor = phyParam.Value;
}
#endif // LORAMAC_CLASSB_ENABLED

void LoRaMacClassBInit( LoRaMacClassBParams_t *classBParams, LoRaMacClassBCallback_t *callbacks )
{
#ifdef LORAMAC_CLASSB_ENABLED
    // Store callbacks
    LoRaMacClassBCallbacks = *callbacks;

    // Store parameter pointers
    LoRaMacClassBParams = *classBParams;

    // Initialize timers
    TimerInit( &BeaconTimer, LoRaMacClassBBeaconTimerEvent );
    TimerInit( &PingSlotTimer, LoRaMacClassBPingSlotTimerEvent );
    TimerInit( &MulticastSlotTimer, LoRaMacClassBMulticastSlotTimerEvent );

    InitClassBDefaults( );
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBSetBeaconState( BeaconState_t beaconState )
{
#ifdef LORAMAC_CLASSB_ENABLED
    if( BeaconState != BEACON_STATE_ACQUISITION )
    {
        // Do only apply a new state if the current state
        // is not BEACON_STATE_ACQUISITION
        BeaconState = beaconState;
    }
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBSetPingSlotState( PingSlotState_t pingSlotState )
{
#ifdef LORAMAC_CLASSB_ENABLED
    PingSlotState = pingSlotState;
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBSetMulticastSlotState( PingSlotState_t multicastSlotState )
{
#ifdef LORAMAC_CLASSB_ENABLED
    MulticastSlotState = multicastSlotState;
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBBeaconTimerEvent( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
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
                BeaconState = BEACON_STATE_LOST;
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
                            // Reset status provides by BeaconTimingAns
                            BeaconCtx.Ctrl.BeaconDelaySet = 0;
                            BeaconCtx.Ctrl.AcquisitionTimerSet = 0;
                            BeaconCtx.Ctrl.BeaconChannelSet = 0;
                            // Set the node into acquisition mode
                            BeaconCtx.Ctrl.AcquisitionPending = 1;

                            beaconEventTime = BeaconCtx.Cfg.Interval;

                            // Use the default channel. We don't know on which
                            // channel the next beacon will be transmitted
                            RxBeaconSetup( 0, true );
                        }
                        BeaconCtx.NextBeaconRx = 0;
                        BeaconCtx.BeaconTimingDelay = 0;
                    }
                    else
                    {
                        activateTimer = false;

                        // Reset status provides by BeaconTimingAns
                        BeaconCtx.Ctrl.BeaconDelaySet = 0;
                        BeaconCtx.Ctrl.AcquisitionTimerSet = 0;
                        // Set the node into acquisition mode
                        BeaconCtx.Ctrl.AcquisitionPending = 1;

                        // Don't use the default channel. We know on which
                        // channel the next beacon will be transmitted
                        RxBeaconSetup( BeaconCtx.Cfg.Reserved, false );
                    }
                }
                else
                {
                    BeaconCtx.Ctrl.AcquisitionPending = 1;
                    beaconEventTime = BeaconCtx.Cfg.Interval;
                    if( BeaconCtx.Ctrl.AcquisitionTimerSet == 0 )
                    {
                        // Start the beacon acquisition. When the MAC has received a beacon in function
                        // RxBeacon successfully, the next state is BEACON_STATE_LOCKED. If the MAC does not
                        // find a beacon, the state machine will stay in state BEACON_STATE_ACQUISITION.
                        // This state detects that a acquisition was pending previously and will change the next
                        // state to BEACON_STATE_LOST.
                        RxBeaconSetup( 0, true );
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
            if( ( currentTime - BeaconCtx.LastBeaconRx ) > BeaconCtx.Cfg.MaxBeaconLessPeriod )
            {
                activateTimer = true;
                BeaconState = BEACON_STATE_LOST;
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
                if( LoRaMacConfirmQueueIsCmdActive( MLME_BEACON_ACQUISITION ) == true )
                {
                    LoRaMacConfirmQueueSetStatus( LORAMAC_EVENT_INFO_STATUS_OK, MLME_BEACON_ACQUISITION );
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
            beaconEventTime = BeaconCtx.NextBeaconRx - Radio.GetWakeupTime( );
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

            // Don't use the default channel. We know on which
            // channel the next beacon will be transmitted
            RxBeaconSetup( BeaconCtx.Cfg.Reserved, false );
            break;
        }
        case BEACON_STATE_LOST:
        {
            // Handle events
            if( LoRaMacClassBParams.LoRaMacFlags->Bits.MlmeReq == 1 )
            {
                if( LoRaMacConfirmQueueIsCmdActive( MLME_BEACON_ACQUISITION ) == true )
                {
                    LoRaMacConfirmQueueSetStatus( LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND, MLME_BEACON_ACQUISITION );
                }
            }
            else
            {
                LoRaMacClassBParams.MlmeIndication->MlmeIndication = MLME_BEACON_LOST;
                LoRaMacClassBParams.MlmeIndication->Status = LORAMAC_EVENT_INFO_STATUS_OK;
                LoRaMacClassBParams.LoRaMacFlags->Bits.MlmeInd = 1;
            }

            // Stop slot timers
            TimerStop( &PingSlotTimer );
            TimerStop( &MulticastSlotTimer );

            // Initialize default state for class b
            InitClassBDefaults( );

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
    static RxConfigParams_t pingSlotRxConfig;
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
                if( BeaconCtx.Ctrl.BeaconAcquired == 1 )
                {
                    // Compute the symbol timeout. Apply it only, if the beacon is acquired
                    // Otherwise, take the enlargement of the symbols into account.
                    RegionComputeRxWindowParameters( *LoRaMacClassBParams.LoRaMacRegion,
                                                     PingSlotCtx.Datarate,
                                                     LoRaMacClassBParams.LoRaMacParams->MinRxSymbols,
                                                     LoRaMacClassBParams.LoRaMacParams->SystemMaxRxError,
                                                     &pingSlotRxConfig );
                    PingSlotCtx.SymbolTimeout = pingSlotRxConfig.WindowTimeout;

                    if( ( int32_t )pingSlotTime > pingSlotRxConfig.WindowOffset )
                    {// Apply the window offset
                        pingSlotTime += pingSlotRxConfig.WindowOffset;
                    }
                }

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

            // Apply a custom frequency if the following bit is set
            if( PingSlotCtx.Ctrl.CustomFreq == 0 )
            {
                // Restore floor plan
                frequency = CalcDownlinkChannelAndFrequency( *LoRaMacClassBParams.LoRaMacDevAddr, BeaconCtx.BeaconTime, BeaconCtx.Cfg.Interval );
            }

            // Open the ping slot window only, if there is no multicast ping slot
            // open. Multicast ping slots have always priority
            if( MulticastSlotState != PINGSLOT_STATE_RX )
            {
                PingSlotState = PINGSLOT_STATE_RX;

                pingSlotRxConfig.Datarate = PingSlotCtx.Datarate;
                pingSlotRxConfig.DownlinkDwellTime = LoRaMacClassBParams.LoRaMacParams->DownlinkDwellTime;
                pingSlotRxConfig.RepeaterSupport = LoRaMacClassBParams.LoRaMacParams->RepeaterSupport;
                pingSlotRxConfig.Frequency = frequency;
                pingSlotRxConfig.RxContinuous = false;
                pingSlotRxConfig.RxSlot = RX_SLOT_WIN_PING_SLOT;

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
    static RxConfigParams_t multicastSlotRxConfig;
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
            // Compute all offsets for every multicast slots
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
                // Calculate the next slot time for every multicast slot
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

            // Schedule the next multicast slot
            if( PingSlotCtx.NextMulticastChannel != NULL )
            {
                if( BeaconCtx.Ctrl.BeaconAcquired == 1 )
                {
                    RegionComputeRxWindowParameters( *LoRaMacClassBParams.LoRaMacRegion,
                                                    PingSlotCtx.Datarate,
                                                    LoRaMacClassBParams.LoRaMacParams->MinRxSymbols,
                                                    LoRaMacClassBParams.LoRaMacParams->SystemMaxRxError,
                                                    &multicastSlotRxConfig );
                    PingSlotCtx.SymbolTimeout = multicastSlotRxConfig.WindowTimeout;
                }

                if( ( int32_t )multicastSlotTime > multicastSlotRxConfig.WindowOffset )
                {// Apply the window offset
                    multicastSlotTime += multicastSlotRxConfig.WindowOffset;
                }

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

            // Verify if the multicast channel is valid
            if( PingSlotCtx.NextMulticastChannel == NULL )
            {
                MulticastSlotState = PINGSLOT_STATE_SET_TIMER;
                TimerSetValue( &MulticastSlotTimer, 1 );
                TimerStart( &MulticastSlotTimer );
                break;
            }

            // Apply a custom frequency if the following bit is set
            if( PingSlotCtx.Ctrl.CustomFreq == 0 )
            {
                // Restore floor plan
                frequency = CalcDownlinkChannelAndFrequency( PingSlotCtx.NextMulticastChannel->Address, BeaconCtx.BeaconTime, BeaconCtx.Cfg.Interval );
            }

            MulticastSlotState = PINGSLOT_STATE_RX;

            multicastSlotRxConfig.Datarate = PingSlotCtx.Datarate;
            multicastSlotRxConfig.DownlinkDwellTime = LoRaMacClassBParams.LoRaMacParams->DownlinkDwellTime;
            multicastSlotRxConfig.RepeaterSupport = LoRaMacClassBParams.LoRaMacParams->RepeaterSupport;
            multicastSlotRxConfig.Frequency = frequency;
            multicastSlotRxConfig.RxContinuous = false;
            multicastSlotRxConfig.RxSlot = RX_SLOT_WIN_MULTICAST_SLOT;

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

    getPhy.Attribute = PHY_BEACON_FORMAT;
    phyParam = RegionGetPhyParam( *LoRaMacClassBParams.LoRaMacRegion, &getPhy );

    // Verify if we are in the state where we expect a beacon
    if( ( BeaconState == BEACON_STATE_RX ) || ( BeaconCtx.Ctrl.AcquisitionPending == 1 ) )
    {
        if( size == phyParam.BeaconFormat.BeaconSize )
        {
            // A beacon frame is defined as:
            // Bytes: |  x   |  4   |  2   |     7      |  y   |  2   |
            //        |------|------|------|------------|------|------|
            // Field: | RFU1 | Time | CRC1 | GwSpecific | RFU2 | CRC2 |
            //
            // Field RFU1 and RFU2 have variable sizes. It depends on the region specific implementation

            // Read CRC1 field from the frame
            beaconCrc0 = ( ( uint16_t )payload[phyParam.BeaconFormat.Rfu1Size + 4] ) & 0x00FF;
            beaconCrc0 |= ( ( uint16_t )payload[phyParam.BeaconFormat.Rfu1Size + 4 + 1] << 8 ) & 0xFF00;
            crc0 = BeaconCrc( payload, phyParam.BeaconFormat.Rfu1Size + 4 );

            // Validate the first crc of the beacon frame
            if( crc0 == beaconCrc0 )
            {
                // Read Time field from the frame
                BeaconCtx.BeaconTime  = ( ( uint32_t )payload[phyParam.BeaconFormat.Rfu1Size] ) & 0x000000FF;
                BeaconCtx.BeaconTime |= ( ( uint32_t )( payload[phyParam.BeaconFormat.Rfu1Size + 1] << 8 ) ) & 0x0000FF00;
                BeaconCtx.BeaconTime |= ( ( uint32_t )( payload[phyParam.BeaconFormat.Rfu1Size + 2] << 16 ) ) & 0x00FF0000;
                BeaconCtx.BeaconTime |= ( ( uint32_t )( payload[phyParam.BeaconFormat.Rfu1Size + 3] << 24 ) ) & 0xFF000000;
                LoRaMacClassBParams.MlmeIndication->BeaconInfo.Time = BeaconCtx.BeaconTime;
                beaconReceived = true;
            }

            // Read CRC2 field from the frame
            beaconCrc1 = ( ( uint16_t )payload[phyParam.BeaconFormat.Rfu1Size + 4 + 2 + 7 + phyParam.BeaconFormat.Rfu2Size] ) & 0x00FF;
            beaconCrc1 |= ( ( uint16_t )payload[phyParam.BeaconFormat.Rfu1Size + 4 + 2 + 7 + phyParam.BeaconFormat.Rfu2Size + 1] << 8 ) & 0xFF00;
            crc1 = BeaconCrc( &payload[phyParam.BeaconFormat.Rfu1Size + 4 + 2], 7 + phyParam.BeaconFormat.Rfu2Size );

            // Validate the second crc of the beacon frame
            if( crc1 == beaconCrc1 )
            {
                // Read GwSpecific field from the frame
                // The GwSpecific field contains 1 byte InfoDesc and 6 bytes Info
                LoRaMacClassBParams.MlmeIndication->BeaconInfo.GwSpecific.InfoDesc = payload[phyParam.BeaconFormat.Rfu1Size + 4 + 2];
                memcpy1( LoRaMacClassBParams.MlmeIndication->BeaconInfo.GwSpecific.Info, &payload[phyParam.BeaconFormat.Rfu1Size + 4 + 2 + 1], 6 );
                beaconReceived = true;
            }

            // Reset beacon variables, if one of the crc is valid
            if( beaconReceived == true )
            {
                BeaconCtx.LastBeaconRx = TimerGetCurrentTime( ) - Radio.TimeOnAir( MODEM_LORA, size );
                BeaconCtx.Ctrl.BeaconAcquired = 1;
                BeaconCtx.Ctrl.BeaconMode = 1;
                BeaconCtx.SymbolTimeout = BeaconCtx.Cfg.SymbolToDefault;
                PingSlotCtx.SymbolTimeout = BeaconCtx.Cfg.SymbolToDefault;
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
        ( BeaconCtx.Ctrl.AcquisitionTimerSet == 1 ) ||
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

bool LoRaMacClassBIsMulticastExpected( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
    if( MulticastSlotState == PINGSLOT_STATE_RX )
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

bool LoRaMacClassBIsAcquisitionTimerSet( void )
{
#ifdef LORAMAC_CLASSB_ENABLED
    if( BeaconCtx.Ctrl.AcquisitionTimerSet == 1 )
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
    if( LoRaMacClassBIsBeaconModeActive( ) == true )
    {
        if( ( BeaconState == BEACON_STATE_TIMEOUT ) ||
            ( BeaconState == BEACON_STATE_LOST ) )
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
    }
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
        case MIB_PING_SLOT_DATARATE:
        {
            mibGet->Param.PingSlotDatarate = PingSlotCtx.Datarate;
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
        case MIB_PING_SLOT_DATARATE:
        {
            PingSlotCtx.Datarate = mibSet->Param.PingSlotDatarate;
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
    if( LoRaMacConfirmQueueIsCmdActive( MLME_PING_SLOT_INFO ) == true )
    {
        LoRaMacConfirmQueueSetStatus( LORAMAC_EVENT_INFO_STATUS_OK, MLME_PING_SLOT_INFO );
        PingSlotCtx.Ctrl.Assigned = 1;
    }
#endif // LORAMAC_CLASSB_ENABLED
}

uint8_t LoRaMacClassBPingSlotChannelReq( uint8_t datarate, uint32_t frequency )
{
#ifdef LORAMAC_CLASSB_ENABLED
    uint8_t status = 0x03;
    VerifyParams_t verify;
    bool isCustomFreq = false;

    if( frequency != 0 )
    {
        isCustomFreq = true;
        if( Radio.CheckRfFrequency( frequency ) == false )
        {
            status &= 0xFE; // Channel frequency KO
        }
    }

    verify.DatarateParams.Datarate = datarate;
    verify.DatarateParams.DownlinkDwellTime = LoRaMacClassBParams.LoRaMacParams->DownlinkDwellTime;

    if( RegionVerify( *LoRaMacClassBParams.LoRaMacRegion, &verify, PHY_RX_DR ) == false )
    {
        status &= 0xFD; // Datarate range KO
    }

    if( status == 0x03 )
    {
        if( isCustomFreq == true )
        {
            PingSlotCtx.Ctrl.CustomFreq = 1;
            PingSlotCtx.Frequency = frequency;
        }
        else
        {
            PingSlotCtx.Ctrl.CustomFreq = 0;
            PingSlotCtx.Frequency = 0;
        }
        PingSlotCtx.Datarate = datarate;
    }

    return status;
#else
    return 0;
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBBeaconTimingAns( uint16_t beaconTimingDelay, uint8_t beaconTimingChannel )
{
#ifdef LORAMAC_CLASSB_ENABLED
    TimerTime_t currentTime = TimerGetCurrentTime( );

    BeaconCtx.BeaconTimingDelay = ( BeaconCtx.Cfg.DelayBeaconTimingAns * beaconTimingDelay );
    BeaconCtx.BeaconTimingChannel = beaconTimingChannel;

    if( LoRaMacConfirmQueueIsCmdActive( MLME_BEACON_TIMING ) == true )
    {
        if( BeaconCtx.BeaconTimingDelay > BeaconCtx.Cfg.Interval )
        {
            // We missed the beacon already
            BeaconCtx.BeaconTimingDelay = 0;
            BeaconCtx.BeaconTimingChannel = 0;
            LoRaMacConfirmQueueSetStatus( LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND, MLME_BEACON_TIMING );
        }
        else
        {
            BeaconCtx.Ctrl.BeaconDelaySet = 1;
            BeaconCtx.Ctrl.BeaconChannelSet = 1;
            BeaconCtx.NextBeaconRx = currentTime + BeaconCtx.BeaconTimingDelay;
            LoRaMacConfirmQueueSetStatus( LORAMAC_EVENT_INFO_STATUS_OK, MLME_BEACON_TIMING );
        }

        LoRaMacClassBParams.MlmeConfirm->BeaconTimingDelay = BeaconCtx.BeaconTimingDelay;
        LoRaMacClassBParams.MlmeConfirm->BeaconTimingChannel = BeaconCtx.BeaconTimingChannel;
    }
#endif // LORAMAC_CLASSB_ENABLED
}

void LoRaMacClassBDeviceTimeAns( TimerTime_t currentTime )
{
#ifdef LORAMAC_CLASSB_ENABLED

    BeaconCtx.LastBeaconRx = currentTime - ( currentTime % BeaconCtx.Cfg.Interval );
    BeaconCtx.NextBeaconRx = BeaconCtx.LastBeaconRx + BeaconCtx.Cfg.Interval;

    if( LoRaMacConfirmQueueIsCmdActive( MLME_DEVICE_TIME ) == true )
    {
        if( currentTime > BeaconCtx.NextBeaconRx )
        {
            // We missed the beacon already
            BeaconCtx.LastBeaconRx = 0;
            BeaconCtx.NextBeaconRx = 0;
            LoRaMacConfirmQueueSetStatus( LORAMAC_EVENT_INFO_STATUS_BEACON_NOT_FOUND, MLME_DEVICE_TIME );
        }
        else
        {
            BeaconCtx.Ctrl.BeaconDelaySet = 1;
            BeaconCtx.BeaconTimingDelay = BeaconCtx.NextBeaconRx - currentTime;
            BeaconCtx.BeaconTime = BeaconCtx.NextBeaconRx;
            LoRaMacConfirmQueueSetStatus( LORAMAC_EVENT_INFO_STATUS_OK, MLME_DEVICE_TIME );
        }
    }
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
