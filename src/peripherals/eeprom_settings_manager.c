/**
 * @file eeprom_settings_manager.c
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2022-03-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "eeprom_settings_manager.h"
#include "config.h"
#include "utilities.h"
#include "LoRaMac.h"
#include "nvmm.h"
#include "geofence.h"
#include "string.h"
#include "bsp.h"
#include "eeprom_settings_manager.h"

/**
 * @brief Get the eeprom stored lorwan region
 * 
 */
void retrieve_eeprom_stored_lorawan_region()
{
#if USE_NVM_STORED_LORAWAN_REGION

	LoRaMacRegion_t eeprom_region;
	NvmmRead((void *)&eeprom_region, sizeof(LoRaMacRegion_t), LORAMAC_REGION_EEPROM_ADDR);

	/**
	 * @brief Reject invalid regions from a corrupt EEPROM 
	 */
	if (eeprom_region <= LORAMAC_REGION_RU864)
	{
		set_current_loramac_region(eeprom_region);
	}

#endif
}

bool update_geofence_settings_in_eeprom(uint8_t *settings_bytes, uint16_t size)
{
	geofence_settings_t settings = {0};

	// copy over bytes to settings struct
	memcpy(settings.values, settings_bytes, size);

	// set CRC32 for the struct
	settings.Crc32 = Crc32((uint8_t *)&settings, sizeof(geofence_settings_t) - sizeof(uint32_t));

	// write geofence settings to EEPROM
	uint16_t bytes_changed = NvmmUpdate((uint8_t *)&settings, TX_PERMISSIONS_LEN, TX_PERMISSIONS_ADDR);

	return bytes_changed == 0 ? false : true;
}

/**
 * @brief Reads and initialised the geofence module with the tx enable/disable settings
 * from EEPROM. Fills passed buffer with the settings. Ensure buffer passed is 
 * of size N_POLYGONS * sizeof(bool)
 * 
 * @param values Pointer to buffer that will be filled with the bool enable/disable setting
 */
void read_geofence_settings_in_eeprom(bool *values)
{
	// read settings stored in EEPROM
	geofence_settings_t geofence_settings;
	NvmmRead((uint8_t *)&geofence_settings, TX_PERMISSIONS_LEN, TX_PERMISSIONS_ADDR);

	// Use eeprom stored values only if CRC is correct. Else assume the default settings
	if (is_crc_correct(sizeof(geofence_settings_t), (void *)&geofence_settings))
	{
		memcpy(values, geofence_settings.values, N_POLYGONS * sizeof(bool));
	}
	else
	{

		bool default_geofence_settings[N_POLYGONS] = {true, true, true, true, true, true,
													  true, true, true, true, true, true,
													  true, true, true, true, true, true,
													  true, true, true, false, false};

		memcpy(values, default_geofence_settings, N_POLYGONS * sizeof(bool));
	}

	geofence_init_with_settings(values);
}

/**
 * @brief Update tx interval in EEPROM
 * 
 * @param interval_ms interval in milliseconds
 * @return true if succesfully updated
 * @return false if not successful in update
 */
bool update_device_tx_interval_in_eeprom(uint32_t address, uint32_t interval_ms)
{

	tx_interval_eeprom_t tx_interval_with_crc = {.tx_interval = interval_ms};
	tx_interval_with_crc.Crc32 = Crc32((uint8_t *)&tx_interval_with_crc, sizeof(tx_interval_eeprom_t) - sizeof(tx_interval_with_crc.Crc32));

	// Now write current keys for this network(including frame count) to EEPROM into the right place in the EEPROM
	uint16_t bytes_changed = NvmmUpdate((uint8_t *)&tx_interval_with_crc, sizeof(tx_interval_eeprom_t), address);

	return bytes_changed == 0 ? false : true;
}

/**
 * @brief Read the eeprom tx interval stored in EEPROM. Read only if value crc matches. Otherwise,
 * return the default value
 * 
 * @param address Address in eeprom where value is stored
 * @param default_value default value if there is nothing in EEPROM
 * @return uint32_t the value in eeprom(or default if it was not in EEPROM)
 */
uint32_t read_tx_interval_in_eeprom(uint32_t address, uint32_t default_value)
{

	tx_interval_eeprom_t tx_interval_with_crc;
	NvmmRead((uint8_t *)&tx_interval_with_crc, sizeof(tx_interval_eeprom_t), address);

	if (is_crc_correct(sizeof(tx_interval_with_crc), &tx_interval_with_crc) == false)
	{
		tx_interval_with_crc.tx_interval = default_value;
	}

	return tx_interval_with_crc.tx_interval;
}

/**
 * @brief Update the eeprom stored lorwan region
 * 
 */
void set_eeprom_stored_lorwan_region()
{
	LoRaMacRegion_t current_region = get_current_loramac_region();

	NvmmUpdate((void *)&current_region, sizeof(LoRaMacRegion_t), LORAMAC_REGION_EEPROM_ADDR);
}

/**
 * @brief Write passed in keys to EEPROM, in the location allocated for the registered_device
 * 
 * @param keys keys to write
 * @param registered_device which key to write
 * @return bool returns if eeprom values changed
 */
bool update_device_credentials_to_eeprom(network_keys_t keys, registered_devices_t registered_device)
{
	// Now write current keys for this network(including frame count) to EEPROM into the right place in the EEPROM
	uint16_t bytes_changed = NvmmUpdate((uint8_t *)&keys, sizeof(network_keys_t), registered_device * sizeof(network_keys_t));

	return bytes_changed == 0 ? false : true;
}

/**
 * @brief reads the keys from EEPROM. if eeprom keys are corrupted, 
 * 
 * @param current_keys Pointer to the structure to fill with keys
 * @param registered_device the registered device
 * @return uint16_t number of bytes read
 */
uint16_t read_current_keys(network_keys_t *current_keys, registered_devices_t registered_device)
{
	uint16_t bytes_read = NvmmRead((uint8_t *)current_keys, sizeof(network_keys_t), registered_device * sizeof(network_keys_t));

	// check if NVM crc matches for fcount
	// if crc fails, frame count = 0
	// else, use the EEPROM stored frame count
	if (is_crc_correct(sizeof(*current_keys), current_keys) == false)
	{
		*current_keys = get_current_network_keys();
	}

	return bytes_read;
}

/**
 * @brief 
 * 
 * @param current_keys 
 * @param registered_device 
 * @return uint16_t 
 */
uint16_t save_lorawan_keys_to_eeprom_with_CRC(network_keys_t *current_keys, registered_devices_t registered_device)
{
	// now update CRC before writing to EEPROM
	current_keys->Crc32 = Crc32((uint8_t *)current_keys, sizeof(*current_keys) - sizeof(current_keys->Crc32));

	// Now write current keys for this network(including frame count) to EEPROM into the right place in the EEPROM
	return update_device_credentials_to_eeprom(*current_keys, registered_device);
}
