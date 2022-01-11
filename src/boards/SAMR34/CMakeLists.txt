##
## \file  CMakeLists.txt
##
## \brief SAMR34 board CMakeLists.txt
##
## The Clear BSD License
## Copyright Semtech Corporation 2021. All rights reserved.
## Copyright Stackforce 2021. All rights reserved.
## Copyright TWTG on behalf of Microchip/Atmel (c)2021
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted (subject to the limitations in the disclaimer
## below) provided that the following conditions are met:
##     * Redistributions of source code must retain the above copyright
##       notice, this list of conditions and the following disclaimer.
##     * Redistributions in binary form must reproduce the above copyright
##       notice, this list of conditions and the following disclaimer in the
##       documentation and/or other materials provided with the distribution.
##     * Neither the name of the Semtech corporation nor the
##       names of its contributors may be used to endorse or promote products
##       derived from this software without specific prior written permission.
##
## NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
## THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
## CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
## NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
## PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##
project(SAMR34)
cmake_minimum_required(VERSION 3.6)
enable_language(ASM)

#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------

list(APPEND ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/delay-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/eeprom-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/gpio-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/rtc-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/spi-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/uart-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/i2c-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/utilities.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hw_timer.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/saml21b/gcc/gcc/startup_saml21.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/saml21b/gcc/system_saml21.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/src/hal_atomic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/src/hal_delay.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/src/hal_ext_irq.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/src/hal_gpio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/src/hal_init.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/src/hal_io.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/src/hal_sleep.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/src/hal_spi_m_sync.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/src/hal_usart_sync.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/src/hal_i2c_m_sync.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/utils/src/utils_assert.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/utils/src/utils_syscalls.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/utils/src/utils_event.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/utils/src/utils_list.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/core/hpl_core_m0plus_base.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/core/hpl_init.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/dmac/hpl_dmac.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/eic/hpl_eic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/gclk/hpl_gclk.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/mclk/hpl_mclk.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/osc32kctrl/hpl_osc32kctrl.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/oscctrl/hpl_oscctrl.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/pm/hpl_pm.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/systick/hpl_systick.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/sercom/hpl_sercom.c"
)

list(APPEND ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/sx127x_hal.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/ral_sx127x_bsp.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/radio_board.c"
)
set(RADIO_UPPERCASE "SX127X -DSX1276")

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

target_compile_definitions(${PROJECT_NAME} PUBLIC -D__SAML21J18B__ -D${RADIO_UPPERCASE})

# Add define if debbuger support is enabled
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<BOOL:${USE_DEBUGGER}>:USE_DEBUGGER>)

# Add define if radio debug pins support is enabled
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<BOOL:${USE_RADIO_DEBUG}>:USE_RADIO_DEBUG>)

# For debug builds set the symbol DEBUG
set(CMAKE_C_FLAGS_DEBUG -DDEBUG)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/saml21b/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/cmsis
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/config
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/Device_Startup
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hal/utils/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/dmac
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/eic
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/gclk
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/mclk
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/osc32kctrl
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/oscctrl
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/pm
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/port
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/rtc
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/sercom
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hpl/systick
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/saml21/hri
    $<TARGET_PROPERTY:board,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:system,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:radio,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:peripherals,INTERFACE_INCLUDE_DIRECTORIES>
)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)
