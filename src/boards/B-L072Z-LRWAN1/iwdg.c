/**
  ******************************************************************************
  * File Name          : IWDG.c
  * Description        : This file provides code for the configuration
  *                      of the IWDG instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "iwdg.h"
#include "stm32l0xx.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

IWDG_HandleTypeDef hiwdg;

/* IWDG init function */
void IWDG_Init(void)
{
  /* Reset time is:
	 * Prescaler = 256
	 * Reload = 4095
	 * T_reset = Prescaler * Reload / 32000
	 * In this case, comes to 32.76 seconds
	 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_256;
  hiwdg.Init.Reload = 4095;
  hiwdg.Init.Window = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    return; // not much to do here
  }
}

/* USER CODE BEGIN 1 */

/* IWDG init function */
void MX_IWDG_DeInit(void)
{
  // Not implemented
}

void IWDG_reset()
{
  HAL_IWDG_Refresh(&hiwdg);
}

/**
 * @brief Obtain the STM32 system reset cause
 * @param None
 * @return reset_cause_t The system reset cause
 */
reset_cause_t reset_cause_get(void)
{

  reset_cause_t reset_cause;

  if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST))
  {
    reset_cause = RESET_CAUSE_LOW_POWER_RESET;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST))
  {
    reset_cause = RESET_CAUSE_WINDOW_WATCHDOG_RESET;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
  {
    reset_cause = RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
  {
    reset_cause = RESET_CAUSE_SOFTWARE_RESET; // This reset is induced by calling the ARM CMSIS `NVIC_SystemReset()` function!
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PORRST))
  {
    reset_cause = RESET_CAUSE_POWER_ON_POWER_DOWN_RESET;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST))
  {
    reset_cause = RESET_CAUSE_OPTIONS_BYTES_LOADING_RESET;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_FWRST))
  {
    reset_cause = RESET_CAUSE_FIREWALL_RESET;
  }
  else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST))
  {
    reset_cause = RESET_CAUSE_EXTERNAL_RESET_PIN_RESET;
  }
  else
  {
    reset_cause = RESET_CAUSE_UNKNOWN;
  }

  // Clear all the reset flags or else they will remain set during future resets until system power is fully removed.
  __HAL_RCC_CLEAR_RESET_FLAGS();

  return reset_cause;
}

/**
 * @brief 
 * @param[in] reset_cause The previously-obtained system reset cause
 * @return A null-terminated ASCII name string describing the system reset cause
 */
const char *reset_cause_get_name(reset_cause_t reset_cause)
{
  const char *reset_cause_name = "TBD";

  switch (reset_cause)
  {
  case RESET_CAUSE_UNKNOWN:
    reset_cause_name = "UNKNOWN";
    break;
  case RESET_CAUSE_LOW_POWER_RESET:
    reset_cause_name = "LOW_POWER_RESET";
    break;
  case RESET_CAUSE_WINDOW_WATCHDOG_RESET:
    reset_cause_name = "WINDOW_WATCHDOG_RESET";
    break;
  case RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET:
    reset_cause_name = "INDEPENDENT_WATCHDOG_RESET";
    break;
  case RESET_CAUSE_SOFTWARE_RESET:
    reset_cause_name = "SOFTWARE_RESET";
    break;
  case RESET_CAUSE_POWER_ON_POWER_DOWN_RESET:
    reset_cause_name = "POWER-ON_RESET (POR) / POWER-DOWN_RESET (PDR)";
    break;
  case RESET_CAUSE_EXTERNAL_RESET_PIN_RESET:
    reset_cause_name = "EXTERNAL_RESET_PIN_RESET";
    break;
  case RESET_CAUSE_OPTIONS_BYTES_LOADING_RESET:
    reset_cause_name = "RESET_CAUSE_OPTIONS_BYTES_LOADING_RESET";
    break;
  case RESET_CAUSE_FIREWALL_RESET:
    reset_cause_name = "RESET_CAUSE_FIREWALL_RESET";
    break;
  case RESET_CAUSE_BROWNOUT_RESET:
    reset_cause_name = "BROWNOUT_RESET (BOR)";
    break;
  }

  return reset_cause_name;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
