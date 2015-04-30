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
 * LoRaMac maximum number of channels
 */
#define LORA_MAX_NB_CHANNELS                        16

/*!
 * Minimal datarate that can be used by the node
 */
#define LORAMAC_MIN_DATARATE                        DR_SF12

/*!
 * Minimal datarate that can be used by the node
 */
#define LORAMAC_MAX_DATARATE                        DR_FSK

/*!
 * Default datarate used by the node
 */
#define LORAMAC_DEFAULT_DATARATE                    DR_SF12

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
 * Returns individual channel mask
 *
 * \param[IN] channelIndex Channel index 1 based
 * \retval channelMask
 */
#define LC( channelIndex )            ( uint16_t )( 1 << ( channelIndex - 1 ) )

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
#define DR_SF12                                     0
#define DR_SF11                                     1
#define DR_SF10                                     2
#define DR_SF9                                      3
#define DR_SF8                                      4
#define DR_SF7                                      5
#define DR_SF7H                                     6
#define DR_FSK                                      7

/*!
 * LoRaMac default channels definition
 */
#if defined( USE_BAND_868 )

/*!
 * Second reception window channel definition.
 */
// Channel = { Frequency [Hz], Datarate }
#define RX_WND_2_CHANNEL                                  { 869525000, DR_SF9 }

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
#define LC1                { 868100000, { ( ( DR_SF7 << 4 )  | DR_SF12 ) }, 1 }
#define LC2                { 868300000, { ( ( DR_SF7H << 4 ) | DR_SF12 ) }, 1 }
#define LC3                { 868500000, { ( ( DR_SF7 << 4 )  | DR_SF12 ) }, 1 }
#define LC4                { 867100000, { ( ( DR_SF7 << 4 )  | DR_SF12 ) }, 0 }
#define LC5                { 867300000, { ( ( DR_SF7 << 4 )  | DR_SF12 ) }, 0 }
#define LC6                { 867500000, { ( ( DR_SF7 << 4 )  | DR_SF12 ) }, 0 }
#define LC7                { 867700000, { ( ( DR_SF7 << 4 )  | DR_SF12 ) }, 0 }
#define LC8                { 867900000, { ( ( DR_SF7 << 4 )  | DR_SF12 ) }, 0 }
#define LC9                { 868900000, { ( ( DR_FSK << 4 )  | DR_FSK  ) }, 2 }

#elif defined( USE_BAND_915 )

/*!
 * Second reception window channel definition.
 */
// Channel = { Frequency [Hz], Datarate }
#define RX_WND_2_CHANNEL                                  { 923300000, DR_SF10 }

/*!
 * LoRaMac maximum number of bands
 */
#define LORA_MAX_NB_BANDS                           1

// Band = { DutyCycle, TxMaxPower, LastTxDoneTime, TimeOff }
#define BAND0              { 1, TX_POWER_14_DBM, 0,  0 } //  100.0 %

/*!
 * LoRaMac default channels
 */
// Channel = { Frequency [Hz], { ( ( DrMax << 4 ) | DrMin ) }, Band }
#define LC1                { 902300000, { ( ( DR_SF10 << 4 ) | DR_SF12 ) }, 0 }
#define LC2                { 902500000, { ( ( DR_SF10 << 4 ) | DR_SF12 ) }, 0 }
#define LC3                { 902700000, { ( ( DR_SF10 << 4 ) | DR_SF12 ) }, 0 }

#else
    #error "Please define a frequency band in the compiler options."
#endif

#endif // __LORAMAC_BOARD_H__
