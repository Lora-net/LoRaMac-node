/**
  ******************************************************************************
  * @file    dfu_mal.c
  * @author  MCD Application Team
  * @version V4.0.0
  * @date    21-January-2013
  * @brief   Generic media access Layer
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "usb-dfu-board.h"
#include "dfu_mal.h"
#include "usb_lib.h"
#include "usb_type.h"
#include "usb_desc.h"
#include "flash_if.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint16_t (*pMAL_Init) (void);
uint16_t (*pMAL_Erase) (uint32_t SectorAddress);
uint16_t (*pMAL_Write) (uint32_t SectorAddress, uint32_t DataLength);
uint8_t  *(*pMAL_Read)  (uint32_t SectorAddress, uint32_t DataLength);
uint8_t  MAL_Buffer[wTransferSize]; /* RAM Buffer for Downloaded Data */

extern ONE_DESCRIPTOR DFU_String_Descriptor[7];

/* This table holds the Typical Sector Erase and 1024 Bytes Write timings.
   These timings will be returned to the host when it checks the device
   status during a write or erase operation to know how much time the host
   should wait before issuing the next get status request. 
   These defines are set in usb_conf.h file.
   The values of this table should be extracted from relative memories 
   datasheet (Typical or Maximum timming value for Sector Erase and for 
   1024 bytes Write). All timings are expressed in millisecond unit (ms).
   Note that "Sector" refers here to the memory unit used for Erase/Write 
   operations. It could be a sector, a page, a block, a word ...
   If the erase operation is not supported, it is advised to set the erase
   timing to 1 (which means 1ms: one USB frame). */
static const uint16_t  TimingTable[5][2] =
  { /*       Sector Erase time,            Sector Program time*/    
    { SPI_FLASH_SECTOR_ERASE_TIME,    SPI_FLASH_SECTOR_WRITE_TIME },    /* SPI Flash */
    { M29W128F_SECTOR_ERASE_TIME,     M29W128F_SECTOR_WRITE_TIME },     /* NOR Flash M29W128F */
    { INTERN_FLASH_SECTOR_ERASE_TIME, INTERN_FLASH_SECTOR_WRITE_TIME }, /* Internal Flash */
    { M29W128G_SECTOR_ERASE_TIME,     M29W128G_SECTOR_WRITE_TIME },     /* NOR Flash M29W128G */
    { S29GL128_SECTOR_ERASE_TIME,     S29GL128_SECTOR_WRITE_TIME }      /* NOR Flash S29GL128 */
  };

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : MAL_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Init(void)
{

  FLASH_If_Init(); /* Internal Flash */

  return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_Erase
* Description    : Erase sector
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Erase(uint32_t SectorAddress)
{

  switch (SectorAddress & MAL_MASK)
  {
    case INTERNAL_FLASH_BASE:
      pMAL_Erase = FLASH_If_Erase;
      break;
      
    default:
      return MAL_FAIL;
  }
  return pMAL_Erase(SectorAddress);
}

/*******************************************************************************
* Function Name  : MAL_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Write (uint32_t SectorAddress, uint32_t DataLength)
{

  switch (SectorAddress & MAL_MASK)
  {
    case INTERNAL_FLASH_BASE:
      pMAL_Write = FLASH_If_Write;
      break;

    default:
      return MAL_FAIL;
  }
  return pMAL_Write(SectorAddress, DataLength);
}

/*******************************************************************************
* Function Name  : MAL_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
uint8_t *MAL_Read (uint32_t SectorAddress, uint32_t DataLength)
{

  switch (SectorAddress & MAL_MASK)
  {
    case INTERNAL_FLASH_BASE:
      pMAL_Read = FLASH_If_Read;
      break;
      
    default:
      return 0;
  }
  return pMAL_Read (SectorAddress, DataLength);
}

/*******************************************************************************
* Function Name  : MAL_GetStatus
* Description    : Get status
* Input          : None
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
uint16_t MAL_GetStatus(uint32_t SectorAddress , uint8_t Cmd, uint8_t *buffer)
{
  uint8_t x = (SectorAddress  >> 26) & 0x03 ; /* 0x000000000 --> 0 */
  /* 0x640000000 --> 1 */
  /* 0x080000000 --> 2 */

  uint8_t y = Cmd & 0x01;
  
  SET_POLLING_TIMING(TimingTable[x][y]);  /* x: Erase/Write Timing */
  /* y: Media              */
  return MAL_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
