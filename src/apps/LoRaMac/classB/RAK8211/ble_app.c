#include <stdbool.h>
#include <stdint.h>

#include "app_config.h"
#include "app_timer.h"
#include "nrf_delay.h"
#include "nrf_sdm.h"
#include "nrf_log_ctrl.h"
#include "softdevice_handler.h"

/**@brief Function for initializing Softdevice. */
/*
 */
void BLE_init( void )
{
	ret_code_t err_code;
	nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;

	// Initialize the SoftDevice handler module.
	SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);

	// Fetch the starting address of the application ram. This is needed by the upcoming SoftDevice calls.
	uint32_t ram_start = 0;
	err_code = softdevice_app_ram_start_get(&ram_start);
	APP_ERROR_CHECK(err_code);

	// Overwrite some of the default configurations for the BLE stack.
	ble_cfg_t ble_cfg;

	// Configure the number of custom UUIDS.
	memset(&ble_cfg, 0, sizeof(ble_cfg));
	ble_cfg.common_cfg.vs_uuid_cfg.vs_uuid_count = 0;
	err_code = sd_ble_cfg_set(BLE_COMMON_CFG_VS_UUID, &ble_cfg, ram_start);
	APP_ERROR_CHECK(err_code);

	// Configure the maximum number of connections.
	memset(&ble_cfg, 0, sizeof(ble_cfg));
	ble_cfg.gap_cfg.role_count_cfg.periph_role_count  = BLE_GAP_ROLE_COUNT_PERIPH_DEFAULT;
	ble_cfg.gap_cfg.role_count_cfg.central_role_count = 0;
	ble_cfg.gap_cfg.role_count_cfg.central_sec_count  = 0;
	err_code = sd_ble_cfg_set(BLE_GAP_CFG_ROLE_COUNT, &ble_cfg, ram_start);
	APP_ERROR_CHECK(err_code);

	memset(&ble_cfg, 0, sizeof(ble_cfg));
	ble_cfg.gatts_cfg.attr_tab_size.attr_tab_size = BLE_GATTS_ATTR_TAB_SIZE_MIN;
	err_code = sd_ble_cfg_set(BLE_GATTS_CFG_ATTR_TAB_SIZE, &ble_cfg, ram_start);
	APP_ERROR_CHECK(err_code);

	// Enable BLE stack.
	err_code = softdevice_enable(&ram_start);
	APP_ERROR_CHECK(err_code);
}

#define FPU_EXCEPTION_MASK	0x0000009F	//!< FPU exception mask used to clear exceptions in FPSCR register.

/**@brief Function for doing power management. */
void power_manage(void)
{
	__set_FPSCR(__get_FPSCR()  & ~(FPU_EXCEPTION_MASK));
	(void) __get_FPSCR();
	NVIC_ClearPendingIRQ(FPU_IRQn);
	ret_code_t err_code = sd_app_evt_wait();
	APP_ERROR_CHECK(err_code);
}
