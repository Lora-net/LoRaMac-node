/*!
 * \file      sx1276.c
 *
 * \brief     SX1276 driver implementation
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
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Wael Guibene ( Semtech )
 */
#include <math.h>
#include <string.h>
#include "utilities.h"
#include "timer.h"
#include "radio.h"
#include "delay.h"
#include "sx1276.h"
#include "sx1276-board.h"

/*!
 * \brief Internal frequency of the radio
 */
#define SX1276_XTAL_FREQ                            32000000UL

/*!
 * \brief Scaling factor used to perform fixed-point operations
 */
#define SX1276_PLL_STEP_SHIFT_AMOUNT                ( 8 )

/*!
 * \brief PLL step - scaled with SX1276_PLL_STEP_SHIFT_AMOUNT
 */
#define SX1276_PLL_STEP_SCALED                      ( SX1276_XTAL_FREQ >> ( 19 - SX1276_PLL_STEP_SHIFT_AMOUNT ) )

/*!
 * \brief Radio buffer size
 */
#define RX_TX_BUFFER_SIZE                           256

/*
 * Local types definition
 */

/*!
 * Radio registers definition
 */
typedef struct
{
    RadioModems_t Modem;
    uint8_t       Addr;
    uint8_t       Value;
}RadioRegisters_t;

/*!
 * FSK bandwidth definition
 */
typedef struct
{
    uint32_t bandwidth;
    uint8_t  RegValue;
}FskBandwidth_t;


/*
 * Private functions prototypes
 */

/*!
 * Performs the Rx chain calibration for LF and HF bands
 * \remark Must be called just after the reset so all registers are at their
 *         default values
 */
static void RxChainCalibration( void );

/*!
 * \brief Sets the SX1276 in transmission mode for the given time
 * \param [IN] timeout Transmission timeout [ms] [0: continuous, others timeout]
 */
static void SX1276SetTx( uint32_t timeout );

/*!
 * \brief Writes the buffer contents to the SX1276 FIFO
 *
 * \param [IN] buffer Buffer containing data to be put on the FIFO.
 * \param [IN] size Number of bytes to be written to the FIFO
 */
static void SX1276WriteFifo( uint8_t *buffer, uint8_t size );

/*!
 * \brief Reads the contents of the SX1276 FIFO
 *
 * \param [OUT] buffer Buffer where to copy the FIFO read data.
 * \param [IN] size Number of bytes to be read from the FIFO
 */
static void SX1276ReadFifo( uint8_t *buffer, uint8_t size );

/*!
 * \brief Sets the SX1276 operating mode
 *
 * \param [IN] opMode New operating mode
 */
static void SX1276SetOpMode( uint8_t opMode );

/*!
 * \brief Get frequency in Hertz for a given number of PLL steps
 *
 * \param [in] pllSteps Number of PLL steps
 *
 * \returns Frequency in Hertz
 */
static uint32_t SX1276ConvertPllStepToFreqInHz( uint32_t pllSteps );

/*!
 * \brief Get the number of PLL steps for a given frequency in Hertz
 *
 * \param [in] freqInHz Frequency in Hertz
 *
 * \returns Number of PLL steps
 */
static uint32_t SX1276ConvertFreqInHzToPllStep( uint32_t freqInHz );

/*!
 * \brief Get the parameter corresponding to a FSK Rx bandwith immediately above the minimum requested one.
 *
 * \param [in] bw Minimum required bandwith in Hz
 *
 * \returns parameter
 */
static uint8_t GetFskBandwidthRegValue( uint32_t bw );

/*!
 * \brief Get the actual value in Hertz of a given LoRa bandwidth
 *
 * \param [in] bw LoRa bandwidth parameter
 *
 * \returns Actual LoRa bandwidth in Hertz
 */
static uint32_t SX1276GetLoRaBandwidthInHz( uint32_t bw );

/*!
 * Compute the numerator for GFSK time-on-air computation.
 *
 * \remark To get the actual time-on-air in second, this value has to be divided by the GFSK bitrate in bits per
 * second.
 *
 * \param [in] preambleLen 
 * \param [in] fixLen 
 * \param [in] payloadLen 
 * \param [in] crcOn 
 *
 * \returns GFSK time-on-air numerator
 */
static uint32_t SX1276GetGfskTimeOnAirNumerator( uint16_t preambleLen, bool fixLen,
                                                 uint8_t payloadLen, bool crcOn );

/*!
 * Compute the numerator for LoRa time-on-air computation.
 *
 * \remark To get the actual time-on-air in second, this value has to be divided by the LoRa bandwidth in Hertz.
 *
 * \param [in] bandwidth 
 * \param [in] datarate 
 * \param [in] coderate 
 * \param [in] preambleLen 
 * \param [in] fixLen 
 * \param [in] payloadLen 
 * \param [in] crcOn 
 *
 * \returns LoRa time-on-air numerator
 */
static uint32_t SX1276GetLoRaTimeOnAirNumerator( uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                              bool crcOn );

/*
 * SX1276 DIO IRQ callback functions prototype
 */

/*!
 * \brief DIO 0 IRQ callback
 */
static void SX1276OnDio0Irq( void* context );

/*!
 * \brief DIO 1 IRQ callback
 */
static void SX1276OnDio1Irq( void* context );

/*!
 * \brief DIO 2 IRQ callback
 */
static void SX1276OnDio2Irq( void* context );

/*!
 * \brief DIO 3 IRQ callback
 */
static void SX1276OnDio3Irq( void* context );

/*!
 * \brief DIO 4 IRQ callback
 */
static void SX1276OnDio4Irq( void* context );

/*!
 * \brief Tx & Rx timeout timer callback
 */
static void SX1276OnTimeoutIrq( void* context );

/*
 * Private global constants
 */

/*!
 * Radio hardware registers initialization
 *
 * \remark RADIO_INIT_REGISTERS_VALUE is defined in sx1276-board.h file
 */
const RadioRegisters_t RadioRegsInit[] = RADIO_INIT_REGISTERS_VALUE;

/*!
 * Constant values need to compute the RSSI value
 */
#define RSSI_OFFSET_LF                              -164
#define RSSI_OFFSET_HF                              -157

/*!
 * Precomputed FSK bandwidth registers values
 */
const FskBandwidth_t FskBandwidths[] =
{
    { 2600  , 0x17 },
    { 3100  , 0x0F },
    { 3900  , 0x07 },
    { 5200  , 0x16 },
    { 6300  , 0x0E },
    { 7800  , 0x06 },
    { 10400 , 0x15 },
    { 12500 , 0x0D },
    { 15600 , 0x05 },
    { 20800 , 0x14 },
    { 25000 , 0x0C },
    { 31300 , 0x04 },
    { 41700 , 0x13 },
    { 50000 , 0x0B },
    { 62500 , 0x03 },
    { 83333 , 0x12 },
    { 100000, 0x0A },
    { 125000, 0x02 },
    { 166700, 0x11 },
    { 200000, 0x09 },
    { 250000, 0x01 },
    { 300000, 0x00 }, // Invalid Bandwidth
};

/*
 * Private global variables
 */

/*!
 * Radio callbacks variable
 */
static RadioEvents_t *RadioEvents;

/*!
 * Reception buffer
 */
static uint8_t RxTxBuffer[RX_TX_BUFFER_SIZE];

/*
 * Public global variables
 */

/*!
 * Radio hardware and global parameters
 */
SX1276_t SX1276;

/*!
 * Hardware DIO IRQ callback initialization
 */
DioIrqHandler *DioIrq[] = { SX1276OnDio0Irq, SX1276OnDio1Irq,
                            SX1276OnDio2Irq, SX1276OnDio3Irq,
                            SX1276OnDio4Irq, NULL };

/*!
 * Tx and Rx timers
 */
TimerEvent_t TxTimeoutTimer;
TimerEvent_t RxTimeoutTimer;
TimerEvent_t RxTimeoutSyncWord;

/*
 * Radio driver functions implementation
 */

void SX1276Init( RadioEvents_t *events )
{
    uint8_t i;

    RadioEvents = events;

    // Initialize driver timeout timers
    TimerInit( &TxTimeoutTimer, SX1276OnTimeoutIrq );
    TimerInit( &RxTimeoutTimer, SX1276OnTimeoutIrq );
    TimerInit( &RxTimeoutSyncWord, SX1276OnTimeoutIrq );

    SX1276Reset( );

    RxChainCalibration( );

    SX1276SetOpMode( RF_OPMODE_SLEEP );

    SX1276IoIrqInit( DioIrq );

    for( i = 0; i < sizeof( RadioRegsInit ) / sizeof( RadioRegisters_t ); i++ )
    {
        SX1276SetModem( RadioRegsInit[i].Modem );
        SX1276Write( RadioRegsInit[i].Addr, RadioRegsInit[i].Value );
    }

    SX1276SetModem( MODEM_FSK );

    SX1276.Settings.State = RF_IDLE;
}

RadioState_t SX1276GetStatus( void )
{
    return SX1276.Settings.State;
}

void SX1276SetChannel( uint32_t freq )
{
    uint32_t freqInPllSteps = SX1276ConvertFreqInHzToPllStep( freq );

    SX1276.Settings.Channel = freq;

    SX1276Write( REG_FRFMSB, ( uint8_t )( ( freqInPllSteps >> 16 ) & 0xFF ) );
    SX1276Write( REG_FRFMID, ( uint8_t )( ( freqInPllSteps >> 8 ) & 0xFF ) );
    SX1276Write( REG_FRFLSB, ( uint8_t )( freqInPllSteps & 0xFF ) );
}

