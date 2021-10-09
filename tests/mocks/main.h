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

#ifdef UNITTESTING_LORA
    int run_app(void);
    int setup_board(void);
    bool run_loop_once(void);
    int init_loramac_stack_and_tx_scheduling(bool use_default_tx_interval);
    void loop(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H_1 */
