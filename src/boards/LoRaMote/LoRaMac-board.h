/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: LoRa MAC layer board dependent definitions

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __LORAMAC_BOARD_H__
#define __LORAMAC_BOARD_H__

/*!
 * Returns individual channel mask
 *
 * \param[IN] channelIndex Channel index 1 based
 * \retval channelMask
 */
#define LC( channelIndex )            ( uint16_t )( 1 << ( channelIndex - 1 ) )

#if defined( USE_BAND_868 )

/*!
 * LoRaMac maximum number of channels
 */
#define LORA_MAX_NB_CHANNELS                        16

/*!
 * Minimal datarate that can be used by the node
 */
#define LORAMAC_TX_MIN_DATARATE                     DR_0

/*!
 * Minimal datarate that can be used by the node
 */
#define LORAMAC_TX_MAX_DATARATE                     DR_7

/*!
 * Minimal datarate that can be used by the node
 */
#define LORAMAC_RX_MIN_DATARATE                     DR_0

/*!
 * Minimal datarate that can be used by the node
 */
#define LORAMAC_RX_MAX_DATARATE                     DR_7

/*!
 * Default datarate used by the node
 */
#define LORAMAC_DEFAULT_DATARATE                    DR_0

/*!
 * Minimal Rx1 receive datarate offset
 */
#define LORAMAC_MIN_RX1_DR_OFFSET                   0

/*!
 * Maximal Rx1 receive datarate offset
 */
#define LORAMAC_MAX_RX1_DR_OFFSET                   5

/*!
 * Minimal Tx output power that can be used by the node
 */
#define LORAMAC_MIN_TX_POWER                        TX_POWER_02_DBM

/*!
 * Minimal Tx output power that can be used by the node
 */
#define LORAMAC_MAX_TX_POWER                        TX_POWER_20_DBM

/*!
 * Default Tx output power used by the node
 */
#define LORAMAC_DEFAULT_TX_POWER                    TX_POWER_14_DBM

/*!
 * LoRaMac TxPower definition
 */
#define TX_POWER_20_DBM                             0
#define TX_POWER_14_DBM                             1
#define TX_POWER_11_DBM                             2
#define TX_POWER_08_DBM                             3
#define TX_POWER_05_DBM                             4
#define TX_POWER_02_DBM                             5

/*!
 * LoRaMac datarates definition
 */
#define DR_0                                        0  // SF12 - BW125
#define DR_1                                        1  // SF11 - BW125
#define DR_2                                        2  // SF10 - BW125
#define DR_3                                        3  // SF9  - BW125
#define DR_4                                        4  // SF8  - BW125
#define DR_5                                        5  // SF7  - BW125
#define DR_6                                        6  // SF7  - BW250
#define DR_7                                        7  // FSK

/*!
 * Second reception window channel definition.
 */
// Channel = { Frequency [Hz], Datarate }
#define RX_WND_2_CHANNEL                                  { 869525000, DR_0 }

/*!
 * LoRaMac maximum number of bands
 */
#define LORA_MAX_NB_BANDS                           5

/*!
 * LoRaMac EU868 default bands
 */
typedef enum
{
    BAND_G1_0,
    BAND_G1_1,
    BAND_G1_2,
    BAND_G1_3,
    BAND_G1_4,
}BandId_t;

// Band = { DutyCycle, TxMaxPower, LastTxDoneTime, TimeOff }
#define BAND0              { 100 , TX_POWER_14_DBM, 0,  0 } //  1.0 %
#define BAND1              { 100 , TX_POWER_14_DBM, 0,  0 } //  1.0 %
#define BAND2              { 1000, TX_POWER_14_DBM, 0,  0 } //  0.1 %
#define BAND3              { 10  , TX_POWER_14_DBM, 0,  0 } // 10.0 %
#define BAND4              { 100 , TX_POWER_14_DBM, 0,  0 } //  1.0 %

/*!
 * LoRaMac default channels
 */
// Channel = { Frequency [Hz], { ( ( DrMax << 4 ) | DrMin ) }, Band }
#define LC1                { 868100000, { ( ( DR_5 << 4 ) | DR_0 ) }, 1 }
#define LC2                { 868300000, { ( ( DR_5 << 4 ) | DR_0 ) }, 1 }
#define LC3                { 868500000, { ( ( DR_5 << 4 ) | DR_0 ) }, 1 }

/*!
 * LoRaMac duty cycle for the join procedure
 */
#define JOIN_DC            1000

/*!
 * LoRaMac channels which are allowed for the join procedure
 */
#define JOIN_CHANNELS      ( uint16_t )( LC( 1 ) | LC( 2 ) | LC( 3 ) )

#elif defined( USE_BAND_915 ) || defined( USE_BAND_915_HYBRID )

