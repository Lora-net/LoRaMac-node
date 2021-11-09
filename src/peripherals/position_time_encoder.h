/**
 * @file position_time_encoder.h
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-10-30
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef POSITION_TIME_H
#define POSITION_TIME_H

#include "bsp.h"
#include "ublox.h"

    time_pos_fix_t encode_time_pos(gps_info_t gps_info);
    gps_info_t decode_time_pos(time_pos_fix_t time_pos_fix);
    uint32_t minutes_since_epoch_to_unix_time(uint32_t minutes_since_epoch);

#endif
#ifdef __cplusplus
}
#endif
