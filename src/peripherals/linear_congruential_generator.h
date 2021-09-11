/**
 * @file linear_congruential_generator.h
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-09-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */



#ifdef __cplusplus
extern "C"
{
#endif

#ifndef LGC_H
#define LGC_H


void init_LGC(int start, int stop, int step);
int LCG(int lower_val, int upper_val);



#endif
#ifdef __cplusplus
}
#endif