bool SX1276IsChannelFree( uint32_t freq, uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime )
{
    bool status = true;
    int16_t rssi = 0;
    uint32_t carrierSenseTime = 0;

    SX1276SetSleep( );

    SX1276SetModem( MODEM_FSK );

    SX1276SetChannel( freq );

    SX1276Write( REG_RXBW, GetFskBandwidthRegValue( rxBandwidth ) );
    SX1276Write( REG_AFCBW, GetFskBandwidthRegValue( rxBandwidth ) );

    SX1276SetOpMode( RF_OPMODE_RECEIVER );

    DelayMs( 1 );

    carrierSenseTime = TimerGetCurrentTime( );

    // Perform carrier sense for maxCarrierSenseTime
    while( TimerGetElapsedTime( carrierSenseTime ) < maxCarrierSenseTime )
    {
        rssi = SX1276ReadRssi( MODEM_FSK );

        if( rssi > rssiThresh )
        {
            status = false;
            break;
        }
    }
    SX1276SetSleep( );
    return status;
}

uint32_t SX1276Random( void )
{
    uint8_t i;
    uint32_t rnd = 0;

    /*
     * Radio setup for random number generation
     */
    // Set LoRa modem ON
    SX1276SetModem( MODEM_LORA );

    // Disable LoRa modem interrupts
    SX1276Write( REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
                  RFLR_IRQFLAGS_RXDONE |
                  RFLR_IRQFLAGS_PAYLOADCRCERROR |
                  RFLR_IRQFLAGS_VALIDHEADER |
                  RFLR_IRQFLAGS_TXDONE |
                  RFLR_IRQFLAGS_CADDONE |
                  RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                  RFLR_IRQFLAGS_CADDETECTED );

    // Set radio in continuous reception
    SX1276SetOpMode( RF_OPMODE_RECEIVER );

    for( i = 0; i < 32; i++ )
    {
        DelayMs( 1 );
        // Unfiltered RSSI value reading. Only takes the LSB value
        rnd |= ( ( uint32_t )SX1276Read( REG_LR_RSSIWIDEBAND ) & 0x01 ) << i;
    }

    SX1276SetSleep( );

    return rnd;
}

/*!
 * Performs the Rx chain calibration for LF and HF bands
 * \remark Must be called just after the reset so all registers are at their
 *         default values
 */
static void RxChainCalibration( void )
{
    uint8_t regPaConfigInitVal;
    uint32_t initialFreq;

    // Save context
    regPaConfigInitVal = SX1276Read( REG_PACONFIG );

    initialFreq = SX1276ConvertPllStepToFreqInHz( ( ( ( uint32_t )SX1276Read( REG_FRFMSB ) << 16 ) |
                                                    ( ( uint32_t )SX1276Read( REG_FRFMID ) << 8 ) |
                                                    ( ( uint32_t )SX1276Read( REG_FRFLSB ) ) ) );

    // Cut the PA just in case, RFO output, power = -1 dBm
    SX1276Write( REG_PACONFIG, 0x00 );

    // Launch Rx chain calibration for LF band
    SX1276Write( REG_IMAGECAL, ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_MASK ) | RF_IMAGECAL_IMAGECAL_START );
    while( ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_RUNNING ) == RF_IMAGECAL_IMAGECAL_RUNNING )
    {
    }

    // Sets a Frequency in HF band
    SX1276SetChannel( 868000000 );

    // Launch Rx chain calibration for HF band
    SX1276Write( REG_IMAGECAL, ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_MASK ) | RF_IMAGECAL_IMAGECAL_START );
    while( ( SX1276Read( REG_IMAGECAL ) & RF_IMAGECAL_IMAGECAL_RUNNING ) == RF_IMAGECAL_IMAGECAL_RUNNING )
    {
    }

    // Restore context
    SX1276Write( REG_PACONFIG, regPaConfigInitVal );
    SX1276SetChannel( initialFreq );
}

