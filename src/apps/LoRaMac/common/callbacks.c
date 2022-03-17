/**
 * @file callbacks.c
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "callbacks.h"
#include "config.h"
#include "stdio.h"
#include "print_utils.h"
#include "board.h"
#include "iwdg.h"
#include <stdlib.h>
#include "NvmDataMgmt.h"
#include "string.h"
#include "ublox.h"
#include "soft-se-hal.h"
#include "nvmm.h"
#include "struct.h"
#include "eeprom_settings_manager.h"

typedef struct
{
    network_keys_t keys;
    registered_devices_t registered_device;
} uplink_key_setter_message_t;

uplink_key_setter_message_t uplink_key_setter_message;

void OnNvmDataChange(LmHandlerNvmContextStates_t state, uint16_t size)
{
    DisplayNvmDataChange(state, size);

    /**
     * @brief Only turn back on the GPS during the Store function call. This is the last
     * event in a Class A transmission, so its now safe to turn back on the GPS. The last 
     * RX window has closed by now.
     * 
     */
    if (state == LORAMAC_HANDLER_NVM_STORE)
    {
#if GPS_ENABLED
        setup_GPS();
#endif
    }
}

void OnNetworkParametersChange(CommissioningParams_t *params)
{
    DisplayNetworkParametersUpdate(params);
}

void OnMacMcpsRequest(LoRaMacStatus_t status, McpsReq_t *mcpsReq, TimerTime_t nextTxIn)
{
    DisplayMacMcpsRequestUpdate(status, mcpsReq, nextTxIn);
}

void OnMacMlmeRequest(LoRaMacStatus_t status, MlmeReq_t *mlmeReq, TimerTime_t nextTxIn)
{
    DisplayMacMlmeRequestUpdate(status, mlmeReq, nextTxIn);
}

void OnJoinRequest(LmHandlerJoinParams_t *params)
{
    DisplayJoinRequestUpdate(params);
    if (params->Status == LORAMAC_HANDLER_ERROR)
    {
        LmHandlerJoin();
    }
    else
    {
        LmHandlerRequestClass(LORAWAN_DEFAULT_CLASS);
    }
}

void OnTxData(LmHandlerTxParams_t *params)
{
    DisplayTxUpdate(params);
}

void OnClassChange(DeviceClass_t deviceClass)
{
    DisplayClassUpdate(deviceClass);

    // Inform the server as soon as possible that the end-device has switched to ClassB
    LmHandlerAppData_t appData =
        {
            .Buffer = NULL,
            .BufferSize = 0,
            .Port = 0};
    LmHandlerSend(&appData, LORAMAC_HANDLER_UNCONFIRMED_MSG);
}

void OnBeaconStatusChange(LoRaMAcHandlerBeaconParams_t *params)
{
    switch (params->State)
    {
    case LORAMAC_HANDLER_BEACON_RX:
    {
        break;
    }
    case LORAMAC_HANDLER_BEACON_LOST:
    case LORAMAC_HANDLER_BEACON_NRX:
    {
        break;
    }
    default:
    {
        break;
    }
    }

    DisplayBeaconUpdate(params);
}

#if (LMH_SYS_TIME_UPDATE_NEW_API == 1)
void OnSysTimeUpdate(bool isSynchronized, int32_t timeCorrection)
{
}
#else
void OnSysTimeUpdate(void)
{
}
#endif

void print_board_info()
{

    const Version_t appVersion = {.Fields.Major = 1, .Fields.Minor = 0, .Fields.Patch = 0};
    const Version_t gitHubVersion = {.Fields.Major = 4, .Fields.Minor = 4, .Fields.Patch = 7};
    DisplayAppInfo("periodic-uplink-lpp",
                   &appVersion,
                   &gitHubVersion);
}

