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
#ifdef DX_LINUX_PLATFORM
#include <sys/mman.h>//for mmap
#include <unistd.h>//for close()
#include <sys/types.h>//open()
#include <sys/stat.h>//open()
#include <fcntl.h>//open()
#include <stdio.h>//for printf
#include <errno.h>//for errno
#include <pthread.h>
#endif
#include <stdint.h>
#include "ssi_pal_types.h"
#include "ssi_regs.h"
//#include "integration_test_ssi_defs.h"
#include "integration_test_plat_defs.h"
#include "crys_rnd.h"

#include "nrf52840.h"


void CRYPTOCELL_IRQHandler(void)
{
    INTEG_TEST_PRINT("Got Cryptocell interrupt\r\n");
}

/*Globals*/
uint32_t g_free_mem_addr;
uint32_t *g_test_stack_base_addr;
uint32_t g_rom_base_addr;
uint32_t g_env_rom_base_addr;


CRYS_RND_Context_t*   rndContext_ptr;
CRYS_RND_WorkBuff_t*  rndWorkBuff_ptr;

#if defined(__CC_ARM)
CRYS_RND_Context_t   rndContext = {0};
CRYS_RND_WorkBuff_t  rndWorkBuff = {0};
#else
CRYS_RND_Context_t   rndContext;
CRYS_RND_WorkBuff_t  rndWorkBuff;
#endif

uint32_t* UserSpace = 0;
int fd_mem;

//initializatoins that need to be done prior to running the integration tests.
SaSiError_t integration_tests_setup(void)
{
    SaSiError_t ret = 0;
    rndContext_ptr = &rndContext;
    rndWorkBuff_ptr = &rndWorkBuff;

    // Initialize Segger RTT logging

    (void)SEGGER_RTT_Init();

    INTEG_TEST_PRINT("==================== TEST START ====================\r\n");

    NRF_CRYPTOCELL->ENABLE = 1;

    return ret;
}

void integration_tests_clear(void)
{
    INTEG_TEST_PRINT("==================== TEST END ====================\r\n");
    while(1);
}

void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    (void)id;
    (void)pc;
    (void)info;
    while(1);
}