void SX1276SetRxConfig( RadioModems_t modem, uint32_t bandwidth,
                         uint32_t datarate, uint8_t coderate,
                         uint32_t bandwidthAfc, uint16_t preambleLen,
                         uint16_t symbTimeout, bool fixLen,
                         uint8_t payloadLen,
                         bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                         bool iqInverted, bool rxContinuous )
{
    SX1276SetModem( modem );

    SX1276SetStby( );

    switch( modem )
    {
    case MODEM_FSK:
        {
            SX1276.Settings.Fsk.Bandwidth = bandwidth;
            SX1276.Settings.Fsk.Datarate = datarate;
            SX1276.Settings.Fsk.BandwidthAfc = bandwidthAfc;
            SX1276.Settings.Fsk.FixLen = fixLen;
            SX1276.Settings.Fsk.PayloadLen = payloadLen;
            SX1276.Settings.Fsk.CrcOn = crcOn;
            SX1276.Settings.Fsk.IqInverted = iqInverted;
            SX1276.Settings.Fsk.RxContinuous = rxContinuous;
            SX1276.Settings.Fsk.PreambleLen = preambleLen;
            SX1276.Settings.Fsk.RxSingleTimeout = ( uint32_t )symbTimeout * 8000UL / datarate;

            uint32_t bitRate = ( uint32_t )( SX1276_XTAL_FREQ / datarate );
            SX1276Write( REG_BITRATEMSB, ( uint8_t )( bitRate >> 8 ) );
            SX1276Write( REG_BITRATELSB, ( uint8_t )( bitRate & 0xFF ) );

            SX1276Write( REG_RXBW, GetFskBandwidthRegValue( bandwidth ) );
            SX1276Write( REG_AFCBW, GetFskBandwidthRegValue( bandwidthAfc ) );

            SX1276Write( REG_PREAMBLEMSB, ( uint8_t )( ( preambleLen >> 8 ) & 0xFF ) );
            SX1276Write( REG_PREAMBLELSB, ( uint8_t )( preambleLen & 0xFF ) );

            if( fixLen == 1 )
            {
                SX1276Write( REG_PAYLOADLENGTH, payloadLen );
            }
            else
            {
                SX1276Write( REG_PAYLOADLENGTH, 0xFF ); // Set payload length to the maximum
            }

            SX1276Write( REG_PACKETCONFIG1,
                         ( SX1276Read( REG_PACKETCONFIG1 ) &
                           RF_PACKETCONFIG1_CRC_MASK &
                           RF_PACKETCONFIG1_PACKETFORMAT_MASK ) |
                           ( ( fixLen == 1 ) ? RF_PACKETCONFIG1_PACKETFORMAT_FIXED : RF_PACKETCONFIG1_PACKETFORMAT_VARIABLE ) |
                           ( crcOn << 4 ) );
            SX1276Write( REG_PACKETCONFIG2, ( SX1276Read( REG_PACKETCONFIG2 ) | RF_PACKETCONFIG2_DATAMODE_PACKET ) );
        }
        break;
    case MODEM_LORA:
        {
            if( bandwidth > 2 )
            {
                // Fatal error: When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported
                while( 1 );
            }
            bandwidth += 7;
            SX1276.Settings.LoRa.Bandwidth = bandwidth;
            SX1276.Settings.LoRa.Datarate = datarate;
            SX1276.Settings.LoRa.Coderate = coderate;
            SX1276.Settings.LoRa.PreambleLen = preambleLen;
            SX1276.Settings.LoRa.FixLen = fixLen;
            SX1276.Settings.LoRa.PayloadLen = payloadLen;
            SX1276.Settings.LoRa.CrcOn = crcOn;
            SX1276.Settings.LoRa.FreqHopOn = freqHopOn;
            SX1276.Settings.LoRa.HopPeriod = hopPeriod;
            SX1276.Settings.LoRa.IqInverted = iqInverted;
            SX1276.Settings.LoRa.RxContinuous = rxContinuous;

            if( datarate > 12 )
            {
                datarate = 12;
            }
            else if( datarate < 6 )
            {
                datarate = 6;
            }

            if( ( ( bandwidth == 7 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
                ( ( bandwidth == 8 ) && ( datarate == 12 ) ) )
            {
                SX1276.Settings.LoRa.LowDatarateOptimize = 0x01;
            }
            else
            {
                SX1276.Settings.LoRa.LowDatarateOptimize = 0x00;
            }

            SX1276Write( REG_LR_MODEMCONFIG1,
                         ( SX1276Read( REG_LR_MODEMCONFIG1 ) &
                           RFLR_MODEMCONFIG1_BW_MASK &
                           RFLR_MODEMCONFIG1_CODINGRATE_MASK &
                           RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK ) |
                           ( bandwidth << 4 ) | ( coderate << 1 ) |
                           fixLen );

            SX1276Write( REG_LR_MODEMCONFIG2,
                         ( SX1276Read( REG_LR_MODEMCONFIG2 ) &
                           RFLR_MODEMCONFIG2_SF_MASK &
                           RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK &
                           RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK ) |
                           ( datarate << 4 ) | ( crcOn << 2 ) |
                           ( ( symbTimeout >> 8 ) & ~RFLR_MODEMCONFIG2_SYMBTIMEOUTMSB_MASK ) );

            SX1276Write( REG_LR_MODEMCONFIG3,
                         ( SX1276Read( REG_LR_MODEMCONFIG3 ) &
                           RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK ) |
                           ( SX1276.Settings.LoRa.LowDatarateOptimize << 3 ) );

            SX1276Write( REG_LR_SYMBTIMEOUTLSB, ( uint8_t )( symbTimeout & 0xFF ) );

            SX1276Write( REG_LR_PREAMBLEMSB, ( uint8_t )( ( preambleLen >> 8 ) & 0xFF ) );
            SX1276Write( REG_LR_PREAMBLELSB, ( uint8_t )( preambleLen & 0xFF ) );

            if( fixLen == 1 )
            {
                SX1276Write( REG_LR_PAYLOADLENGTH, payloadLen );
            }

            if( SX1276.Settings.LoRa.FreqHopOn == true )
            {
                SX1276Write( REG_LR_PLLHOP, ( SX1276Read( REG_LR_PLLHOP ) & RFLR_PLLHOP_FASTHOP_MASK ) | RFLR_PLLHOP_FASTHOP_ON );
                SX1276Write( REG_LR_HOPPERIOD, SX1276.Settings.LoRa.HopPeriod );
            }

            if( ( bandwidth == 9 ) && ( SX1276.Settings.Channel > RF_MID_BAND_THRESH ) )
            {
                // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
                SX1276Write( REG_LR_HIGHBWOPTIMIZE1, 0x02 );
                SX1276Write( REG_LR_HIGHBWOPTIMIZE2, 0x64 );
            }
            else if( bandwidth == 9 )
            {
                // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
                SX1276Write( REG_LR_HIGHBWOPTIMIZE1, 0x02 );
                SX1276Write( REG_LR_HIGHBWOPTIMIZE2, 0x7F );
            }
            else
            {
                // ERRATA 2.1 - Sensitivity Optimization with a 500 kHz Bandwidth
                SX1276Write( REG_LR_HIGHBWOPTIMIZE1, 0x03 );
            }

            if( datarate == 6 )
            {
                SX1276Write( REG_LR_DETECTOPTIMIZE,
                             ( SX1276Read( REG_LR_DETECTOPTIMIZE ) &
                               RFLR_DETECTIONOPTIMIZE_MASK ) |
                               RFLR_DETECTIONOPTIMIZE_SF6 );
                SX1276Write( REG_LR_DETECTIONTHRESHOLD,
                             RFLR_DETECTIONTHRESH_SF6 );
            }
            else
            {
                SX1276Write( REG_LR_DETECTOPTIMIZE,
                             ( SX1276Read( REG_LR_DETECTOPTIMIZE ) &
                             RFLR_DETECTIONOPTIMIZE_MASK ) |
                             RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12 );
                SX1276Write( REG_LR_DETECTIONTHRESHOLD,
                             RFLR_DETECTIONTHRESH_SF7_TO_SF12 );
            }
        }
        break;
    }
}

void SX1276SetTxConfig( RadioModems_t modem, int8_t power, uint32_t fdev,
                        uint32_t bandwidth, uint32_t datarate,
                        uint8_t coderate, uint16_t preambleLen,
                        bool fixLen, bool crcOn, bool freqHopOn,
                        uint8_t hopPeriod, bool iqInverted, uint32_t timeout )
{
    SX1276SetModem( modem );

    SX1276SetStby( );

    SX1276SetRfTxPower( power );

    switch( modem )
    {
    case MODEM_FSK:
        {
            SX1276.Settings.Fsk.Power = power;
            SX1276.Settings.Fsk.Fdev = fdev;
            SX1276.Settings.Fsk.Bandwidth = bandwidth;
            SX1276.Settings.Fsk.Datarate = datarate;
            SX1276.Settings.Fsk.PreambleLen = preambleLen;
            SX1276.Settings.Fsk.FixLen = fixLen;
            SX1276.Settings.Fsk.CrcOn = crcOn;
            SX1276.Settings.Fsk.IqInverted = iqInverted;
            SX1276.Settings.Fsk.TxTimeout = timeout;

            uint32_t fdevInPllSteps = SX1276ConvertFreqInHzToPllStep( fdev );
            SX1276Write( REG_FDEVMSB, ( uint8_t )( fdevInPllSteps >> 8 ) );
            SX1276Write( REG_FDEVLSB, ( uint8_t )( fdevInPllSteps & 0xFF ) );

            uint32_t bitRate = ( uint32_t )( SX1276_XTAL_FREQ / datarate );
            SX1276Write( REG_BITRATEMSB, ( uint8_t )( bitRate >> 8 ) );
            SX1276Write( REG_BITRATELSB, ( uint8_t )( bitRate & 0xFF ) );

            SX1276Write( REG_PREAMBLEMSB, ( preambleLen >> 8 ) & 0x00FF );
            SX1276Write( REG_PREAMBLELSB, preambleLen & 0xFF );

            SX1276Write( REG_PACKETCONFIG1,
                         ( SX1276Read( REG_PACKETCONFIG1 ) &
                           RF_PACKETCONFIG1_CRC_MASK &
                           RF_PACKETCONFIG1_PACKETFORMAT_MASK ) |
                           ( ( fixLen == 1 ) ? RF_PACKETCONFIG1_PACKETFORMAT_FIXED : RF_PACKETCONFIG1_PACKETFORMAT_VARIABLE ) |
                           ( crcOn << 4 ) );
            SX1276Write( REG_PACKETCONFIG2, ( SX1276Read( REG_PACKETCONFIG2 ) | RF_PACKETCONFIG2_DATAMODE_PACKET ) );
        }
        break;
    case MODEM_LORA:
        {
            SX1276.Settings.LoRa.Power = power;
            if( bandwidth > 2 )
            {
                // Fatal error: When using LoRa modem only bandwidths 125, 250 and 500 kHz are supported
                while( 1 );
            }
            bandwidth += 7;
            SX1276.Settings.LoRa.Bandwidth = bandwidth;
            SX1276.Settings.LoRa.Datarate = datarate;
            SX1276.Settings.LoRa.Coderate = coderate;
            SX1276.Settings.LoRa.PreambleLen = preambleLen;
            SX1276.Settings.LoRa.FixLen = fixLen;
            SX1276.Settings.LoRa.FreqHopOn = freqHopOn;
            SX1276.Settings.LoRa.HopPeriod = hopPeriod;
            SX1276.Settings.LoRa.CrcOn = crcOn;
            SX1276.Settings.LoRa.IqInverted = iqInverted;
            SX1276.Settings.LoRa.TxTimeout = timeout;

            if( datarate > 12 )
            {
                datarate = 12;
            }
            else if( datarate < 6 )
            {
                datarate = 6;
            }
            if( ( ( bandwidth == 7 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
                ( ( bandwidth == 8 ) && ( datarate == 12 ) ) )
            {
                SX1276.Settings.LoRa.LowDatarateOptimize = 0x01;
            }
            else
            {
                SX1276.Settings.LoRa.LowDatarateOptimize = 0x00;
            }

            if( SX1276.Settings.LoRa.FreqHopOn == true )
            {
                SX1276Write( REG_LR_PLLHOP, ( SX1276Read( REG_LR_PLLHOP ) & RFLR_PLLHOP_FASTHOP_MASK ) | RFLR_PLLHOP_FASTHOP_ON );
                SX1276Write( REG_LR_HOPPERIOD, SX1276.Settings.LoRa.HopPeriod );
            }

            SX1276Write( REG_LR_MODEMCONFIG1,
                         ( SX1276Read( REG_LR_MODEMCONFIG1 ) &
                           RFLR_MODEMCONFIG1_BW_MASK &
                           RFLR_MODEMCONFIG1_CODINGRATE_MASK &
                           RFLR_MODEMCONFIG1_IMPLICITHEADER_MASK ) |
                           ( bandwidth << 4 ) | ( coderate << 1 ) |
                           fixLen );

            SX1276Write( REG_LR_MODEMCONFIG2,
                         ( SX1276Read( REG_LR_MODEMCONFIG2 ) &
                           RFLR_MODEMCONFIG2_SF_MASK &
                           RFLR_MODEMCONFIG2_RXPAYLOADCRC_MASK ) |
                           ( datarate << 4 ) | ( crcOn << 2 ) );

            SX1276Write( REG_LR_MODEMCONFIG3,
                         ( SX1276Read( REG_LR_MODEMCONFIG3 ) &
                           RFLR_MODEMCONFIG3_LOWDATARATEOPTIMIZE_MASK ) |
                           ( SX1276.Settings.LoRa.LowDatarateOptimize << 3 ) );

            SX1276Write( REG_LR_PREAMBLEMSB, ( preambleLen >> 8 ) & 0x00FF );
            SX1276Write( REG_LR_PREAMBLELSB, preambleLen & 0xFF );

            if( datarate == 6 )
            {
                SX1276Write( REG_LR_DETECTOPTIMIZE,
                             ( SX1276Read( REG_LR_DETECTOPTIMIZE ) &
                               RFLR_DETECTIONOPTIMIZE_MASK ) |
                               RFLR_DETECTIONOPTIMIZE_SF6 );
                SX1276Write( REG_LR_DETECTIONTHRESHOLD,
                             RFLR_DETECTIONTHRESH_SF6 );
            }
            else
            {
                SX1276Write( REG_LR_DETECTOPTIMIZE,
                             ( SX1276Read( REG_LR_DETECTOPTIMIZE ) &
                             RFLR_DETECTIONOPTIMIZE_MASK ) |
                             RFLR_DETECTIONOPTIMIZE_SF7_TO_SF12 );
                SX1276Write( REG_LR_DETECTIONTHRESHOLD,
                             RFLR_DETECTIONTHRESH_SF7_TO_SF12 );
            }
        }
        break;
    }
}

uint32_t SX1276GetTimeOnAir( RadioModems_t modem, uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                              bool crcOn )
{
    uint32_t numerator = 0;
    uint32_t denominator = 1;

    switch( modem )
    {
    case MODEM_FSK:
        {
            numerator   = 1000U * SX1276GetGfskTimeOnAirNumerator( preambleLen, fixLen, payloadLen, crcOn );
            denominator = datarate;
        }
        break;
    case MODEM_LORA:
        {
            numerator   = 1000U * SX1276GetLoRaTimeOnAirNumerator( bandwidth, datarate, coderate, preambleLen, fixLen,
                                                                   payloadLen, crcOn );
            denominator = SX1276GetLoRaBandwidthInHz( bandwidth );
        }
        break;
    }
    // Perform integral ceil()
    return ( numerator + denominator - 1 ) / denominator;
}

void SX1276Send( uint8_t *buffer, uint8_t size )
{
    uint32_t txTimeout = 0;

    switch( SX1276.Settings.Modem )
    {
    case MODEM_FSK:
        {
            SX1276.Settings.FskPacketHandler.NbBytes = 0;
            SX1276.Settings.FskPacketHandler.Size = size;

            if( SX1276.Settings.Fsk.FixLen == false )
            {
                SX1276WriteFifo( ( uint8_t* )&size, 1 );
            }
            else
            {
                SX1276Write( REG_PAYLOADLENGTH, size );
            }

            if( ( size > 0 ) && ( size <= 64 ) )
            {
                SX1276.Settings.FskPacketHandler.ChunkSize = size;
            }
            else
            {
                memcpy1( RxTxBuffer, buffer, size );
                SX1276.Settings.FskPacketHandler.ChunkSize = 32;
            }

            // Write payload buffer
            SX1276WriteFifo( buffer, SX1276.Settings.FskPacketHandler.ChunkSize );
            SX1276.Settings.FskPacketHandler.NbBytes += SX1276.Settings.FskPacketHandler.ChunkSize;
            txTimeout = SX1276.Settings.Fsk.TxTimeout;
        }
        break;
    case MODEM_LORA:
        {
            if( SX1276.Settings.LoRa.IqInverted == true )
            {
                SX1276Write( REG_LR_INVERTIQ, ( ( SX1276Read( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_ON ) );
                SX1276Write( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_ON );
            }
            else
            {
                SX1276Write( REG_LR_INVERTIQ, ( ( SX1276Read( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_OFF ) );
                SX1276Write( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_OFF );
            }

            SX1276.Settings.LoRaPacketHandler.Size = size;

            // Initializes the payload size
            SX1276Write( REG_LR_PAYLOADLENGTH, size );

            // Full buffer used for Tx
            SX1276Write( REG_LR_FIFOTXBASEADDR, 0 );
            SX1276Write( REG_LR_FIFOADDRPTR, 0 );

            // FIFO operations can not take place in Sleep mode
            if( ( SX1276Read( REG_OPMODE ) & ~RF_OPMODE_MASK ) == RF_OPMODE_SLEEP )
            {
                SX1276SetStby( );
                DelayMs( 1 );
            }
            // Write payload buffer
            SX1276WriteFifo( buffer, size );
            txTimeout = SX1276.Settings.LoRa.TxTimeout;
        }
        break;
    }

    SX1276SetTx( txTimeout );
}

void SX1276SetSleep( void )
{
    TimerStop( &RxTimeoutTimer );
    TimerStop( &TxTimeoutTimer );
    TimerStop( &RxTimeoutSyncWord );

    SX1276SetOpMode( RF_OPMODE_SLEEP );

    // Disable TCXO radio is in SLEEP mode
    SX1276SetBoardTcxo( false );

    SX1276.Settings.State = RF_IDLE;
}

void SX1276SetStby( void )
{
    TimerStop( &RxTimeoutTimer );
    TimerStop( &TxTimeoutTimer );
    TimerStop( &RxTimeoutSyncWord );

    SX1276SetOpMode( RF_OPMODE_STANDBY );
    SX1276.Settings.State = RF_IDLE;
}

void SX1276SetRx( uint32_t timeout )
{
    bool rxContinuous = false;
    TimerStop( &TxTimeoutTimer );

    switch( SX1276.Settings.Modem )
    {
    case MODEM_FSK:
        {
            rxContinuous = SX1276.Settings.Fsk.RxContinuous;

            // DIO0=PayloadReady
            // DIO1=FifoLevel
            // DIO2=SyncAddr
            // DIO3=FifoEmpty
            // DIO4=Preamble
            // DIO5=ModeReady
            SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RF_DIOMAPPING1_DIO0_MASK &
                                                                            RF_DIOMAPPING1_DIO1_MASK &
                                                                            RF_DIOMAPPING1_DIO2_MASK ) |
                                                                            RF_DIOMAPPING1_DIO0_00 |
                                                                            RF_DIOMAPPING1_DIO1_00 |
                                                                            RF_DIOMAPPING1_DIO2_11 );

            SX1276Write( REG_DIOMAPPING2, ( SX1276Read( REG_DIOMAPPING2 ) & RF_DIOMAPPING2_DIO4_MASK &
                                                                            RF_DIOMAPPING2_MAP_MASK ) |
                                                                            RF_DIOMAPPING2_DIO4_11 |
                                                                            RF_DIOMAPPING2_MAP_PREAMBLEDETECT );

            SX1276.Settings.FskPacketHandler.FifoThresh = SX1276Read( REG_FIFOTHRESH ) & 0x3F;

            SX1276Write( REG_RXCONFIG, RF_RXCONFIG_AFCAUTO_ON | RF_RXCONFIG_AGCAUTO_ON | RF_RXCONFIG_RXTRIGER_PREAMBLEDETECT );

            SX1276.Settings.FskPacketHandler.PreambleDetected = false;
            SX1276.Settings.FskPacketHandler.SyncWordDetected = false;
            SX1276.Settings.FskPacketHandler.NbBytes = 0;
            SX1276.Settings.FskPacketHandler.Size = 0;
        }
        break;
    case MODEM_LORA:
        {
            if( SX1276.Settings.LoRa.IqInverted == true )
            {
                SX1276Write( REG_LR_INVERTIQ, ( ( SX1276Read( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_ON | RFLR_INVERTIQ_TX_OFF ) );
                SX1276Write( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_ON );
            }
            else
            {
                SX1276Write( REG_LR_INVERTIQ, ( ( SX1276Read( REG_LR_INVERTIQ ) & RFLR_INVERTIQ_TX_MASK & RFLR_INVERTIQ_RX_MASK ) | RFLR_INVERTIQ_RX_OFF | RFLR_INVERTIQ_TX_OFF ) );
                SX1276Write( REG_LR_INVERTIQ2, RFLR_INVERTIQ2_OFF );
            }

            // ERRATA 2.3 - Receiver Spurious Reception of a LoRa Signal
            if( SX1276.Settings.LoRa.Bandwidth < 9 )
            {
                SX1276Write( REG_LR_DETECTOPTIMIZE, SX1276Read( REG_LR_DETECTOPTIMIZE ) & 0x7F );
                SX1276Write( REG_LR_IFFREQ2, 0x00 );
                switch( SX1276.Settings.LoRa.Bandwidth )
                {
                case 0: // 7.8 kHz
                    SX1276Write( REG_LR_IFFREQ1, 0x48 );
                    SX1276SetChannel(SX1276.Settings.Channel + 7810 );
                    break;
                case 1: // 10.4 kHz
                    SX1276Write( REG_LR_IFFREQ1, 0x44 );
                    SX1276SetChannel(SX1276.Settings.Channel + 10420 );
                    break;
                case 2: // 15.6 kHz
                    SX1276Write( REG_LR_IFFREQ1, 0x44 );
                    SX1276SetChannel(SX1276.Settings.Channel + 15620 );
                    break;
                case 3: // 20.8 kHz
                    SX1276Write( REG_LR_IFFREQ1, 0x44 );
                    SX1276SetChannel(SX1276.Settings.Channel + 20830 );
                    break;
                case 4: // 31.2 kHz
                    SX1276Write( REG_LR_IFFREQ1, 0x44 );
                    SX1276SetChannel(SX1276.Settings.Channel + 31250 );
                    break;
                case 5: // 41.4 kHz
                    SX1276Write( REG_LR_IFFREQ1, 0x44 );
                    SX1276SetChannel(SX1276.Settings.Channel + 41670 );
                    break;
                case 6: // 62.5 kHz
                    SX1276Write( REG_LR_IFFREQ1, 0x40 );
                    break;
                case 7: // 125 kHz
                    SX1276Write( REG_LR_IFFREQ1, 0x40 );
                    break;
                case 8: // 250 kHz
                    SX1276Write( REG_LR_IFFREQ1, 0x40 );
                    break;
                }
            }
            else
            {
                SX1276Write( REG_LR_DETECTOPTIMIZE, SX1276Read( REG_LR_DETECTOPTIMIZE ) | 0x80 );
            }

            rxContinuous = SX1276.Settings.LoRa.RxContinuous;

            if( SX1276.Settings.LoRa.FreqHopOn == true )
            {
                SX1276Write( REG_LR_IRQFLAGSMASK, //RFLR_IRQFLAGS_RXTIMEOUT |
                                                  //RFLR_IRQFLAGS_RXDONE |
                                                  //RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                                  RFLR_IRQFLAGS_VALIDHEADER |
                                                  RFLR_IRQFLAGS_TXDONE |
                                                  RFLR_IRQFLAGS_CADDONE |
                                                  //RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                                  RFLR_IRQFLAGS_CADDETECTED );

                // DIO0=RxDone, DIO2=FhssChangeChannel
                SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RFLR_DIOMAPPING1_DIO0_MASK & RFLR_DIOMAPPING1_DIO2_MASK  ) | RFLR_DIOMAPPING1_DIO0_00 | RFLR_DIOMAPPING1_DIO2_00 );
            }
            else
            {
                SX1276Write( REG_LR_IRQFLAGSMASK, //RFLR_IRQFLAGS_RXTIMEOUT |
                                                  //RFLR_IRQFLAGS_RXDONE |
                                                  //RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                                  RFLR_IRQFLAGS_VALIDHEADER |
                                                  RFLR_IRQFLAGS_TXDONE |
                                                  RFLR_IRQFLAGS_CADDONE |
                                                  RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                                  RFLR_IRQFLAGS_CADDETECTED );

                // DIO0=RxDone
                SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RFLR_DIOMAPPING1_DIO0_MASK ) | RFLR_DIOMAPPING1_DIO0_00 );
            }
            SX1276Write( REG_LR_FIFORXBASEADDR, 0 );
            SX1276Write( REG_LR_FIFOADDRPTR, 0 );
        }
        break;
    }

    SX1276.Settings.State = RF_RX_RUNNING;
    if( timeout != 0 )
    {
        TimerSetValue( &RxTimeoutTimer, timeout );
        TimerStart( &RxTimeoutTimer );
    }

    if( SX1276.Settings.Modem == MODEM_FSK )
    {
        SX1276SetOpMode( RF_OPMODE_RECEIVER );

        if( rxContinuous == false )
        {
            TimerSetValue( &RxTimeoutSyncWord, SX1276.Settings.Fsk.RxSingleTimeout );
            TimerStart( &RxTimeoutSyncWord );
        }
    }
    else
    {
        if( rxContinuous == true )
        {
            SX1276SetOpMode( RFLR_OPMODE_RECEIVER );
        }
        else
        {
            SX1276SetOpMode( RFLR_OPMODE_RECEIVER_SINGLE );
        }
    }
}

static void SX1276SetTx( uint32_t timeout )
{
    TimerStop( &RxTimeoutTimer );

    TimerSetValue( &TxTimeoutTimer, timeout );

    switch( SX1276.Settings.Modem )
    {
    case MODEM_FSK:
        {
            // DIO0=PacketSent
            // DIO1=FifoLevel
            // DIO2=FifoFull
            // DIO3=FifoEmpty
            // DIO4=LowBat
            // DIO5=ModeReady
            SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RF_DIOMAPPING1_DIO0_MASK &
                                                                            RF_DIOMAPPING1_DIO1_MASK &
                                                                            RF_DIOMAPPING1_DIO2_MASK ) );

            SX1276Write( REG_DIOMAPPING2, ( SX1276Read( REG_DIOMAPPING2 ) & RF_DIOMAPPING2_DIO4_MASK &
                                                                            RF_DIOMAPPING2_MAP_MASK ) );
            SX1276.Settings.FskPacketHandler.FifoThresh = SX1276Read( REG_FIFOTHRESH ) & 0x3F;
        }
        break;
    case MODEM_LORA:
        {
            if( SX1276.Settings.LoRa.FreqHopOn == true )
            {
                SX1276Write( REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
                                                  RFLR_IRQFLAGS_RXDONE |
                                                  RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                                  RFLR_IRQFLAGS_VALIDHEADER |
                                                  //RFLR_IRQFLAGS_TXDONE |
                                                  RFLR_IRQFLAGS_CADDONE |
                                                  //RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                                  RFLR_IRQFLAGS_CADDETECTED );

                // DIO0=TxDone, DIO2=FhssChangeChannel
                SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RFLR_DIOMAPPING1_DIO0_MASK & RFLR_DIOMAPPING1_DIO2_MASK ) | RFLR_DIOMAPPING1_DIO0_01 | RFLR_DIOMAPPING1_DIO2_00 );
            }
            else
            {
                SX1276Write( REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
                                                  RFLR_IRQFLAGS_RXDONE |
                                                  RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                                  RFLR_IRQFLAGS_VALIDHEADER |
                                                  //RFLR_IRQFLAGS_TXDONE |
                                                  RFLR_IRQFLAGS_CADDONE |
                                                  RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL |
                                                  RFLR_IRQFLAGS_CADDETECTED );

                // DIO0=TxDone
                SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RFLR_DIOMAPPING1_DIO0_MASK ) | RFLR_DIOMAPPING1_DIO0_01 );
            }
        }
        break;
    }

    SX1276.Settings.State = RF_TX_RUNNING;
    TimerStart( &TxTimeoutTimer );
    SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
}

