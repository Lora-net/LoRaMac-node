/*********************************************************************
*               SEGGER MICROCONTROLLER GmbH & Co. KG                 *
*       Solutions for real time microcontroller applications         *
**********************************************************************
*                                                                    *
*       (c) 2014 - 2015  SEGGER Microcontroller GmbH & Co. KG        *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : SEGGER_RTT_Conf.h
Purpose : Implementation of SEGGER real-time transfer (RTT) which
          allows real-time communication on targets which support
          debugger memory accesses while the CPU is running.
---------------------------END-OF-HEADER------------------------------
*/

#ifndef SEGGER_RTT_CONF_H
#define SEGGER_RTT_CONF_H
#include "sdk_config.h"
#include "compiler_abstraction.h"
#include "app_util_platform.h"
#ifdef __ICCARM__
  #include <intrinsics.h>
#endif

/*********************************************************************
*
*       Defines, configurable
*
**********************************************************************
*/
#define SEGGER_RTT_MAX_NUM_UP_BUFFERS             SEGGER_RTT_CONFIG_MAX_NUM_UP_BUFFERS     // Max. number of up-buffers (T->H) available on this target    (Default: 2)
#define SEGGER_RTT_MAX_NUM_DOWN_BUFFERS           SEGGER_RTT_CONFIG_MAX_NUM_DOWN_BUFFERS     // Max. number of down-buffers (H->T) available on this target  (Default: 2)

//Up Buffer size is set based on RTT configuration and nrf_log configuration. Larger buffer setting is used.
#if defined(NRF_LOG_BACKEND_RTT_OUTPUT_BUFFER_SIZE) && NRF_LOG_BACKEND_RTT_OUTPUT_BUFFER_SIZE > SEGGER_RTT_CONFIG_BUFFER_SIZE_UP
#define BUFFER_SIZE_UP                            NRF_LOG_BACKEND_RTT_OUTPUT_BUFFER_SIZE  // Size of the buffer for terminal output of target, up to host (Default: 1k)
#else
#define BUFFER_SIZE_UP                            SEGGER_RTT_CONFIG_BUFFER_SIZE_UP  // Size of the buffer for terminal output of target, up to host (Default: 1k)
#endif

#define BUFFER_SIZE_DOWN                          SEGGER_RTT_CONFIG_BUFFER_SIZE_DOWN    // Size of the buffer for terminal input to target from host (Usually keyboard input) (Default: 16)

#define SEGGER_RTT_PRINTF_BUFFER_SIZE             (64u)    // Size of buffer for RTT printf to bulk-send chars via RTT     (Default: 64)

#define SEGGER_RTT_MODE_DEFAULT                   SEGGER_RTT_CONFIG_DEFAULT_MODE // Mode for preinitialized terminal channel (buffer 0)

//
// Target is not allowed to perform other RTT operations while string still has not been stored completely.
// Otherwise we would probably end up with a mixed string in the buffer.
// If using  RTT from within interrupts, multiple tasks or multi processors, define the SEGGER_RTT_LOCK() and SEGGER_RTT_UNLOCK() function here.
//
/*********************************************************************
*
*       RTT lock configuration for SEGGER Embedded Studio,
*       Rowley CrossStudio and GCC
*/

#define SEGGER_RTT_LOCK(SavedState)       \
                   SavedState = 0;        \
                   CRITICAL_REGION_ENTER()

#define SEGGER_RTT_UNLOCK(SavedState)     \
                   (void)SavedState;      \
                   CRITICAL_REGION_EXIT()

/*************************** End of file ****************************/
#endif // SEGGER_RTT_CONF_H
