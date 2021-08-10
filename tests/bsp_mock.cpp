#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include <stdio.h>
#include "iwdg.h"
#include "ublox.h"
#include "board.h"
#include "bsp.h"
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

uint32_t BoardGetBatteryVoltage(void)
{
    return 3323;
}

gps_status_t get_location_fix(uint32_t timeout)
{
    return GPS_SUCCESS;
}

/* 
 * sets up gps by putting in airbourne mode, setting to use GPS satellites only, turning off NMEA
 * Needs TO BE REFACTORED TO TIME OUT OR EXIT IF NO MESSAGED IS ReCEIVED BACK!
 */
gps_status_t setup_GPS()
{
    return GPS_SUCCESS;
}

/* Get solar voltage when under load from GPS */
uint16_t get_load_solar_voltage()
{
    return 2323;
}

gps_info_t get_latest_gps_info()
{
    auto returnValue = mock().actualCall(__func__).returnPointerValue();
    return *(gps_info_t *)(returnValue);
}
