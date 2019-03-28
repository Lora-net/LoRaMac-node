/**
  ******************************************************************************
  * @file    EEPROM_Emul/Core/eeprom_emul.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the EEPROM
  *          emulation firmware library.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EEPROM_EMUL_H
#define __EEPROM_EMUL_H

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "eeprom_emul_conf.h"
#include "eeprom_emul_types.h"
#include "flash_interface.h"
#include "stm32l4xx_ll_crc.h"
#include "stm32l4xx_ll_bus.h"
#if defined(RECOVERY_TEST)
#include "stm32l4xx_ll_rtc.h"
#endif

/** @addtogroup EEPROM_Emulation
  * @{
  */

/* Private constants ---------------------------------------------------------*/
/** @defgroup EEPROM_Private_Constants EEPROM Private Constants
  * @{
  */

/** @defgroup Private_Other_Constants Private Other Constants
  * @{
  */

/* Page definitions */
#define PAGE_SIZE               FLASH_PAGE_SIZE                                  /*!< Page size */
#define PAGE_HEADER_SIZE        EE_ELEMENT_SIZE * 4U                             /*!< Page Header is 4 elements to save page state */
#define NB_MAX_ELEMENTS_BY_PAGE ((PAGE_SIZE - PAGE_HEADER_SIZE) / EE_ELEMENT_SIZE) /*!< Max number of elements by page */
#define PAGES_NUMBER            (((((NB_OF_VARIABLES + NB_MAX_ELEMENTS_BY_PAGE) / NB_MAX_ELEMENTS_BY_PAGE) * 2U) * CYCLES_NUMBER) + GUARD_PAGES_NUMBER)
                                                                                 /*!< Number of consecutives pages used by the application */
#define NB_MAX_WRITTEN_ELEMENTS ((NB_MAX_ELEMENTS_BY_PAGE * PAGES_NUMBER) / 2U)  /*!< Max number of elements written before triggering pages transfer */
#define START_PAGE              PAGE(START_PAGE_ADDRESS)                         /*!< Page index of the 1st page used for EEPROM emul, in the bank */
#define END_EEPROM_ADDRESS      (START_PAGE_ADDRESS + (PAGES_NUMBER * FLASH_PAGE_SIZE) - 1) /*!< Last address of EEPROM emulation flash pages */

/* No page define */
#define EE_NO_PAGE_FOUND        ((uint32_t)0xFFFFFFFFU)

/**
  * @}
  */

/**
  * @}
  */

/* Private macro -------------------------------------------------------------*/
/** @defgroup EEPROM_Private_Macros EEPROM Private Macros
  * @{
  */

/** @defgroup Macros_Pages Macros to manipulate pages
  * @{
  */

/* Macros to manipulate pages */
#define PAGE_ADDRESS(__PAGE__)   (uint32_t)(FLASH_BASE + (__PAGE__) * PAGE_SIZE + ((START_PAGE_ADDRESS - FLASH_BASE) / BANK_SIZE) * BANK_SIZE) /*!< Get page address from page index */
#define PAGE(__ADDRESS__)        (uint32_t)((((__ADDRESS__) - FLASH_BASE) % BANK_SIZE) / FLASH_PAGE_SIZE) /*!< Get page index from page address */
#define PREVIOUS_PAGE(__PAGE__)  (uint32_t)((((__PAGE__) - START_PAGE - 1U + PAGES_NUMBER) % PAGES_NUMBER) + START_PAGE) /*!< Get page index of previous page, among circular page list */
#define FOLLOWING_PAGE(__PAGE__) (uint32_t)((((__PAGE__) - START_PAGE + 1U) % PAGES_NUMBER) + START_PAGE) /*!< Get page index of following page, among circular page list */

/**
  * @}
  */

/** @defgroup Macros_Elements Macros to manipulate elements
  * @{
  */

/* Macros to manipulate elements */
#define EE_VIRTUALADDRESS_VALUE(__ELEMENT__)            (EE_VIRTUALADDRESS_TYPE)((__ELEMENT__) & EE_MASK_VIRTUALADDRESS) /*!< Get virtual address value from element value */
#define EE_DATA_VALUE(__ELEMENT__)                      (EE_DATA_TYPE)(((__ELEMENT__) & EE_MASK_DATA) >> EE_DATA_SHIFT) /*!< Get Data value from element value */
#define EE_CRC_VALUE(__ELEMENT__)                       (EE_CRC_TYPE)(((__ELEMENT__) & EE_MASK_CRC) >> EE_CRC_SHIFT) /*!< Get Crc value from element value */
#define EE_ELEMENT_VALUE(__VIRTADDR__,__DATA__,__CRC__) (((EE_ELEMENT_TYPE)(__DATA__) << EE_DATA_SHIFT) | (__CRC__) << EE_CRC_SHIFT | (__VIRTADDR__)) /*!< Get element value from virtual addr, data and crc values */

/**
  * @}
  */

/**
  * @}
  */

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/** @defgroup EEPROM_Exported_Functions EEPROM Exported Functions
  * @{
  */
EE_Status EE_Format(EE_Erase_type EraseType);
EE_Status EE_Init(uint16_t* VirtAddTab, EE_Erase_type EraseType);
#if defined(EE_ACCESS_32BITS)
EE_Status EE_ReadVariable32bits(uint16_t VirtAddress, uint32_t* pData);
EE_Status EE_WriteVariable32bits(uint16_t VirtAddress, uint32_t Data);
#endif
EE_Status EE_ReadVariable16bits(uint16_t VirtAddress, uint16_t* pData);
EE_Status EE_WriteVariable16bits(uint16_t VirtAddress, uint16_t Data);
EE_Status EE_ReadVariable8bits(uint16_t VirtAddress, uint8_t* pData);
EE_Status EE_WriteVariable8bits(uint16_t VirtAddress, uint8_t Data);
EE_Status EE_CleanUp(void);
EE_Status EE_CleanUp_IT(void);
EE_Status EE_DeleteCorruptedFlashAddress(uint32_t Address);
void EE_EndOfCleanup_UserCallback(void);

/**
  * @}
  */

/**
  * @}
  */

#endif /* __EEPROM_EMUL_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
