/**
 * @file message_sender.h
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-10-27
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#ifndef __MESSAGE_SENDER_H__
#define __MESSAGE_SENDER_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "LmHandlerTypes.h"
#include "stdbool.h"

    /**
     * @brief Returns true if successfully sent. Otherwise, return false
     * 
     * @param AppData pointer to the datastructure holding the data to be transmitted
     * @param current_stack_region region the stack has been initialised with
     * @return true 
     * @return false 
     */
    bool sensor_read_and_send(LmHandlerAppData_t *AppData, LoRaMacRegion_t current_stack_region);

#ifdef __cplusplus
}
#endif

#endif
