/**
 * @file deep_sleep_delay.h
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-20
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "stdint.h"


/**
 * @brief Delay in deep sleep for ms milliseconds
 * 
 * @param ms 
 */
void DeepSleepDelayMs( uint32_t ms );

void DeepSleepDelayMsInit();