void SX1276StartCad( void )
{
    switch( SX1276.Settings.Modem )
    {
    case MODEM_FSK:
        {

        }
        break;
    case MODEM_LORA:
        {
            SX1276Write( REG_LR_IRQFLAGSMASK, RFLR_IRQFLAGS_RXTIMEOUT |
                                        RFLR_IRQFLAGS_RXDONE |
                                        RFLR_IRQFLAGS_PAYLOADCRCERROR |
                                        RFLR_IRQFLAGS_VALIDHEADER |
                                        RFLR_IRQFLAGS_TXDONE |
                                        //RFLR_IRQFLAGS_CADDONE |
                                        RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL // |
                                        //RFLR_IRQFLAGS_CADDETECTED
                                        );

            // DIO3=CADDone
            SX1276Write( REG_DIOMAPPING1, ( SX1276Read( REG_DIOMAPPING1 ) & RFLR_DIOMAPPING1_DIO3_MASK ) | RFLR_DIOMAPPING1_DIO3_00 );

            SX1276.Settings.State = RF_CAD;
            SX1276SetOpMode( RFLR_OPMODE_CAD );
        }
        break;
    default:
        break;
    }
}

void SX1276SetTxContinuousWave( uint32_t freq, int8_t power, uint16_t time )
{
    uint32_t timeout = ( uint32_t )time * 1000;

    SX1276SetChannel( freq );

    SX1276SetTxConfig( MODEM_FSK, power, 0, 0, 4800, 0, 5, false, false, 0, 0, 0, timeout );

    SX1276Write( REG_PACKETCONFIG2, ( SX1276Read( REG_PACKETCONFIG2 ) & RF_PACKETCONFIG2_DATAMODE_MASK ) );
    // Disable radio interrupts
    SX1276Write( REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_11 | RF_DIOMAPPING1_DIO1_11 );
    SX1276Write( REG_DIOMAPPING2, RF_DIOMAPPING2_DIO4_10 | RF_DIOMAPPING2_DIO5_10 );

    TimerSetValue( &TxTimeoutTimer, timeout );

    SX1276.Settings.State = RF_TX_RUNNING;
    TimerStart( &TxTimeoutTimer );
    SX1276SetOpMode( RF_OPMODE_TRANSMITTER );
}

