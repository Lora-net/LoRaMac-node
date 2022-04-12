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
#include "iwdg.h"

bool can_tx(LoRaMacRegion_t current_stack_region);

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
    IWDG_reset();

    /**
     * @brief Check if our LoRaWAN stack has been setup in the right region params AND
     * check if we can TX in this location. If we can, then go ahead and do a transmission.
     * 
     */
    if (can_tx(current_stack_region) == true)
    {
        IWDG_reset();

        print_current_region();
        fill_tx_buffer(AppData);
        LmHandlerSend(AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE);

        IWDG_reset();

        ret = true;
    }

    return ret;
}

/**
 * @brief Find out if its ok to TX here. Uses our geofence to check if TX is allowed here,
 * and also checks if our LoRaWAN stack has been configured to the right regional settings
 * 
 * @param current_stack_region The regional settings the LoRaWAN stack has been setup to.
 * @return true  TX is allowed
 * @return false TX not allowed
 */
bool can_tx(LoRaMacRegion_t current_stack_region)
{
    bool ret = false;

    if (get_current_loramac_region() == current_stack_region)
    {
        if (get_current_tx_permission() == TX_OK)
        {
            ret = true;
        }
    }

    return ret;
}
