/*!
 * \file      RegionCommon.c
 *
 * \brief     LoRa MAC common region implementation
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
#include <math.h>
#include "radio.h"
#include "utilities.h"
#include "RegionCommon.h"
#include "systime.h"

#define BACKOFF_DC_1_HOUR                   100
#define BACKOFF_DC_10_HOURS                 1000
#define BACKOFF_DC_24_HOURS                 10000

#define BACKOFF_DUTY_CYCLE_1_HOUR_IN_S      3600
#define BACKOFF_DUTY_CYCLE_10_HOURS_IN_S    ( BACKOFF_DUTY_CYCLE_1_HOUR_IN_S + ( BACKOFF_DUTY_CYCLE_1_HOUR_IN_S * 10 ) )
#define BACKOFF_DUTY_CYCLE_24_HOURS_IN_S    ( BACKOFF_DUTY_CYCLE_10_HOURS_IN_S + ( BACKOFF_DUTY_CYCLE_1_HOUR_IN_S * 24 ) )
#define BACKOFF_24_HOURS_IN_S               ( BACKOFF_DUTY_CYCLE_1_HOUR_IN_S * 24 )

#ifndef DUTY_CYCLE_TIME_PERIOD
/*!
 * Default duty cycle observation time period
 *
 * \remark The ETSI observation time period is 1 hour (3600000 ms) but, the implemented algorithm may violate the
 *         defined duty-cycle restrictions. In order to ensure that these restrictions never get violated we changed the
 *         default duty cycle observation time period to 1/2 hour (1800000 ms).
 */
#define DUTY_CYCLE_TIME_PERIOD              1800000
#endif

/*!
 * \brief Returns `N / D` rounded to the smallest integer value greater than or equal to `N / D`
 *
 * \warning when `D == 0`, the result is undefined
 *
 * \remark `N` and `D` can be signed or unsigned
 *
 * \param [IN] N the numerator, which can have any sign
 * \param [IN] D the denominator, which can have any sign
 * \retval N / D with any fractional part rounded to the smallest integer value greater than or equal to `N / D`
 */
#define DIV_CEIL( N, D )                                                       \
    (                                                                          \
        ( N > 0 ) ?                                                            \
        ( ( ( N ) + ( D ) - 1 ) / ( D ) ) :                                    \
        ( ( N ) / ( D ) )                                                      \
    )

static uint16_t GetDutyCycle( Band_t* band, bool joined, SysTime_t elapsedTimeSinceStartup )
{
    uint16_t dutyCycle = band->DCycle;

    if( joined == false )
    {
        uint16_t joinDutyCycle = BACKOFF_DC_24_HOURS;

        if( elapsedTimeSinceStartup.Seconds < BACKOFF_DUTY_CYCLE_1_HOUR_IN_S )
        {
            joinDutyCycle = BACKOFF_DC_1_HOUR;
        }
        else if( elapsedTimeSinceStartup.Seconds < BACKOFF_DUTY_CYCLE_10_HOURS_IN_S )
        {
            joinDutyCycle = BACKOFF_DC_10_HOURS;
        }
        else
        {
            joinDutyCycle = BACKOFF_DC_24_HOURS;
        }
        // Take the most restrictive duty cycle
        dutyCycle = MAX( dutyCycle, joinDutyCycle );
    }

    // Prevent value of 0
    if( dutyCycle == 0 )
    {
        dutyCycle = 1;
    }

    return dutyCycle;
}

