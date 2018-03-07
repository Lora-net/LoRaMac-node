/*!
 * \file      screen.h
 *
 * \brief     Screen management
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
 */
#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <stdint.h>
#include <stdbool.h>
#include "LoRaMac.h"

#define APP_FIRMWARE_VERSION_MAJOR                  0
#define APP_FIRMWARE_VERSION_MINOR                  9
#define APP_FIRMWARE_VERSION_REVISION               1 // 0: Official release, x: RCx releases candidates

#define APP_GITHUB_VERSION_MAJOR                    4
#define APP_GITHUB_VERSION_MINOR                    3
#define APP_GITHUB_VERSION_REVISION                 0 // 0: Official release, x: RCx releases candidates

/*!
 * Enumeration for the screens to be displayed
 */
typedef enum
{
    SCREEN_BOOT = 0,
    SCREEN_JOIN,
    SCREEN_1,
    SCREEN_2,
    SCREEN_3,
    SCREEN_4,
    SCREEN_5,
    SCREEN_GPS,
    SCREEN_LAST,
    SCREEN_UNKNOWN = 0xFF,
}Screen_t;

/*!
 * Structure containing the application status
 */
typedef struct sAppSettings
{
    bool Otaa;
    uint8_t *DevEui;
    uint8_t *AppEui;
    uint8_t *AppKey;

    uint8_t *NwkSKey;
    uint8_t *AppSKey;

    /*!
     * Device address
     */
    uint32_t DevAddr;

    /*!
     * Indicates if the node is sending confirmed or unconfirmed messages
     */
    uint8_t IsTxConfirmed;
    bool DutyCycleOn;
    bool AdrOn;
    DeviceClass_t DeviceClass;

}AppSettings_t;

/*!
 * Structure containing the Uplink status
 */
typedef struct sLoRaMacUplinkStatus
{
    bool StatusUpdated;
    bool UplinkInProgress;
    bool Confirmed;
    bool Acked;

    int8_t TxPower;
    int8_t Datarate;
    uint8_t NbTrials;

    uint16_t UplinkCounter;

    uint8_t Port;
    uint8_t *Buffer;
    uint8_t BufferSize;

    uint16_t Pressure;
    int16_t AltitudeBar;
    int16_t Temperature;

    bool GpsHaxFix;
    int32_t Latitude;
    int32_t Longitude;
    int16_t AltitudeGps;
    uint8_t BatteryLevel;

}LoRaMacUplinkStatus_t;

/*!
 * Structure containing the Downlink status
 */
typedef struct sLoRaMacDownlinkStatus
{
    bool StatusUpdated;
    bool DownlinkInProgress;
    uint8_t RxSlot;
    int16_t Rssi;
    int8_t Snr;
    uint16_t DownlinkCounter;
    bool RxData;
    uint8_t Port;
    uint8_t *Buffer;
    uint8_t BufferSize;
}LoRaMacDownlinkStatus_t;

/*!
 * \brief Initializes the screen display
 */
void ScreenInit( void );

/*!
 * \brief Gets the current active screen ID
 *
 * \retval screenId Current screen ID
 */
Screen_t ScreenGetCurrent( void );

/*!
 * \brief Sets the current screen ID
 *
 * \param current Current screen ID
 */
void ScreenSetCurrent( Screen_t current );

/*!
 * \brief Handles the screens update
 *
 * \param appSettings Current application settings
 * \param uplink      Current uplink status
 * \param downlink    Current downlink status
 */
void ScreenProcess( AppSettings_t *appSettings, LoRaMacUplinkStatus_t *uplink, LoRaMacDownlinkStatus_t *downlink );

#endif // __SCREEN_H__
