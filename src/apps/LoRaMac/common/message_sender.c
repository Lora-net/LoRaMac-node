/**
 * @file message_sender.c
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-10-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "message_sender.h"
#include "bsp.h"
#include "playback.h"
#include "geofence.h"
#include "print_utils.h"
#include "callbacks.h"

bool sensor_read_and_send(LmHandlerAppData_t *AppData, LoRaMacRegion_t current_stack_region)
{

    bool ret = false;
    /**
	 * @brief Compare old region vs new region. If the new region is different from the old one, 
	 * then the lormac stack is wrongly configured. So just avoid transmitting, and the next time
	 * this function is called, the stack and frequency parameters would have been initialised correctly
	 * for this region.
	 */

    BSP_sensor_Read(); /* reading sensors and GPS. This could take up to 3 minutes */

    if (get_current_loramac_region() == current_stack_region)
    {
        print_current_region();
        fill_tx_buffer(AppData);
        LmHandlerSend(AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE);
        ret = true;
    }

    return ret;
}