static uint16_t SetMaxTimeCredits( Band_t* band, bool joined, SysTime_t elapsedTimeSinceStartup,
                                   bool dutyCycleEnabled, bool lastTxIsJoinRequest )
{
    uint16_t dutyCycle = band->DCycle;
    TimerTime_t maxCredits = DUTY_CYCLE_TIME_PERIOD;
    TimerTime_t elapsedTime = SysTimeToMs( elapsedTimeSinceStartup );
    SysTime_t timeDiff = { 0 };

    // Get the band duty cycle. If not joined, the function either returns the join duty cycle
    // or the band duty cycle, whichever is more restrictive.
    dutyCycle = GetDutyCycle( band, joined, elapsedTimeSinceStartup );

    if( joined == false )
    {
        if( dutyCycle == BACKOFF_DC_1_HOUR )
        {
            maxCredits = DUTY_CYCLE_TIME_PERIOD;
            band->LastMaxCreditAssignTime = elapsedTime;
        }
        else if( dutyCycle == BACKOFF_DC_10_HOURS )
        {
            maxCredits = DUTY_CYCLE_TIME_PERIOD * 10;
            band->LastMaxCreditAssignTime = elapsedTime;
        }
        else
        {
            maxCredits = DUTY_CYCLE_TIME_PERIOD * 24;
        }

        timeDiff = SysTimeSub( elapsedTimeSinceStartup, SysTimeFromMs( band->LastMaxCreditAssignTime ) );

        // Verify if we have to assign the maximum credits in cases
        // of the preconditions have changed.
        if( ( ( dutyCycleEnabled == false ) && ( lastTxIsJoinRequest == false ) ) ||
            ( band->MaxTimeCredits != maxCredits ) ||
            ( timeDiff.Seconds >= BACKOFF_24_HOURS_IN_S ) )
        {
            band->TimeCredits = maxCredits;

            if( elapsedTimeSinceStartup.Seconds >= BACKOFF_DUTY_CYCLE_24_HOURS_IN_S )
            {
                timeDiff.Seconds = ( elapsedTimeSinceStartup.Seconds - BACKOFF_DUTY_CYCLE_24_HOURS_IN_S ) / BACKOFF_24_HOURS_IN_S;
                timeDiff.Seconds *= BACKOFF_24_HOURS_IN_S;
                timeDiff.Seconds += BACKOFF_DUTY_CYCLE_24_HOURS_IN_S;
                timeDiff.SubSeconds = 0;
                band->LastMaxCreditAssignTime = SysTimeToMs( timeDiff );
            }
        }
    }
    else
    {
        if( dutyCycleEnabled == false )
        {
            // Assign max credits when the duty cycle is disabled.
            band->TimeCredits = maxCredits;
        }
    }

    // Assign the max credits if its the first time
    if( band->LastBandUpdateTime == 0 )
    {
        band->TimeCredits = maxCredits;
    }

    // Setup the maximum allowed credits. We can assign them
    // safely all the time.
    band->MaxTimeCredits = maxCredits;

    return dutyCycle;
}

static uint16_t UpdateTimeCredits( Band_t* band, bool joined, bool dutyCycleEnabled,
                                   bool lastTxIsJoinRequest, SysTime_t elapsedTimeSinceStartup,
                                   TimerTime_t currentTime )
{
    uint16_t dutyCycle = SetMaxTimeCredits( band, joined, elapsedTimeSinceStartup,
                                            dutyCycleEnabled, lastTxIsJoinRequest );

    if( joined == true )
    {
        // Apply a sliding window for the duty cycle with collection and speding
        // credits.
        band->TimeCredits += TimerGetElapsedTime( band->LastBandUpdateTime );
    }

    // Limit band credits to maximum
    if( band->TimeCredits > band->MaxTimeCredits )
    {
        band->TimeCredits = band->MaxTimeCredits;
    }

    // Synchronize update time
    band->LastBandUpdateTime = currentTime;

    return dutyCycle;
}

static uint8_t CountChannels( uint16_t mask, uint8_t nbBits )
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

bool RegionCommonChanVerifyDr( uint8_t nbChannels, uint16_t* channelsMask, int8_t dr, int8_t minDr, int8_t maxDr, ChannelParams_t* channels )
{
    if( RegionCommonValueInRange( dr, minDr, maxDr ) == 0 )
    {
        return false;
    }

    for( uint8_t i = 0, k = 0; i < nbChannels; i += 16, k++ )
    {
        for( uint8_t j = 0; j < 16; j++ )
        {
            if( ( ( channelsMask[k] & ( 1 << j ) ) != 0 ) )
            {// Check datarate validity for enabled channels
                if( RegionCommonValueInRange( dr, ( channels[i + j].DrRange.Fields.Min & 0x0F ),
                                                  ( channels[i + j].DrRange.Fields.Max & 0x0F ) ) == 1 )
                {
                    // At least 1 channel has been found we can return OK.
                    return true;
                }
            }
        }
    }
    return false;
}

uint8_t RegionCommonValueInRange( int8_t value, int8_t min, int8_t max )
{
    if( ( value >= min ) && ( value <= max ) )
    {
        return 1;
    }
    return 0;
}

