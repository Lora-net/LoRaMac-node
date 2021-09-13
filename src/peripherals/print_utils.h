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

#include "LoRaMac.h"

void print_bytes(void *ptr, int size);

const char *get_lorawan_region_string(LoRaMacRegion_t region);
