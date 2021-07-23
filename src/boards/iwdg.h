/**
  ******************************************************************************
  * File Name          : IWDG.h
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __iwdg_H
#define __iwdg_H
#ifdef __cplusplus
extern "C"
{
#endif

  /* Includes ------------------------------------------------------------------*/

  /* USER CODE BEGIN Includes */

  /* USER CODE END Includes */

  /* USER CODE BEGIN Private defines */

  /* USER CODE END Private defines */

  void IWDG_Init(void);

  void IWDG_reset(void);

  /**
   * @brief Possible STM32 system reset causes
   */
  typedef enum reset_cause_e
  {
    RESET_CAUSE_UNKNOWN = 0,
    RESET_CAUSE_LOW_POWER_RESET,
    RESET_CAUSE_WINDOW_WATCHDOG_RESET,
    RESET_CAUSE_INDEPENDENT_WATCHDOG_RESET,
    RESET_CAUSE_SOFTWARE_RESET,
    RESET_CAUSE_POWER_ON_POWER_DOWN_RESET,
    RESET_CAUSE_EXTERNAL_RESET_PIN_RESET,
    RESET_CAUSE_BROWNOUT_RESET,
    RESET_CAUSE_OPTIONS_BYTES_LOADING_RESET,
    RESET_CAUSE_FIREWALL_RESET
  } reset_cause_t;

  /* ==================================================================== */
  /* ======================= public functions =========================== */
  /* ==================================================================== */

  /* Function prototypes for public (external) functions go here */
  reset_cause_t reset_cause_get(void);
  const char *reset_cause_get_name(reset_cause_t reset_cause);

  /* USER CODE BEGIN Prototypes */

  /* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ iwdg_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