bool RegionCommonChanDisable( uint16_t* channelsMask, uint8_t id, uint8_t maxChannels )
{
    uint8_t index = id / 16;

    if( ( index > ( maxChannels / 16 ) ) || ( id >= maxChannels ) )
    {
        return false;
    }

    // Deactivate channel
    channelsMask[index] &= ~( 1 << ( id % 16 ) );

    return true;
}

uint8_t RegionCommonCountChannels( uint16_t* channelsMask, uint8_t startIdx, uint8_t stopIdx )
{
    uint8_t nbChannels = 0;

    if( channelsMask == NULL )
    {
        return 0;
    }

    for( uint8_t i = startIdx; i < stopIdx; i++ )
    {
        nbChannels += CountChannels( channelsMask[i], 16 );
    }

    return nbChannels;
}

void RegionCommonChanMaskCopy( uint16_t* channelsMaskDest, uint16_t* channelsMaskSrc, uint8_t len )
{
    if( ( channelsMaskDest != NULL ) && ( channelsMaskSrc != NULL ) )
    {
        for( uint8_t i = 0; i < len; i++ )
        {
            channelsMaskDest[i] = channelsMaskSrc[i];
        }
    }
}

void RegionCommonSetBandTxDone( Band_t* band, TimerTime_t lastTxAirTime, bool joined, SysTime_t elapsedTimeSinceStartup )
{
    // Get the band duty cycle. If not joined, the function either returns the join duty cycle
    // or the band duty cycle, whichever is more restrictive.
    uint16_t dutyCycle = GetDutyCycle( band, joined, elapsedTimeSinceStartup );

    // Reduce with transmission time
    if( band->TimeCredits > ( lastTxAirTime * dutyCycle ) )
    {
        // Reduce time credits by the time of air
        band->TimeCredits -= ( lastTxAirTime * dutyCycle );
    }
    else
    {
        band->TimeCredits = 0;
    }
}

TimerTime_t RegionCommonUpdateBandTimeOff( bool joined, Band_t* bands,
                                           uint8_t nbBands, bool dutyCycleEnabled,
                                           bool lastTxIsJoinRequest, SysTime_t elapsedTimeSinceStartup,
                                           TimerTime_t expectedTimeOnAir )
{
    TimerTime_t minTimeToWait = TIMERTIME_T_MAX;
    TimerTime_t currentTime = TimerGetCurrentTime( );
    TimerTime_t creditCosts = 0;
    uint16_t dutyCycle = 1;
    uint8_t validBands = 0;

    for( uint8_t i = 0; i < nbBands; i++ )
    {
        // Synchronization of bands and credits
        dutyCycle = UpdateTimeCredits( &bands[i], joined, dutyCycleEnabled,
                                       lastTxIsJoinRequest, elapsedTimeSinceStartup,
                                       currentTime );

        // Calculate the credit costs for the next transmission
        // with the duty cycle and the expected time on air
        creditCosts = expectedTimeOnAir * dutyCycle;

        // Check if the band is ready for transmission. Its ready,
        // when the duty cycle is off, or the TimeCredits of the band
        // is higher than the credit costs for the transmission.
        if( ( bands[i].TimeCredits > creditCosts ) ||
            ( ( dutyCycleEnabled == false ) && ( joined == true ) ) )
        {
            bands[i].ReadyForTransmission = true;
            // This band is a potential candidate for an
            // upcoming transmission, so increase the counter.
            validBands++;
        }
        else
        {
            // In this case, the band has not enough credits
            // for the next transmission.
            bands[i].ReadyForTransmission = false;

            if( bands[i].MaxTimeCredits > creditCosts )
            {
                // The band can only be taken into account, if the maximum credits
                // of the band are higher than the credit costs.
                // We calculate the minTimeToWait among the bands which are not
                // ready for transmission and which are potentially available
                // for a transmission in the future.
                minTimeToWait = MIN( minTimeToWait, ( creditCosts - bands[i].TimeCredits ) );
                // This band is a potential candidate for an
                // upcoming transmission (even if its time credits are not enough
                // at the moment), so increase the counter.
                validBands++;
            }

            // Apply a special calculation if the device is not joined.
            if( joined == false )
            {
                SysTime_t backoffTimeRange = {
                    .Seconds    = 0,
                    .SubSeconds = 0,
                };
                // Get the backoff time range based on the duty cycle definition
                if( dutyCycle == BACKOFF_DC_1_HOUR )
                {
                    backoffTimeRange.Seconds = BACKOFF_DUTY_CYCLE_1_HOUR_IN_S;
                }
                else if( dutyCycle == BACKOFF_DC_10_HOURS )
                {
                    backoffTimeRange.Seconds = BACKOFF_DUTY_CYCLE_10_HOURS_IN_S;
                }
                else
                {
                    backoffTimeRange.Seconds = BACKOFF_DUTY_CYCLE_24_HOURS_IN_S;
                }
                // Calculate the time to wait.
                if( elapsedTimeSinceStartup.Seconds > BACKOFF_DUTY_CYCLE_24_HOURS_IN_S )
                {
                    backoffTimeRange.Seconds += BACKOFF_24_HOURS_IN_S * ( ( ( elapsedTimeSinceStartup.Seconds - BACKOFF_DUTY_CYCLE_24_HOURS_IN_S ) / BACKOFF_24_HOURS_IN_S ) + 1 );
                }
                // Calculate the time difference between now and the next range
                backoffTimeRange  = SysTimeSub( backoffTimeRange, elapsedTimeSinceStartup );
                minTimeToWait = SysTimeToMs( backoffTimeRange );
            }
        }
    }


    if( validBands == 0 )
    {
        // There is no valid band available to handle a transmission
        // in the given DUTY_CYCLE_TIME_PERIOD.
        return TIMERTIME_T_MAX;
    }
    return minTimeToWait;
}

