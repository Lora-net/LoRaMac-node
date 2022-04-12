/**
 * @file main.h
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-08-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H_1
#define __MAIN_H_1

#ifdef __cplusplus
extern "C"
{
#endif

#include "LmHandler.h"

#ifdef UNITTESTING_LORA
    int run_app(void);
    void do_n_transmissions(uint32_t n_transmissions_todo);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H_1 */