/**
 * @file struct.h
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-10
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef STRUCT_H
#define STRUCT_H

#include "stdint.h"


uint32_t extractLong_from_buff(uint8_t spotToStart, uint8_t *buff);


#endif
#ifdef __cplusplus
}
#endif