uint8_t RegionCommonParseLinkAdrReq( uint8_t* payload, RegionCommonLinkAdrParams_t* linkAdrParams )
{
    uint8_t retIndex = 0;

    if( payload[0] == SRV_MAC_LINK_ADR_REQ )
    {
        // Parse datarate and tx power
        linkAdrParams->Datarate = payload[1];
        linkAdrParams->TxPower = linkAdrParams->Datarate & 0x0F;
        linkAdrParams->Datarate = ( linkAdrParams->Datarate >> 4 ) & 0x0F;
        // Parse ChMask
        linkAdrParams->ChMask = ( uint16_t )payload[2];
        linkAdrParams->ChMask |= ( uint16_t )payload[3] << 8;
        // Parse ChMaskCtrl and nbRep
        linkAdrParams->NbRep = payload[4];
        linkAdrParams->ChMaskCtrl = ( linkAdrParams->NbRep >> 4 ) & 0x07;
        linkAdrParams->NbRep &= 0x0F;

        // LinkAdrReq has 4 bytes length + 1 byte CMD
        retIndex = 5;
    }
    return retIndex;
}

uint8_t RegionCommonLinkAdrReqVerifyParams( RegionCommonLinkAdrReqVerifyParams_t* verifyParams, int8_t* dr, int8_t* txPow, uint8_t* nbRep )
{
    uint8_t status = verifyParams->Status;
    int8_t datarate = verifyParams->Datarate;
    int8_t txPower = verifyParams->TxPower;
    int8_t nbRepetitions = verifyParams->NbRep;

    // Handle the case when ADR is off.
    if( verifyParams->AdrEnabled == false )
    {
        // When ADR is off, we are allowed to change the channels mask
        nbRepetitions = verifyParams->CurrentNbRep;
        datarate =  verifyParams->CurrentDatarate;
        txPower =  verifyParams->CurrentTxPower;
    }

    if( status != 0 )
    {
        // Verify datarate. The variable phyParam. Value contains the minimum allowed datarate.
        if( datarate == 0x0F )
        { // 0xF means that the device MUST ignore that field, and keep the current parameter value.
            datarate =  verifyParams->CurrentDatarate;
        }
        else if( RegionCommonChanVerifyDr( verifyParams->NbChannels, verifyParams->ChannelsMask, datarate,
                                      verifyParams->MinDatarate, verifyParams->MaxDatarate, verifyParams->Channels  ) == false )
        {
            status &= 0xFD; // Datarate KO
        }

        // Verify tx power
        if( txPower == 0x0F )
        { // 0xF means that the device MUST ignore that field, and keep the current parameter value.
            txPower =  verifyParams->CurrentTxPower;
        }
        else if( RegionCommonValueInRange( txPower, verifyParams->MaxTxPower, verifyParams->MinTxPower ) == 0 )
        {
            // Verify if the maximum TX power is exceeded
            if( verifyParams->MaxTxPower > txPower )
            { // Apply maximum TX power. Accept TX power.
                txPower = verifyParams->MaxTxPower;
            }
            else
            {
                status &= 0xFB; // TxPower KO
            }
        }
    }

    // If the status is ok, verify the NbRep
    if( status == 0x07 )
    {
        if( nbRepetitions == 0 )
        { // Set nbRep to the default value of 1.
            nbRepetitions = 1;
        }
    }

    // Apply changes
    *dr = datarate;
    *txPow = txPower;
    *nbRep = nbRepetitions;

    return status;
}