int16_t SX1276ReadRssi( RadioModems_t modem )
{
    int16_t rssi = 0;

    switch( modem )
    {
    case MODEM_FSK:
        rssi = -( SX1276Read( REG_RSSIVALUE ) >> 1 );
        break;
    case MODEM_LORA:
        if( SX1276.Settings.Channel > RF_MID_BAND_THRESH )
        {
            rssi = RSSI_OFFSET_HF + SX1276Read( REG_LR_RSSIVALUE );
        }
        else
        {
            rssi = RSSI_OFFSET_LF + SX1276Read( REG_LR_RSSIVALUE );
        }
        break;
    default:
        rssi = -1;
        break;
    }
    return rssi;
}

static void SX1276SetOpMode( uint8_t opMode )
{
#if defined( USE_RADIO_DEBUG )
    switch( opMode )
    {
        case RF_OPMODE_TRANSMITTER:
            SX1276DbgPinTxWrite( 1 );
            SX1276DbgPinRxWrite( 0 );
            break;
        case RF_OPMODE_RECEIVER:
        case RFLR_OPMODE_RECEIVER_SINGLE:
            SX1276DbgPinTxWrite( 0 );
            SX1276DbgPinRxWrite( 1 );
            break;
        default:
            SX1276DbgPinTxWrite( 0 );
            SX1276DbgPinRxWrite( 0 );
            break;
    }
#endif
    if( opMode == RF_OPMODE_SLEEP )
    {
        SX1276SetAntSwLowPower( true );
    }
    else
    {
        // Enable TCXO if operating mode different from SLEEP.
        SX1276SetBoardTcxo( true );
        SX1276SetAntSwLowPower( false );
        SX1276SetAntSw( opMode );
    }
    SX1276Write( REG_OPMODE, ( SX1276Read( REG_OPMODE ) & RF_OPMODE_MASK ) | opMode );
}

void SX1276SetModem( RadioModems_t modem )
{
    if( ( SX1276Read( REG_OPMODE ) & RFLR_OPMODE_LONGRANGEMODE_ON ) != 0 )
    {
        SX1276.Settings.Modem = MODEM_LORA;
    }
    else
    {
        SX1276.Settings.Modem = MODEM_FSK;
    }

    if( SX1276.Settings.Modem == modem )
    {
        return;
    }

    SX1276.Settings.Modem = modem;
    switch( SX1276.Settings.Modem )
    {
    default:
    case MODEM_FSK:
        SX1276SetOpMode( RF_OPMODE_SLEEP );
        SX1276Write( REG_OPMODE, ( SX1276Read( REG_OPMODE ) & RFLR_OPMODE_LONGRANGEMODE_MASK ) | RFLR_OPMODE_LONGRANGEMODE_OFF );

        SX1276Write( REG_DIOMAPPING1, 0x00 );
        SX1276Write( REG_DIOMAPPING2, 0x30 ); // DIO5=ModeReady
        break;
    case MODEM_LORA:
        SX1276SetOpMode( RF_OPMODE_SLEEP );
        SX1276Write( REG_OPMODE, ( SX1276Read( REG_OPMODE ) & RFLR_OPMODE_LONGRANGEMODE_MASK ) | RFLR_OPMODE_LONGRANGEMODE_ON );

        SX1276Write( REG_DIOMAPPING1, 0x00 );
        SX1276Write( REG_DIOMAPPING2, 0x00 );
        break;
    }
}

