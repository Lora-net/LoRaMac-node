/**
 * @file print_utils.c
 * @author Medad Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-08-05
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "stdio.h"

void print_bytes(void *ptr, int size)
{
    unsigned char *p = ptr;
    int i;
    for (i = 0; i < size; i++)
    {
        // printf("%02hhX ", p[i]);
        printf("%02x ", p[i]);
    }
    printf("\n");
}