uint32_t RegionCommonComputeSymbolTimeLoRa( uint8_t phyDr, uint32_t bandwidthInHz )
{
    return ( 1 << phyDr ) * 1000000 / bandwidthInHz;
}

uint32_t RegionCommonComputeSymbolTimeFsk( uint8_t phyDrInKbps )
{
    return 8000 / ( uint32_t )phyDrInKbps; // 1 symbol equals 1 byte
}

void RegionCommonComputeRxWindowParameters( uint32_t tSymbolInUs, uint8_t minRxSymbols, uint32_t rxErrorInMs, uint32_t wakeUpTimeInMs, uint32_t* windowTimeoutInSymbols, int32_t* windowOffsetInMs )
{
    *windowTimeoutInSymbols = MAX( DIV_CEIL( ( ( 2 * minRxSymbols - 8 ) * tSymbolInUs + 2 * ( rxErrorInMs * 1000 ) ),  tSymbolInUs ), minRxSymbols ); // Computed number of symbols
    *windowOffsetInMs = ( int32_t )DIV_CEIL( ( int32_t )( 4 * tSymbolInUs ) -
                                               ( int32_t )DIV_CEIL( ( *windowTimeoutInSymbols * tSymbolInUs ), 2 ) -
                                               ( int32_t )( wakeUpTimeInMs * 1000 ), 1000 );
}

int8_t RegionCommonComputeTxPower( int8_t txPowerIndex, float maxEirp, float antennaGain )
{
    int8_t phyTxPower = 0;

    phyTxPower = ( int8_t )floor( ( maxEirp - ( txPowerIndex * 2U ) ) - antennaGain );

    return phyTxPower;
}

void RegionCommonRxBeaconSetup( RegionCommonRxBeaconSetupParams_t* rxBeaconSetupParams )
{
    bool rxContinuous = true;
    uint8_t datarate;

    // Set the radio into sleep mode
    Radio.Sleep( );

    // Setup frequency and payload length
    Radio.SetChannel( rxBeaconSetupParams->Frequency );
    Radio.SetMaxPayloadLength( MODEM_LORA, rxBeaconSetupParams->BeaconSize );

    // Check the RX continuous mode
    if( rxBeaconSetupParams->RxTime != 0 )
    {
        rxContinuous = false;
    }

    // Get region specific datarate
    datarate = rxBeaconSetupParams->Datarates[rxBeaconSetupParams->BeaconDatarate];

    // Setup radio
    Radio.SetRxConfig( MODEM_LORA, rxBeaconSetupParams->BeaconChannelBW, datarate,
                       1, 0, 10, rxBeaconSetupParams->SymbolTimeout, true, rxBeaconSetupParams->BeaconSize, false, 0, 0, false, rxContinuous );

    Radio.Rx( rxBeaconSetupParams->RxTime );
}

