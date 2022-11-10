/**
 * @file eeprom_settings_manager.h
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2022-03-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef __EEPROM_SETTINGS_MANAGER_H__
#define __EEPROM_SETTINGS_MANAGER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdint.h"
#include "stdbool.h"
#include "LoRaWAN_config_switcher.h"

    typedef struct
    {
        uint32_t tx_interval;
        uint32_t Crc32;
    } tx_interval_eeprom_t;

#define N_POLYGONS 23

    typedef struct
    {
        bool values[N_POLYGONS];
        uint32_t Crc32;
    } geofence_settings_t;

    bool update_device_tx_interval_in_eeprom(uint32_t address, uint32_t interval_ms);
    uint32_t read_tx_interval_in_eeprom(uint32_t address, uint32_t default_value);
    bool update_geofence_settings_in_eeprom(uint8_t *settings, uint16_t size);
    void read_geofence_settings_in_eeprom(bool *values);
    void retrieve_eeprom_stored_lorawan_region(void);
    void set_eeprom_stored_lorwan_region(void);
    bool update_device_credentials_to_eeprom(network_keys_t keys, registered_devices_t registered_device);
    uint16_t read_current_keys(network_keys_t *current_keys, registered_devices_t registered_device);
    uint16_t save_lorawan_keys_to_eeprom_with_CRC(network_keys_t *current_keys, registered_devices_t registered_device);
    uint8_t get_settings_crc(void);

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_SETTINGS_MANAGER_H__ */
