#include "board.h"
#include "nrf52.h"

#include "app_config.h"
#include "app_timer.h"

#include "nrf_drv_gpiote.h"
#include "nrf_drv_wdt.h"
#include "mem_manager.h"
#include "nrf_log_ctrl.h"

#include <string.h>

#define BATTERY_MAX_LEVEL							4150 // mV
#define BATTERY_MIN_LEVEL							3200 // mV
#define BATTERY_SHUTDOWN_LEVEL						3100 // mV

static uint16_t BatteryVoltage = BATTERY_MAX_LEVEL;

static bool IsExtPower = false;

uint8_t GetBoardPowerSource( void )
{
    if ( IsExtPower )
		return USB_POWER;
	return BATTERY_POWER;
}

uint16_t BoardGetBatteryVoltage( void )
{
	return BatteryVoltage;
}

uint16_t BoardBatteryMeasureVolage( void )
{
	return BatteryVoltage;
}

uint8_t BoardGetBatteryLevel( void )
{
	uint8_t batteryLevel = 0;

	BatteryVoltage = BoardBatteryMeasureVolage( );

	if ( GetBoardPowerSource( ) == USB_POWER )
	{
		batteryLevel = 0;
	}
	else
	{
		if ( BatteryVoltage >= BATTERY_MAX_LEVEL )
		{
			batteryLevel = 254;
		}
		else if ( ( BatteryVoltage > BATTERY_MIN_LEVEL ) && ( BatteryVoltage < BATTERY_MAX_LEVEL ) )
		{
			batteryLevel = ( ( 253 * ( BatteryVoltage - BATTERY_MIN_LEVEL ) ) / ( BATTERY_MAX_LEVEL - BATTERY_MIN_LEVEL ) ) + 1;
		}
		else if ( ( BatteryVoltage > BATTERY_SHUTDOWN_LEVEL ) && ( BatteryVoltage <= BATTERY_MIN_LEVEL ) )
		{
			batteryLevel = 1;
		}
		else // if ( BatteryVoltage <= BATTERY_SHUTDOWN_LEVEL )
		{
			batteryLevel = 255;
		}
	}
	return batteryLevel;
}

uint32_t BoardGetRandomSeed( void )
{
	uint32_t seed = NRF_FICR->DEVICEID[0];
	return seed;
}

void BoardGetUniqueId( uint8_t *id )
{
	uint32_t did[2];
	did[0] = NRF_FICR->DEVICEID[0];
	did[1] = NRF_FICR->DEVICEID[1];
	memcpy(id, did, 8);
}

/*!
 * Nested interrupt counter.
 *
 * \remark Interrupt should only be fully disabled once the value is 0
 */
static int IrqNestLevel = 0;

void BoardDisableIrq( void )
{
    __disable_irq( );
    IrqNestLevel++;
}

void BoardEnableIrq( void )
{
    IrqNestLevel--;
    if( IrqNestLevel == 0 )
    {
        __enable_irq( );
    }
}

void BoardInitMcu( void )
{
	ret_code_t err_code;

	APP_ERROR_CHECK(NRF_LOG_INIT(NULL));

	/* Initialize */
	err_code = app_timer_init();
	APP_ERROR_CHECK(err_code);

	err_code = nrf_drv_gpiote_init();
	APP_ERROR_CHECK(err_code);

	err_code = nrf_mem_init();
	APP_ERROR_CHECK(err_code);

	RtcInit();

    SX1276IoInit( );
}

void BoardProcess( void )
{
	while (NRF_LOG_PROCESS())
		;
}

static nrf_drv_wdt_channel_id	m_channel_id;

void wdt_event_handler(void)
{
    //NOTE: The max amount of time we can spend in WDT interrupt is two cycles of 32768[Hz] clock - after that, reset occurs
}

void wdt_init(void)
{
    nrf_drv_wdt_config_t config = NRF_DRV_WDT_DEAFULT_CONFIG;
    ret_code_t err_code = nrf_drv_wdt_init(&config, wdt_event_handler);
    APP_ERROR_CHECK(err_code);
    err_code = nrf_drv_wdt_channel_alloc(&m_channel_id);
    APP_ERROR_CHECK(err_code);
    nrf_drv_wdt_enable();
}

/**
 * @brief Assert callback.
 *
 * @param[in] id    Fault identifier. See @ref NRF_FAULT_IDS.
 * @param[in] pc    The program counter of the instruction that triggered the fault, or 0 if
 *                  unavailable.
 * @param[in] info  Optional additional information regarding the fault. Refer to each fault
 *                  identifier for details.
 */
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
	wdt_init();
	while (1)
	{ }
}

/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in]   line_num   Line number of the failing ASSERT call.
 * @param[in]   file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t *p_file_name)
{
	app_error_handler(DEAD_BEEF, line_num, p_file_name);
}