void RegionCommonCountNbOfEnabledChannels( RegionCommonCountNbOfEnabledChannelsParams_t* countNbOfEnabledChannelsParams,
                                           uint8_t* enabledChannels, uint8_t* nbEnabledChannels, uint8_t* nbRestrictedChannels )
{
    uint8_t nbChannelCount = 0;
    uint8_t nbRestrictedChannelsCount = 0;

    for( uint8_t i = 0, k = 0; i < countNbOfEnabledChannelsParams->MaxNbChannels; i += 16, k++ )
    {
        for( uint8_t j = 0; j < 16; j++ )
        {
            if( ( countNbOfEnabledChannelsParams->ChannelsMask[k] & ( 1 << j ) ) != 0 )
            {
                if( countNbOfEnabledChannelsParams->Channels[i + j].Frequency == 0 )
                { // Check if the channel is enabled
                    continue;
                }
                if( ( countNbOfEnabledChannelsParams->Joined == false ) &&
                    ( countNbOfEnabledChannelsParams->JoinChannels != NULL ) )
                {
                    if( ( countNbOfEnabledChannelsParams->JoinChannels[k] & ( 1 << j ) ) == 0 )
                    {
                        continue;
                    }
                }
                if( RegionCommonValueInRange( countNbOfEnabledChannelsParams->Datarate,
                                              countNbOfEnabledChannelsParams->Channels[i + j].DrRange.Fields.Min,
                                              countNbOfEnabledChannelsParams->Channels[i + j].DrRange.Fields.Max ) == false )
                { // Check if the current channel selection supports the given datarate
                    continue;
                }
                if( countNbOfEnabledChannelsParams->Bands[countNbOfEnabledChannelsParams->Channels[i + j].Band].ReadyForTransmission == false )
                { // Check if the band is available for transmission
                    nbRestrictedChannelsCount++;
                    continue;
                }
                enabledChannels[nbChannelCount++] = i + j;
            }
        }
    }
    *nbEnabledChannels = nbChannelCount;
    *nbRestrictedChannels = nbRestrictedChannelsCount;
}

LoRaMacStatus_t RegionCommonIdentifyChannels( RegionCommonIdentifyChannelsParam_t* identifyChannelsParam,
                                              TimerTime_t* aggregatedTimeOff, uint8_t* enabledChannels,
                                              uint8_t* nbEnabledChannels, uint8_t* nbRestrictedChannels,
                                              TimerTime_t* nextTxDelay )
{
    TimerTime_t elapsed = TimerGetElapsedTime( identifyChannelsParam->LastAggrTx );
    *nextTxDelay = identifyChannelsParam->AggrTimeOff - elapsed;
    *nbRestrictedChannels = 1;
    *nbEnabledChannels = 0;

    if( ( identifyChannelsParam->LastAggrTx == 0 ) ||
        ( identifyChannelsParam->AggrTimeOff <= elapsed ) )
    {
        // Reset Aggregated time off
        *aggregatedTimeOff = 0;

        // Update bands Time OFF
        *nextTxDelay = RegionCommonUpdateBandTimeOff( identifyChannelsParam->CountNbOfEnabledChannelsParam->Joined,
                                                      identifyChannelsParam->CountNbOfEnabledChannelsParam->Bands,
                                                      identifyChannelsParam->MaxBands,
                                                      identifyChannelsParam->DutyCycleEnabled,
                                                      identifyChannelsParam->LastTxIsJoinRequest,
                                                      identifyChannelsParam->ElapsedTimeSinceStartUp,
                                                      identifyChannelsParam->ExpectedTimeOnAir );

        RegionCommonCountNbOfEnabledChannels( identifyChannelsParam->CountNbOfEnabledChannelsParam, enabledChannels,
                                              nbEnabledChannels, nbRestrictedChannels );
    }

    if( *nbEnabledChannels > 0 )
    {
        *nextTxDelay = 0;
        return LORAMAC_STATUS_OK;
    }
    else if( *nbRestrictedChannels > 0 )
    {
        return LORAMAC_STATUS_DUTYCYCLE_RESTRICTED;
    }
    else
    {
        return LORAMAC_STATUS_NO_CHANNEL_FOUND;
    }
}

int8_t RegionCommonGetNextLowerTxDr( RegionCommonGetNextLowerTxDrParams_t *params )
{
    int8_t drLocal = params->CurrentDr;

    if( params->CurrentDr == params->MinDr )
    {
        return params->MinDr;
    }
    else
    {
        do
        {
            drLocal = ( drLocal - 1 );
        } while( ( drLocal != params->MinDr ) &&
                 ( RegionCommonChanVerifyDr( params->NbChannels, params->ChannelsMask, drLocal, params->MinDr, params->MaxDr, params->Channels  ) == false ) );

        return drLocal;
    }
}

int8_t RegionCommonLimitTxPower( int8_t txPower, int8_t maxBandTxPower )
{
    // Limit tx power to the band max
    return MAX( txPower, maxBandTxPower );
}

uint32_t RegionCommonGetBandwidth( uint32_t drIndex, const uint32_t* bandwidths )
{
    switch( bandwidths[drIndex] )
    {
        default:
        case 125000:
            return 0;
        case 250000:
            return 1;
        case 500000:
            return 2;
    }
}
