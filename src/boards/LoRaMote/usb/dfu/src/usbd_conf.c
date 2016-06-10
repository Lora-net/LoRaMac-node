/**
  ******************************************************************************
  * @file    usbd_conf.c
  * @author  MCD Application Team
  * @version V1.5.0
  * @date    04-March-2016
  * @brief   This file implements the USB Device library callbacks and MSP
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright © 2016 STMicroelectronics International N.V. 
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

/* Includes ------------------------------------------------------------------*/
#include "stm32l1xx.h"
#include "stm32l1xx_hal.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_dfu.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
PCD_HandleTypeDef hpcd;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
                       PCD BSP Routines
*******************************************************************************/

/**
  * @brief  Initializes the PCD MSP.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd)
{
  GPIO_InitTypeDef  GPIO_InitStruct;
  
  /* Enable the GPIOA clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  /* Configure USB DM/DP pin. This is optional, and maintained only for user guidance.
     For the STM32L products there is no need to configure the PA12/PA11 pins couple 
     as Alternate Function */
  GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* Enable USB Clock */
  __HAL_RCC_USB_CLK_ENABLE();
  
  /* Enable SYSCFG Clock */
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  
  /* Set USB FS Interrupt priority */
  HAL_NVIC_SetPriority(USB_LP_IRQn, 5, 0);
  
  /* Enable USB Interrupt */
  HAL_NVIC_EnableIRQ(USB_LP_IRQn);
}

/**
  * @brief  De-Initializes the PCD MSP.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_MspDeInit(PCD_HandleTypeDef *hpcd)
{
  /* Disable USB FS Clock */
  __HAL_RCC_USB_CLK_DISABLE();
  
  /* Disable SYSCFG Clock */
  __HAL_RCC_SYSCFG_CLK_DISABLE();
}

/*******************************************************************************
                       LL Driver Callbacks (PCD -> USB Device Library)
*******************************************************************************/

/**
  * @brief  SetupStage callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_SetupStage(hpcd->pData, (uint8_t *)hpcd->Setup);
}

/**
  * @brief  DataOut Stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint Number
  * @retval None
  */
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_DataOutStage(hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

/**
  * @brief  DataIn Stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint Number
  * @retval None
  */
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_DataInStage(hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_SOF(hpcd->pData);
}

/**
  * @brief  Reset callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_SetSpeed(hpcd->pData, USBD_SPEED_FULL);
  /* Reset Device */
  USBD_LL_Reset(hpcd->pData);
}

/**
  * @brief  Suspend callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
}

/**
  * @brief  Resume callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
}

/**
  * @brief  ISOOUTIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint Number
  * @retval None
  */
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_IsoOUTIncomplete(hpcd->pData, epnum);
}

/**
  * @brief  ISOINIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint Number
  * @retval None
  */
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_IsoINIncomplete(hpcd->pData, epnum);
}

/**
  * @brief  ConnectCallback callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_DevConnected(hpcd->pData);
}

/**
  * @brief  Disconnect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_DevDisconnected(hpcd->pData);
}

/*******************************************************************************
                       LL Driver Interface (USB Device Library --> PCD)
*******************************************************************************/

/**
  * @brief  Initializes the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
  /* Set LL Driver parameters */
  hpcd.Instance = USB;
  hpcd.Init.dev_endpoints = 8;
  hpcd.Init.ep0_mps = PCD_EP0MPS_64;
  hpcd.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd.Init.speed = PCD_SPEED_FULL;
  hpcd.Init.low_power_enable = 0;
  /* Link The driver to the stack */
  hpcd.pData = pdev;
  pdev->pData = &hpcd;
  /* Initialize LL Driver */
  HAL_PCD_Init(pdev->pData);
  
  HAL_PCDEx_PMAConfig(pdev->pData , 0x00 , PCD_SNG_BUF, 0x18);
  HAL_PCDEx_PMAConfig(pdev->pData , 0x80 , PCD_SNG_BUF, 0x58); 
  
  return USBD_OK;
}

/**
  * @brief  De-Initializes the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
  HAL_PCD_DeInit(pdev->pData);
  return USBD_OK;
}

/**
  * @brief  Starts the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
  HAL_PCD_Start(pdev->pData);
  return USBD_OK;
}

/**
  * @brief  Stops the Low Level portion of the Device driver.
  * @param  pdev: Device handle
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
  HAL_PCD_Stop(pdev->pData);
  return USBD_OK;
}

/**
  * @brief  Opens an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  ep_type: Endpoint Type
  * @param  ep_mps: Endpoint Max Packet Size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev,
                                  uint8_t ep_addr,
                                  uint8_t ep_type,
                                  uint16_t ep_mps)
{
  HAL_PCD_EP_Open(pdev->pData,
                  ep_addr,
                  ep_mps,
                  ep_type);

  return USBD_OK;
}

/**
  * @brief  Closes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_Close(pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Flushes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_Flush(pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Sets a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_SetStall(pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Clears a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_PCD_EP_ClrStall(pdev->pData, ep_addr);
  return USBD_OK;
}

/**
  * @brief  Returns Stall condition.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval Stall (1: Yes, 0: No)
  */
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  PCD_HandleTypeDef *hpcd = pdev->pData;
  
  if ((ep_addr & 0x80) == 0x80)
  {
    return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  }
  else
  {
    return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
  }
}

/**
  * @brief  Assigns a USB address to the device.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
  HAL_PCD_SetAddress(pdev->pData, dev_addr);
  return USBD_OK;
}

/**
  * @brief  Transmits data over an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  pbuf: Pointer to data to be sent
  * @param  size: Data size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev,
                                    uint8_t ep_addr,
                                    uint8_t *pbuf,
                                    uint16_t size)
{
  HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size);
  return USBD_OK;
}

/**
  * @brief  Prepares an endpoint for reception.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @param  pbuf: Pointer to data to be received
  * @param  size: Data size
  * @retval USBD Status
  */
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev,
                                          uint8_t ep_addr,
                                          uint8_t *pbuf,
                                          uint16_t size)
{
  HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size);
  return USBD_OK;
}

/**
  * @brief  Returns the last transfered packet size.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint Number
  * @retval Recived Data Size
  */
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  return HAL_PCD_EP_GetRxCount(pdev->pData, ep_addr);
}

/**
  * @brief  Delays routine for the USB Device Library.
  * @param  Delay: Delay in ms
  * @retval None
  */
void USBD_LL_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

/**
  * @brief  static single allocation.
  * @param  size: size of allocated memory
  * @retval None
  */
void *USBD_static_malloc(uint32_t size)
{
  static uint32_t mem[MAX_STATIC_ALLOC_SIZE];
  return mem;
}

/**
  * @brief  Dummy memory free
  * @param  *p pointer to allocated  memory address
  * @retval None
  */
void USBD_static_free(void *p)
{
}

/**
  * @brief  Software Device Connection
  * @param  hpcd: PCD handle
  * @param  state: connection state (0 : disconnected / 1: connected)
  * @retval None
  */
void HAL_PCDEx_SetConnectionState(PCD_HandleTypeDef *hpcd, uint8_t state)
{
  if (state == 1)
  {
    /*  DP Pull-Down is Internal */
    __HAL_SYSCFG_USBPULLUP_ENABLE();
  }
  else
  {
    /*  DP Pull-Down is Internal */
    __HAL_SYSCFG_USBPULLUP_DISABLE();
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
