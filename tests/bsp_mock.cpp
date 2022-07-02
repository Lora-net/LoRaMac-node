#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "iwdg.h"
#include "board.h"
#include "bsp.h"
}

/* IWDG init function */
void IWDG_Init(void)
{
    
}

void IWDG_reset()
{
    //printf("\nSquareRoot_sqrt gets called with parameter: %f.\n", number);
}



int32_t HW_GetTemperatureLevel_int(void)
{

    int16_t temperatureDegreeC_Int = 12;

    return temperatureDegreeC_Int;
}

