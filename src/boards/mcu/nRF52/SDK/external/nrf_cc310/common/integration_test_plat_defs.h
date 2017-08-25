// Copyright (c) 2016-2017, ARM Limited or its affiliates. All rights reserved 
// 
// This file and the related binary are licensed under the ARM Object Code and 
// Headers License; you may not use these files except in compliance with this 
// license. 
// 
// You may obtain a copy of the License at <.../external/nrf_cc310/license.txt> 
// 
// See the License for the specific language governing permissions and 
// limitations under the License.
#include <stdio.h>
#include <stdarg.h>

#include "SEGGER_RTT.h"

#if DX_LINUX_PLATFORM
extern uint32_t g_free_mem_addr;
extern uint32_t *g_test_stack_base_addr;
extern uint32_t g_rom_base_addr;
extern uint32_t g_env_rom_base_addr;
#endif


SaSiError_t   mapEnvMemory(void);
void unmapMemory(void);
SaSiError_t integration_tests_setup(void);
void integration_tests_clear(void);


#define REG_BASE_ADR			DX_BASE_CC
#define REG_AREA_LEN			0x20000
#define ENV_REG_BASE_ADR		DX_BASE_ENV_REGS
#define ENV_REG_AREA_LEN		0x20000
#define SRAM_BASE_ADR			0
#define TEST_DATA_START_ADDRESS         sizeof(CRYS_RND_Context_t)+sizeof(CRYS_RND_WorkBuff_t)

/* Zynq EVBs have 1GB and we reserve the memory at offset 768M */
#define FREE_MEM_BASE_ADR             	0x20020000
#define CONTIG_FREE_MEM	                0x8000
/* Free memory is dedicated for SW image storage */
#define BOOT_FREE_MEM_BASE_ADR          FREE_MEM_BASE_ADR
#define BOOT_FREE_MEM_LEN		0x500000

/* user space follows the free memory, and uses for stack and workspace memory */
#define PTHREAD_STACK_BASE_ADR		(FREE_MEM_BASE_ADR + CONTIG_FREE_MEM)
#define PTHREAD_STACK_SIZE		16*1024

#define WORKSPACE_BASE_ADDR         (PTHREAD_STACK_BASE_ADR + PTHREAD_STACK_SIZE)

#define INTEG_TEST_PRINT(...)           SEGGER_RTT_printf(0, __VA_ARGS__)
