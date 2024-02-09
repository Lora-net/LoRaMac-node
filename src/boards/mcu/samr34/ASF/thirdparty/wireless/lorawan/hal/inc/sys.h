/**
* \file  sys.h
*
* \brief Sytem management module include  file
*		
*
* Copyright (c) 2018 Microchip Technology Inc. and its subsidiaries. 
*
* \asf_license_start
*
* \page License
*
* Subject to your compliance with these terms, you may use Microchip
* software and any derivatives exclusively with Microchip products. 
* It is your responsibility to comply with third party license terms applicable 
* to your use of third party software (including open source software) that 
* may accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, 
* WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, 
* INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, 
* AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE 
* LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL 
* LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE 
* SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE 
* POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT 
* ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY 
* RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY, 
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*
* \asf_license_stop
*
*/
/*
* Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
*/
 
#ifndef _SYSTEM_H
#define	_SYSTEM_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "asf.h"

#ifdef UT
#define INTERRUPT_GlobalInterruptEnable()	
#define INTERRUPT_GlobalInterruptDisable()
#else
#define INTERRUPT_GlobalInterruptEnable  Enable_global_interrupt
#define INTERRUPT_GlobalInterruptDisable Disable_global_interrupt
#endif
    
#define MAX_EEPROM_PARAM_INDEX                      0x18AU 
#define USER_MEM_BASE       0x0300
#define USER_MEM_SIZE       0x0100
#define EEDATA_MAX_ADDR     0x4000
#define EXTERN_MAX_ADDR     0x8000
#define UNIQUE_ID_OFFSET    0x80F8
    
void System_GetExternalEui(uint8_t *id);
uint16_t System_GetAnalogReading(uint8_t channel);
void SystemBlockingWaitMs(uint32_t ms);

#ifdef	__cplusplus
}
#endif

#endif	/* _SYSTEM_H */

