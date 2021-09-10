/**
 * @file struct.c
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "struct.h"

//Given a spot in the payload array, extract four bytes and build a long
uint32_t extractLong_from_buff(uint8_t spotToStart, uint8_t *buff)
{
    uint32_t val = 0;
    val |= (uint32_t)buff[spotToStart + 0] << 8 * 0;
    val |= (uint32_t)buff[spotToStart + 1] << 8 * 1;
    val |= (uint32_t)buff[spotToStart + 2] << 8 * 2;
    val |= (uint32_t)buff[spotToStart + 3] << 8 * 3;
    return (val);
}