void SX1276Write( uint32_t addr, uint8_t data )
{
    SX1276WriteBuffer( addr, &data, 1 );
}

uint8_t SX1276Read( uint32_t addr )
{
    uint8_t data;
    SX1276ReadBuffer( addr, &data, 1 );
    return data;
}

void SX1276WriteBuffer( uint32_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //NSS = 0;
    GpioWrite( &SX1276.Spi.Nss, 0 );

    SpiInOut( &SX1276.Spi, addr | 0x80 );
    for( i = 0; i < size; i++ )
    {
        SpiInOut( &SX1276.Spi, buffer[i] );
    }

    //NSS = 1;
    GpioWrite( &SX1276.Spi.Nss, 1 );
}

void SX1276ReadBuffer( uint32_t addr, uint8_t *buffer, uint8_t size )
{
    uint8_t i;

    //NSS = 0;
    GpioWrite( &SX1276.Spi.Nss, 0 );

    SpiInOut( &SX1276.Spi, addr & 0x7F );

    for( i = 0; i < size; i++ )
    {
        buffer[i] = SpiInOut( &SX1276.Spi, 0 );
    }

    //NSS = 1;
    GpioWrite( &SX1276.Spi.Nss, 1 );
}

static void SX1276WriteFifo( uint8_t *buffer, uint8_t size )
{
    SX1276WriteBuffer( 0, buffer, size );
}

static void SX1276ReadFifo( uint8_t *buffer, uint8_t size )
{
    SX1276ReadBuffer( 0, buffer, size );
}

void SX1276SetMaxPayloadLength( RadioModems_t modem, uint8_t max )
{
    SX1276SetModem( modem );

    switch( modem )
    {
    case MODEM_FSK:
        if( SX1276.Settings.Fsk.FixLen == false )
        {
            SX1276Write( REG_PAYLOADLENGTH, max );
        }
        break;
    case MODEM_LORA:
        SX1276Write( REG_LR_PAYLOADMAXLENGTH, max );
        break;
    }
}

void SX1276SetPublicNetwork( bool enable )
{
    SX1276SetModem( MODEM_LORA );
    SX1276.Settings.LoRa.PublicNetwork = enable;
    if( enable == true )
    {
        // Change LoRa modem SyncWord
        SX1276Write( REG_LR_SYNCWORD, LORA_MAC_PUBLIC_SYNCWORD );
    }
    else
    {
        // Change LoRa modem SyncWord
        SX1276Write( REG_LR_SYNCWORD, LORA_MAC_PRIVATE_SYNCWORD );
    }
}

uint32_t SX1276GetWakeupTime( void )
{
    return SX1276GetBoardTcxoWakeupTime( ) + RADIO_WAKEUP_TIME;
}

static uint32_t SX1276ConvertPllStepToFreqInHz( uint32_t pllSteps )
{
    uint32_t freqInHzInt;
    uint32_t freqInHzFrac;
    
    // freqInHz = pllSteps * ( SX1276_XTAL_FREQ / 2^19 )
    // Get integer and fractional parts of the frequency computed with a PLL step scaled value
    freqInHzInt = pllSteps >> SX1276_PLL_STEP_SHIFT_AMOUNT;
    freqInHzFrac = pllSteps - ( freqInHzInt << SX1276_PLL_STEP_SHIFT_AMOUNT );
    
    // Apply the scaling factor to retrieve a frequency in Hz (+ ceiling)
    return freqInHzInt * SX1276_PLL_STEP_SCALED + 
           ( ( freqInHzFrac * SX1276_PLL_STEP_SCALED + ( 128 ) ) >> SX1276_PLL_STEP_SHIFT_AMOUNT );
}

static uint32_t SX1276ConvertFreqInHzToPllStep( uint32_t freqInHz )
{
    uint32_t stepsInt;
    uint32_t stepsFrac;

    // pllSteps = freqInHz / (SX1276_XTAL_FREQ / 2^19 )
    // Get integer and fractional parts of the frequency computed with a PLL step scaled value
    stepsInt = freqInHz / SX1276_PLL_STEP_SCALED;
    stepsFrac = freqInHz - ( stepsInt * SX1276_PLL_STEP_SCALED );
    
    // Apply the scaling factor to retrieve a frequency in Hz (+ ceiling)
    return ( stepsInt << SX1276_PLL_STEP_SHIFT_AMOUNT ) + 
           ( ( ( stepsFrac << SX1276_PLL_STEP_SHIFT_AMOUNT ) + ( SX1276_PLL_STEP_SCALED >> 1 ) ) /
             SX1276_PLL_STEP_SCALED );
}

static uint8_t GetFskBandwidthRegValue( uint32_t bw )
{
    uint8_t i;

    for( i = 0; i < ( sizeof( FskBandwidths ) / sizeof( FskBandwidth_t ) ) - 1; i++ )
    {
        if( ( bw >= FskBandwidths[i].bandwidth ) && ( bw < FskBandwidths[i + 1].bandwidth ) )
        {
            return FskBandwidths[i].RegValue;
        }
    }
    // ERROR: Value not found
    while( 1 );
}

static uint32_t SX1276GetLoRaBandwidthInHz( uint32_t bw )
{
    uint32_t bandwidthInHz = 0;

    switch( bw )
    {
    case 0: // 125 kHz
        bandwidthInHz = 125000UL;
        break;
    case 1: // 250 kHz
        bandwidthInHz = 250000UL;
        break;
    case 2: // 500 kHz
        bandwidthInHz = 500000UL;
        break;
    }

    return bandwidthInHz;
}

static uint32_t SX1276GetGfskTimeOnAirNumerator( uint16_t preambleLen, bool fixLen,
                                                 uint8_t payloadLen, bool crcOn )
{
    const uint8_t syncWordLength = 3;

    return ( preambleLen << 3 ) +
           ( ( fixLen == false ) ? 8 : 0 ) +
             ( syncWordLength << 3 ) +
             ( ( payloadLen +
               ( 0 ) + // Address filter size
               ( ( crcOn == true ) ? 2 : 0 ) 
               ) << 3 
             );
}