void fill_tx_buffer(LmHandlerAppData_t *AppData)
{

    PicoTrackerAppData_t data = prepare_tx_buffer();

    AppData->Port = LORAWAN_APP_PORT;
    AppData->Buffer = data.Buffer;
    AppData->BufferSize = data.BufferSize;

    // Print out buffer for debug
    printf("Buffer to tx (%d bytes):\n", AppData->BufferSize);
    print_bytes(AppData->Buffer, AppData->BufferSize);
}

int setup_board()
{
    /* Get reset cause for diagnosis */
    reset_cause_t reset_cause = reset_cause_get();

    /* Initialising board and peripherals */
    BoardInitMcu();

    /* Print reset cause after print function initialised */
    printf("\n\nThe system reset cause is \"%s\"\n", reset_cause_get_name(reset_cause));

#if DISABLE_SERIAL_OUTPUT
    printf("DISABLING SERIAL(UART) DEBUG OUTPUT. NO MORE DATA WILL APPEAR ON YOUR SERIAL TERMINAL!\n");

    disable_serial_output();
#endif

    /* Get random seed and set it */
    srand1(SoftSeHalGetRandomNumber());

    BoardInitPeriph();

    /* Print board info */
    print_board_info();

    /* Get initial GPS fix for setting loramac region */
    retrieve_eeprom_stored_lorawan_region();

    return EXIT_SUCCESS;
}

void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
    DisplayRxUpdate(appData, params);

    switch (appData->Port)
    {

    case DOWNLINK_CONFIG_PORT: // Poll specific range of past position fixes by date range.
    {

        printf("Received data to poll date range: ");
        print_bytes(appData->Buffer, appData->BufferSize);
        manage_incoming_instruction(appData->Buffer);
    }
    break;

    case CHANGE_KEYS_PORT: // Send up a set of LoRaWAN keys, and which keys to change in EEPROM
    {

        printf("Received data to CHANGE_KEYS:  ");
        print_bytes(appData->Buffer, appData->BufferSize);
        memcpy(&uplink_key_setter_message, appData->Buffer, appData->BufferSize);                                         // copy the bytes to the struct
        update_device_credentials_to_eeprom(uplink_key_setter_message.keys, uplink_key_setter_message.registered_device); // update keys in EEPROM. make it return success
    }
    break;

    case WIPE_EEPROM_PORT: // send any data to the WIPE_EEPROM_PORT and it will wipe out the EEPROM
    {

        printf("Received data to WIPE EEPROM:  ");
        print_bytes(appData->Buffer, appData->BufferSize);

        uint32_t start = extractLong_from_buff(0, appData->Buffer);
        uint32_t end = extractLong_from_buff(4, appData->Buffer);
        EEPROM_Wipe(start, end);
    }
    break;

    case CHANGE_TX_INTERVAL_PORT: // Change the transmit interval
    {

        printf("Received data to CHANGE TX INTERVAL:  ");
        print_bytes(appData->Buffer, appData->BufferSize);

        uint32_t target_tx_interval;

        memcpy(&target_tx_interval, appData->Buffer, appData->BufferSize); // copy the bytes to the struct

        update_device_tx_interval_in_eeprom(TX_INTERVAL_EEPROM_ADDRESS, target_tx_interval);
    }
    break;

    case CHANGE_GPS_SEARCH_TIME_PORT: // Change the transmit interval
    {

        printf("Received data to GPS search time:  ");
        print_bytes(appData->Buffer, appData->BufferSize);

        uint32_t target_tx_interval;

        memcpy(&target_tx_interval, appData->Buffer, appData->BufferSize); // copy the bytes to the struct

        update_device_tx_interval_in_eeprom(GPS_SEARCH_TIME_ADDR, target_tx_interval);
    }
    break;

    case CHANGE_GEOFENCE_SETTINGS: // Change which countries we can tx on or not.
    {

        printf("Received data to change Geofence settings:  ");
        print_bytes(appData->Buffer, appData->BufferSize);


        update_geofence_settings_in_eeprom(appData->Buffer, appData->BufferSize);
    }
    break;

    default:
        break;
    }
}
