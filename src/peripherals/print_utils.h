/**
 * @file print_utils.h
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-08-05
 * 
 * @copyright Copyright (c) 2021
 * 
 */


#ifdef __cplusplus
extern "C"
{
#endif

#include "LoRaMac.h"
#include "LoRaWAN_config_switcher.h"
void print_current_region(void);
void print_bytes(void *ptr, int size);

const char *get_lorawan_region_string(LoRaMacRegion_t region);
void print_registered_device(registered_devices_t registered_device);


#ifdef __cplusplus
}
#endif
