/*!
 * \file      LoRaMacClassBNvm.h
 *
 * \brief     LoRa MAC Class B non-volatile data.
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
 *              (C)2013 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author     Miguel Luis ( Semtech )
 *
 * \author     Daniel Jaeckle ( STACKFORCE )
 *
 * \addtogroup LORAMACCLASSB
 *
 * \{
 */
#ifndef __LORAMACCLASSBNVM_H__
#define __LORAMACCLASSBNVM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/*!
 * LoRaMac Class B Context structure for NVM parameters
 * related to ping slots
 */
typedef struct sLoRaMacClassBPingSlotNvmData
{
    struct sPingSlotCtrlNvm
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
    /*!
     * Number of ping slots
     */
    uint8_t PingNb;
    /*!
     * Period of the ping slots
     */
    uint16_t PingPeriod;
    /*!
     * Reception frequency of the ping slot windows
     */
    uint32_t Frequency;
    /*!
     * Datarate of the ping slot
     */
    int8_t Datarate;
} LoRaMacClassBPingSlotNvmData_t;

/*!
 * LoRaMac Class B Context structure for NVM parameters
 * related to beaconing
 */
typedef struct sLoRaMacClassBBeaconNvmData
{
    struct sBeaconCtrlNvm
    {
        /*!
         * Set if the node has a custom frequency for beaconing and ping slots
         */
        uint8_t CustomFreq          : 1;
    }Ctrl;
    /*!
     * Beacon reception frequency
     */
    uint32_t Frequency;
} LoRaMacClassBBeaconNvmData_t;

/*!
 * LoRaMac Class B Context structure
 */
typedef struct sLoRaMacClassBNvmData
{
    /*!
     * Class B ping slot context
     */
    LoRaMacClassBPingSlotNvmData_t PingSlotCtx;
    /*!
     * Class B beacon context
     */
    LoRaMacClassBBeaconNvmData_t BeaconCtx;
    /*!
     * CRC32 value of the ClassB data structure.
     */
    uint32_t Crc32;
} LoRaMacClassBNvmData_t;

#ifdef __cplusplus
}
#endif

#endif // __LORAMACCLASSBNVM_H__