static uint32_t SX1276GetLoRaTimeOnAirNumerator( uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                              bool crcOn )
{
    int32_t crDenom           = coderate + 4;
    bool    lowDatareOptimize = false;

    // Ensure that the preamble length is at least 12 symbols when using SF5 or
    // SF6
    if( ( datarate == 5 ) || ( datarate == 6 ) )
    {
        if( preambleLen < 12 )
        {
            preambleLen = 12;
        }
    }

    if( ( ( bandwidth == 0 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
        ( ( bandwidth == 1 ) && ( datarate == 12 ) ) )
    {
        lowDatareOptimize = true;
    }

    int32_t ceilDenominator;
    int32_t ceilNumerator = ( payloadLen << 3 ) +
                            ( crcOn ? 16 : 0 ) -
                            ( 4 * datarate ) +
                            ( fixLen ? 0 : 20 );

    if( datarate <= 6 )
    {
        ceilDenominator = 4 * datarate;
    }
    else
    {
        ceilNumerator += 8;

        if( lowDatareOptimize == true )
        {
            ceilDenominator = 4 * ( datarate - 2 );
        }
        else
        {
            ceilDenominator = 4 * datarate;
        }
    }

    if( ceilNumerator < 0 )
    {
        ceilNumerator = 0;
    }

    // Perform integral ceil()
    int32_t intermediate =
        ( ( ceilNumerator + ceilDenominator - 1 ) / ceilDenominator ) * crDenom + preambleLen + 12;

    if( datarate <= 6 )
    {
        intermediate += 2;
    }

    return ( uint32_t )( ( 4 * intermediate + 1 ) * ( 1 << ( datarate - 2 ) ) );
}

static void SX1276OnTimeoutIrq( void* context )
{
    switch( SX1276.Settings.State )
    {
    case RF_RX_RUNNING:
        if( SX1276.Settings.Modem == MODEM_FSK )
        {
            SX1276.Settings.FskPacketHandler.PreambleDetected = false;
            SX1276.Settings.FskPacketHandler.SyncWordDetected = false;
            SX1276.Settings.FskPacketHandler.NbBytes = 0;
            SX1276.Settings.FskPacketHandler.Size = 0;

            // Clear Irqs
            SX1276Write( REG_IRQFLAGS1, RF_IRQFLAGS1_RSSI |
                                        RF_IRQFLAGS1_PREAMBLEDETECT |
                                        RF_IRQFLAGS1_SYNCADDRESSMATCH );
            SX1276Write( REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN );

            if( SX1276.Settings.Fsk.RxContinuous == true )
            {
                // Continuous mode restart Rx chain
                SX1276Write( REG_RXCONFIG, SX1276Read( REG_RXCONFIG ) | RF_RXCONFIG_RESTARTRXWITHOUTPLLLOCK );
            }
            else
            {
                SX1276.Settings.State = RF_IDLE;
                TimerStop( &RxTimeoutSyncWord );
            }
        }
        if( ( RadioEvents != NULL ) && ( RadioEvents->RxTimeout != NULL ) )
        {
            RadioEvents->RxTimeout( );
        }
        break;
    case RF_TX_RUNNING:
        // Tx timeout shouldn't happen.
        // Reported issue of SPI data corruption resulting in TX TIMEOUT 
        // is NOT related to a bug in radio transceiver.
        // It is mainly caused by improper PCB routing of SPI lines and/or
        // violation of SPI specifications.
        // To mitigate redesign, Semtech offers a workaround which resets
        // the radio transceiver and putting it into a known state.

        // BEGIN WORKAROUND

        // Reset the radio
        SX1276Reset( );

        // Calibrate Rx chain
        RxChainCalibration( );

        // Initialize radio default values
        SX1276SetOpMode( RF_OPMODE_SLEEP );

        for( uint8_t i = 0; i < sizeof( RadioRegsInit ) / sizeof( RadioRegisters_t ); i++ )
        {
            SX1276SetModem( RadioRegsInit[i].Modem );
            SX1276Write( RadioRegsInit[i].Addr, RadioRegsInit[i].Value );
        }
        SX1276SetModem( MODEM_FSK );

        // Restore previous network type setting.
        SX1276SetPublicNetwork( SX1276.Settings.LoRa.PublicNetwork );
        // END WORKAROUND

        SX1276.Settings.State = RF_IDLE;
        if( ( RadioEvents != NULL ) && ( RadioEvents->TxTimeout != NULL ) )
        {
            RadioEvents->TxTimeout( );
        }
        break;
    default:
        break;
    }
}

static void SX1276OnDio0Irq( void* context )
{
    volatile uint8_t irqFlags = 0;

    switch( SX1276.Settings.State )
    {
        case RF_RX_RUNNING:
            //TimerStop( &RxTimeoutTimer );
            // RxDone interrupt
            switch( SX1276.Settings.Modem )
            {
            case MODEM_FSK:
                if( SX1276.Settings.Fsk.CrcOn == true )
                {
                    irqFlags = SX1276Read( REG_IRQFLAGS2 );
                    if( ( irqFlags & RF_IRQFLAGS2_CRCOK ) != RF_IRQFLAGS2_CRCOK )
                    {
                        // Clear Irqs
                        SX1276Write( REG_IRQFLAGS1, RF_IRQFLAGS1_RSSI |
                                                    RF_IRQFLAGS1_PREAMBLEDETECT |
                                                    RF_IRQFLAGS1_SYNCADDRESSMATCH );
                        SX1276Write( REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN );

                        TimerStop( &RxTimeoutTimer );

                        if( SX1276.Settings.Fsk.RxContinuous == false )
                        {
                            TimerStop( &RxTimeoutSyncWord );
                            SX1276.Settings.State = RF_IDLE;
                        }
                        else
                        {
                            // Continuous mode restart Rx chain
                            SX1276Write( REG_RXCONFIG, SX1276Read( REG_RXCONFIG ) | RF_RXCONFIG_RESTARTRXWITHOUTPLLLOCK );
                        }

                        if( ( RadioEvents != NULL ) && ( RadioEvents->RxError != NULL ) )
                        {
                            RadioEvents->RxError( );
                        }
                        SX1276.Settings.FskPacketHandler.PreambleDetected = false;
                        SX1276.Settings.FskPacketHandler.SyncWordDetected = false;
                        SX1276.Settings.FskPacketHandler.NbBytes = 0;
                        SX1276.Settings.FskPacketHandler.Size = 0;
                        break;
                    }
                }

                // Read received packet size
                if( ( SX1276.Settings.FskPacketHandler.Size == 0 ) && ( SX1276.Settings.FskPacketHandler.NbBytes == 0 ) )
                {
                    if( SX1276.Settings.Fsk.FixLen == false )
                    {
                        SX1276ReadFifo( ( uint8_t* )&SX1276.Settings.FskPacketHandler.Size, 1 );
                    }
                    else
                    {
                        SX1276.Settings.FskPacketHandler.Size = SX1276Read( REG_PAYLOADLENGTH );
                    }
                    SX1276ReadFifo( RxTxBuffer + SX1276.Settings.FskPacketHandler.NbBytes, SX1276.Settings.FskPacketHandler.Size - SX1276.Settings.FskPacketHandler.NbBytes );
                    SX1276.Settings.FskPacketHandler.NbBytes += ( SX1276.Settings.FskPacketHandler.Size - SX1276.Settings.FskPacketHandler.NbBytes );
                }
                else
                {
                    SX1276ReadFifo( RxTxBuffer + SX1276.Settings.FskPacketHandler.NbBytes, SX1276.Settings.FskPacketHandler.Size - SX1276.Settings.FskPacketHandler.NbBytes );
                    SX1276.Settings.FskPacketHandler.NbBytes += ( SX1276.Settings.FskPacketHandler.Size - SX1276.Settings.FskPacketHandler.NbBytes );
                }

                TimerStop( &RxTimeoutTimer );

                if( SX1276.Settings.Fsk.RxContinuous == false )
                {
                    SX1276.Settings.State = RF_IDLE;
                    TimerStop( &RxTimeoutSyncWord );
                }
                else
                {
                    // Continuous mode restart Rx chain
                    SX1276Write( REG_RXCONFIG, SX1276Read( REG_RXCONFIG ) | RF_RXCONFIG_RESTARTRXWITHOUTPLLLOCK );
                }

                if( ( RadioEvents != NULL ) && ( RadioEvents->RxDone != NULL ) )
                {
                    RadioEvents->RxDone( RxTxBuffer, SX1276.Settings.FskPacketHandler.Size, SX1276.Settings.FskPacketHandler.RssiValue, 0 );
                }
                SX1276.Settings.FskPacketHandler.PreambleDetected = false;
                SX1276.Settings.FskPacketHandler.SyncWordDetected = false;
                SX1276.Settings.FskPacketHandler.NbBytes = 0;
                SX1276.Settings.FskPacketHandler.Size = 0;
                break;
            case MODEM_LORA:
                {
                    // Clear Irq
                    SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_RXDONE );

                    irqFlags = SX1276Read( REG_LR_IRQFLAGS );
                    if( ( irqFlags & RFLR_IRQFLAGS_PAYLOADCRCERROR_MASK ) == RFLR_IRQFLAGS_PAYLOADCRCERROR )
                    {
                        // Clear Irq
                        SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_PAYLOADCRCERROR );

                        if( SX1276.Settings.LoRa.RxContinuous == false )
                        {
                            SX1276.Settings.State = RF_IDLE;
                        }
                        TimerStop( &RxTimeoutTimer );

                        if( ( RadioEvents != NULL ) && ( RadioEvents->RxError != NULL ) )
                        {
                            RadioEvents->RxError( );
                        }
                        break;
                    }

                    // Returns SNR value [dB] rounded to the nearest integer value
                    SX1276.Settings.LoRaPacketHandler.SnrValue = ( ( ( int8_t )SX1276Read( REG_LR_PKTSNRVALUE ) ) + 2 ) >> 2;

                    int16_t rssi = SX1276Read( REG_LR_PKTRSSIVALUE );
                    if( SX1276.Settings.LoRaPacketHandler.SnrValue < 0 )
                    {
                        if( SX1276.Settings.Channel > RF_MID_BAND_THRESH )
                        {
                            SX1276.Settings.LoRaPacketHandler.RssiValue = RSSI_OFFSET_HF + rssi + ( rssi >> 4 ) +
                                                                          SX1276.Settings.LoRaPacketHandler.SnrValue;
                        }
                        else
                        {
                            SX1276.Settings.LoRaPacketHandler.RssiValue = RSSI_OFFSET_LF + rssi + ( rssi >> 4 ) +
                                                                          SX1276.Settings.LoRaPacketHandler.SnrValue;
                        }
                    }
                    else
                    {
                        if( SX1276.Settings.Channel > RF_MID_BAND_THRESH )
                        {
                            SX1276.Settings.LoRaPacketHandler.RssiValue = RSSI_OFFSET_HF + rssi + ( rssi >> 4 );
                        }
                        else
                        {
                            SX1276.Settings.LoRaPacketHandler.RssiValue = RSSI_OFFSET_LF + rssi + ( rssi >> 4 );
                        }
                    }

                    SX1276.Settings.LoRaPacketHandler.Size = SX1276Read( REG_LR_RXNBBYTES );
                    SX1276Write( REG_LR_FIFOADDRPTR, SX1276Read( REG_LR_FIFORXCURRENTADDR ) );
                    SX1276ReadFifo( RxTxBuffer, SX1276.Settings.LoRaPacketHandler.Size );

                    if( SX1276.Settings.LoRa.RxContinuous == false )
                    {
                        SX1276.Settings.State = RF_IDLE;
                    }
                    TimerStop( &RxTimeoutTimer );

                    if( ( RadioEvents != NULL ) && ( RadioEvents->RxDone != NULL ) )
                    {
                        RadioEvents->RxDone( RxTxBuffer, SX1276.Settings.LoRaPacketHandler.Size, SX1276.Settings.LoRaPacketHandler.RssiValue, SX1276.Settings.LoRaPacketHandler.SnrValue );
                    }
                }
                break;
            default:
                break;
            }
            break;
        case RF_TX_RUNNING:
            TimerStop( &TxTimeoutTimer );
            // TxDone interrupt
            switch( SX1276.Settings.Modem )
            {
            case MODEM_LORA:
                // Clear Irq
                SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_TXDONE );
                // Intentional fall through
            case MODEM_FSK:
            default:
                SX1276.Settings.State = RF_IDLE;
                if( ( RadioEvents != NULL ) && ( RadioEvents->TxDone != NULL ) )
                {
                    RadioEvents->TxDone( );
                }
                break;
            }
            break;
        default:
            break;
    }
}

