#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "ublox.h"
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

gps_status_t get_latest_gps_status(void)
{
    return GPS_SUCCESS;
}