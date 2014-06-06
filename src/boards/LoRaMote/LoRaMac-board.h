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
 * \remark DCycle field isn't used. This channel is Rx only
 */
// Channel = { Frequency [Hz], Datarate }
#define RX_WND_2_CHANNEL                                  { 865525000, DR_SF9 }

/*!
 * LoRaMac default channels
 */
// Channel = { Frequency [Hz], { ( ( DrMax << 4 ) | DrMin ) }, DCycle }
#define LC1                { 868100000, { ( ( DR_SF7 << 4 ) | DR_SF12 ) }, 0 }
#define LC2                { 868300000, { ( ( DR_SF7 << 4 ) | DR_SF12 ) }, 0 }
#define LC3                { 868500000, { ( ( DR_SF7 << 4 ) | DR_SF12 ) }, 0 }

#elif defined( USE_BAND_915 )

/*!
 * Second reception window channel definition.
 * \remark DCycle field isn't used. This channel is Rx only
 */
// Channel = { Frequency [Hz], Datarate }
#define RX_WND_2_CHANNEL                                  { 903700000, DR_SF9 }

/*!
 * LoRaMac default channels
 */
// Channel = { Frequency [Hz], { ( ( DrMax << 4 ) | DrMin ) }, DCycle }
#define LC1                { 902700000, { ( ( DR_SF10 << 4 ) | DR_SF12 ) }, 0 }
#define LC2                { 902900000, { ( ( DR_SF10 << 4 ) | DR_SF12 ) }, 0 }
#define LC3                { 903100000, { ( ( DR_SF10 << 4 ) | DR_SF12 ) }, 0 }

#else
    #error "Please define a frequency band in the compiler options."
#endif

#endif // __LORAMAC_BOARD_H__
