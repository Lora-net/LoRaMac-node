#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include <stdio.h>
#include "iwdg.h"
#include "nvmm.h"
#include "ublox.h"
#include "board.h"
}

void IWDG_reset()
{
    //printf("\nSquareRoot_sqrt gets called with parameter: %f.\n", number);
}

uint16_t NvmmRead(uint8_t *dest, uint16_t size, uint16_t offset)
{
    return 0;
}

uint16_t NvmmWrite(uint8_t *src, uint16_t size, uint16_t offset)
{
    return 0;
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

gps_info_t gps_info = {.unix_time = UINT16_MAX, .latest_gps_status = GPS_SUCCESS};

gps_info_t *get_gps_info_ptr(void)
{

    return &gps_info;
}