/*!
 * LoRaMac maximum number of channels
 */
#define LORA_MAX_NB_CHANNELS                        72

/*!
 * Minimal datarate that can be used by the node
 */
#define LORAMAC_TX_MIN_DATARATE                     DR_0

/*!
 * Minimal datarate that can be used by the node
 */
#define LORAMAC_TX_MAX_DATARATE                     DR_4

/*!
 * Minimal datarate that can be used by the node
 */
#define LORAMAC_RX_MIN_DATARATE                     DR_8

/*!
 * Minimal datarate that can be used by the node
 */
#define LORAMAC_RX_MAX_DATARATE                     DR_13

/*!
 * Default datarate used by the node
 */
#define LORAMAC_DEFAULT_DATARATE                    DR_0

/*!
 * Minimal Rx1 receive datarate offset
 */
#define LORAMAC_MIN_RX1_DR_OFFSET                   0

/*!
 * Maximal Rx1 receive datarate offset
 */
#define LORAMAC_MAX_RX1_DR_OFFSET                   3

/*!
 * Minimal Tx output power that can be used by the node
 */
#define LORAMAC_MIN_TX_POWER                        TX_POWER_10_DBM

/*!
 * Minimal Tx output power that can be used by the node
 */
#define LORAMAC_MAX_TX_POWER                        TX_POWER_30_DBM

/*!
 * Default Tx output power used by the node
 */
#define LORAMAC_DEFAULT_TX_POWER                    TX_POWER_20_DBM

/*!
 * LoRaMac TxPower definition
 */
#define TX_POWER_30_DBM                             0
#define TX_POWER_28_DBM                             1
#define TX_POWER_26_DBM                             2
#define TX_POWER_24_DBM                             3
#define TX_POWER_22_DBM                             4
#define TX_POWER_20_DBM                             5
#define TX_POWER_18_DBM                             6
#define TX_POWER_16_DBM                             7
#define TX_POWER_14_DBM                             8
#define TX_POWER_12_DBM                             9
#define TX_POWER_10_DBM                             10

/*!
 * LoRaMac datarates definition
 */
#define DR_0                                        0  // SF10 - BW125 |
#define DR_1                                        1  // SF9  - BW125 |
#define DR_2                                        2  // SF8  - BW125 +-> Up link
#define DR_3                                        3  // SF7  - BW125 |
#define DR_4                                        4  // SF8  - BW500 |
#define DR_5                                        5  // RFU
#define DR_6                                        6  // RFU
#define DR_7                                        7  // RFU
#define DR_8                                        8  // SF12 - BW500 |
#define DR_9                                        9  // SF11 - BW500 |
#define DR_10                                       10 // SF10 - BW500 |
#define DR_11                                       11 // SF9  - BW500 |
#define DR_12                                       12 // SF8  - BW500 +-> Down link
#define DR_13                                       13 // SF7  - BW500 |
#define DR_14                                       14 // RFU          |
#define DR_15                                       15 // RFU          |

/*!
 * Second reception window channel definition.
 */
// Channel = { Frequency [Hz], Datarate }
#define RX_WND_2_CHANNEL                                  { 923300000, DR_8 }

/*!
 * LoRaMac maximum number of bands
 */
#define LORA_MAX_NB_BANDS                           1

// Band = { DutyCycle, TxMaxPower, LastTxDoneTime, TimeOff }
#define BAND0              { 1, TX_POWER_20_DBM, 0,  0 } //  100.0 %

/*!
 * LoRaMac default channels
 */
// Channel = { Frequency [Hz], { ( ( DrMax << 4 ) | DrMin ) }, Band }
/*
 * US band channels are initialized using a loop in LoRaMacInit function
 * \code
 * // 125 kHz channels
 * for( uint8_t i = 0; i < LORA_MAX_NB_CHANNELS - 8; i++ )
 * {
 *     Channels[i].Frequency = 902.3e6 + i * 200e3;
 *     Channels[i].DrRange.Value = ( DR_3 << 4 ) | DR_0;
 *     Channels[i].Band = 0;
 * }
 * // 500 kHz channels
 * for( uint8_t i = LORA_MAX_NB_CHANNELS - 8; i < LORA_MAX_NB_CHANNELS; i++ )
 * {
 *     Channels[i].Frequency = 903.0e6 + ( i - ( LORA_MAX_NB_CHANNELS - 8 ) ) * 1.6e6;
 *     Channels[i].DrRange.Value = ( DR_4 << 4 ) | DR_4;
 *     Channels[i].Band = 0;
 * }
 * \endcode
 */
#else
    #error "Please define a frequency band in the compiler options."
#endif

#endif // __LORAMAC_BOARD_H__