static void SX1276OnDio1Irq( void* context )
{
    switch( SX1276.Settings.State )
    {
        case RF_RX_RUNNING:
            switch( SX1276.Settings.Modem )
            {
            case MODEM_FSK:
                // Check FIFO level DIO1 pin state
                //
                // As DIO1 interrupt is triggered when a rising or a falling edge is detected the IRQ handler must
                // verify DIO1 pin state in order to decide if something has to be done.
                // When radio is operating in FSK reception mode a rising edge must be detected in order to handle the
                // IRQ.
                if( SX1276GetDio1PinState( ) == 0 )
                {
                    break;
                }
                // Stop timer
                TimerStop( &RxTimeoutSyncWord );

                // FifoLevel interrupt
                // Read received packet size
                if( ( SX1276.Settings.FskPacketHandler.Size == 0 ) && ( SX1276.Settings.FskPacketHandler.NbBytes == 0 ) )
                {
                    if( SX1276.Settings.Fsk.FixLen == false )
                    {
                        SX1276ReadFifo( ( uint8_t* )&SX1276.Settings.FskPacketHandler.Size, 1 );
                    }
                    else
                    {
                        SX1276.Settings.FskPacketHandler.Size = SX1276Read( REG_PAYLOADLENGTH );
                    }
                }

                // ERRATA 3.1 - PayloadReady Set for 31.25ns if FIFO is Empty
                //
                //              When FifoLevel interrupt is used to offload the
                //              FIFO, the microcontroller should  monitor  both
                //              PayloadReady  and FifoLevel interrupts, and
                //              read only (FifoThreshold-1) bytes off the FIFO
                //              when FifoLevel fires
                if( ( SX1276.Settings.FskPacketHandler.Size - SX1276.Settings.FskPacketHandler.NbBytes ) >= SX1276.Settings.FskPacketHandler.FifoThresh )
                {
                    SX1276ReadFifo( ( RxTxBuffer + SX1276.Settings.FskPacketHandler.NbBytes ), SX1276.Settings.FskPacketHandler.FifoThresh - 1 );
                    SX1276.Settings.FskPacketHandler.NbBytes += SX1276.Settings.FskPacketHandler.FifoThresh - 1;
                }
                else
                {
                    SX1276ReadFifo( ( RxTxBuffer + SX1276.Settings.FskPacketHandler.NbBytes ), SX1276.Settings.FskPacketHandler.Size - SX1276.Settings.FskPacketHandler.NbBytes );
                    SX1276.Settings.FskPacketHandler.NbBytes += ( SX1276.Settings.FskPacketHandler.Size - SX1276.Settings.FskPacketHandler.NbBytes );
                }
                break;
            case MODEM_LORA:
                // Check RxTimeout DIO1 pin state
                //
                // DIO1 irq is setup to be triggered on rsing and falling edges
                // As DIO1 interrupt is triggered when a rising or a falling edge is detected the IRQ handler must
                // verify DIO1 pin state in order to decide if something has to be done.
                // When radio is operating in LoRa reception mode a rising edge must be detected in order to handle the
                // IRQ.
                if( SX1276GetDio1PinState( ) == 0 )
                {
                    break;
                }
                // Sync time out
                TimerStop( &RxTimeoutTimer );
                // Clear Irq
                SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_RXTIMEOUT );

                SX1276.Settings.State = RF_IDLE;
                if( ( RadioEvents != NULL ) && ( RadioEvents->RxTimeout != NULL ) )
                {
                    RadioEvents->RxTimeout( );
                }
                break;
            default:
                break;
            }
            break;
        case RF_TX_RUNNING:
            switch( SX1276.Settings.Modem )
            {
            case MODEM_FSK:
                // Check FIFO level DIO1 pin state
                //
                // As DIO1 interrupt is triggered when a rising or a falling edge is detected the IRQ handler must
                // verify DIO1 pin state in order to decide if something has to be done.
                // When radio is operating in FSK transmission mode a falling edge must be detected in order to handle
                // the IRQ.
                if( SX1276GetDio1PinState( ) == 1 )
                {
                    break;
                }

                // FifoLevel interrupt
                if( ( SX1276.Settings.FskPacketHandler.Size - SX1276.Settings.FskPacketHandler.NbBytes ) > SX1276.Settings.FskPacketHandler.ChunkSize )
                {
                    SX1276WriteFifo( ( RxTxBuffer + SX1276.Settings.FskPacketHandler.NbBytes ), SX1276.Settings.FskPacketHandler.ChunkSize );
                    SX1276.Settings.FskPacketHandler.NbBytes += SX1276.Settings.FskPacketHandler.ChunkSize;
                }
                else
                {
                    // Write the last chunk of data
                    SX1276WriteFifo( RxTxBuffer + SX1276.Settings.FskPacketHandler.NbBytes, SX1276.Settings.FskPacketHandler.Size - SX1276.Settings.FskPacketHandler.NbBytes );
                    SX1276.Settings.FskPacketHandler.NbBytes += SX1276.Settings.FskPacketHandler.Size - SX1276.Settings.FskPacketHandler.NbBytes;
                }
                break;
            case MODEM_LORA:
                break;
            default:
                break;
            }
            break;
        default:
            break;
    }
}

static void SX1276OnDio2Irq( void* context )
{
    switch( SX1276.Settings.State )
    {
        case RF_RX_RUNNING:
            switch( SX1276.Settings.Modem )
            {
            case MODEM_FSK:
                // Checks if DIO4 is connected. If it is not PreambleDetected is set to true.
                if( SX1276.DIO4.port == NULL )
                {
                    SX1276.Settings.FskPacketHandler.PreambleDetected = true;
                }

                if( ( SX1276.Settings.FskPacketHandler.PreambleDetected != 0 ) && ( SX1276.Settings.FskPacketHandler.SyncWordDetected == 0 ) )
                {
                    TimerStop( &RxTimeoutSyncWord );

                    SX1276.Settings.FskPacketHandler.SyncWordDetected = true;

                    SX1276.Settings.FskPacketHandler.RssiValue = -( SX1276Read( REG_RSSIVALUE ) >> 1 );

                    SX1276.Settings.FskPacketHandler.AfcValue = ( int32_t )SX1276ConvertPllStepToFreqInHz( ( ( uint16_t )SX1276Read( REG_AFCMSB ) << 8 ) |
                                                                                                           ( uint16_t )SX1276Read( REG_AFCLSB ) );
                    SX1276.Settings.FskPacketHandler.RxGain = ( SX1276Read( REG_LNA ) >> 5 ) & 0x07;
                }
                break;
            case MODEM_LORA:
                if( SX1276.Settings.LoRa.FreqHopOn == true )
                {
                    // Clear Irq
                    SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL );

                    if( ( RadioEvents != NULL ) && ( RadioEvents->FhssChangeChannel != NULL ) )
                    {
                        RadioEvents->FhssChangeChannel( ( SX1276Read( REG_LR_HOPCHANNEL ) & RFLR_HOPCHANNEL_CHANNEL_MASK ) );
                    }
                }
                break;
            default:
                break;
            }
            break;
        case RF_TX_RUNNING:
            switch( SX1276.Settings.Modem )
            {
            case MODEM_FSK:
                break;
            case MODEM_LORA:
                if( SX1276.Settings.LoRa.FreqHopOn == true )
                {
                    // Clear Irq
                    SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_FHSSCHANGEDCHANNEL );

                    if( ( RadioEvents != NULL ) && ( RadioEvents->FhssChangeChannel != NULL ) )
                    {
                        RadioEvents->FhssChangeChannel( ( SX1276Read( REG_LR_HOPCHANNEL ) & RFLR_HOPCHANNEL_CHANNEL_MASK ) );
                    }
                }
                break;
            default:
                break;
            }
            break;
        default:
            break;
    }
}

static void SX1276OnDio3Irq( void* context )
{
    switch( SX1276.Settings.Modem )
    {
    case MODEM_FSK:
        break;
    case MODEM_LORA:
        if( ( SX1276Read( REG_LR_IRQFLAGS ) & RFLR_IRQFLAGS_CADDETECTED ) == RFLR_IRQFLAGS_CADDETECTED )
        {
            // Clear Irq
            SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_CADDETECTED | RFLR_IRQFLAGS_CADDONE );
            if( ( RadioEvents != NULL ) && ( RadioEvents->CadDone != NULL ) )
            {
                RadioEvents->CadDone( true );
            }
        }
        else
        {
            // Clear Irq
            SX1276Write( REG_LR_IRQFLAGS, RFLR_IRQFLAGS_CADDONE );
            if( ( RadioEvents != NULL ) && ( RadioEvents->CadDone != NULL ) )
            {
                RadioEvents->CadDone( false );
            }
        }
        break;
    default:
        break;
    }
}

static void SX1276OnDio4Irq( void* context )
{
    switch( SX1276.Settings.Modem )
    {
    case MODEM_FSK:
        {
            if( SX1276.Settings.FskPacketHandler.PreambleDetected == false )
            {
                SX1276.Settings.FskPacketHandler.PreambleDetected = true;
            }
        }
        break;
    case MODEM_LORA:
        break;
    default:
        break;
    }
}
