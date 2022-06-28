##
## \file  CMakeLists.txt
##
## \brief radio CMakeLists.txt
##
## The Clear BSD License
## Copyright Semtech Corporation 2021. All rights reserved.
## Copyright Stackforce 2021. All rights reserved.
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
project(radio)
cmake_minimum_required(VERSION 3.6)

#---------------------------------------------------------------------------------------
# Options
#---------------------------------------------------------------------------------------

# Allow switching of radios
set(RADIO_LIST sx1272 sx1276 sx126x lr11xx)
set(RADIO sx1272 CACHE STRING "Default radio is sx1272")
set_property(CACHE RADIO PROPERTY STRINGS ${RADIO_LIST})
set_property(CACHE RADIO PROPERTY ADVANCED)

#---------------------------------------------------------------------------------------
# Radio drivers
#---------------------------------------------------------------------------------------

# Check if selected radio supports LR-FHSS modulation
if((${LORAMAC_LR_FHSS_IS_ON} STREQUAL ON) AND (${RADIO} STREQUAL sx1272 OR ${RADIO} STREQUAL sx1276))
    message(FATAL_ERROR "\nSX127x radios do not suppport LR-FHSS modulation.\nPlease set LORAMAC_LR_FHSS_IS_ON option to \"OFF\" state.")
endif()

list(APPEND ${PROJECT_NAME}_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/loramac_radio.c
)

if(${RADIO} STREQUAL lr11xx)
    list(APPEND ${PROJECT_NAME}_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/smtc_ral/src/ral_lr11xx.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lr11xx_driver/src/lr11xx_bootloader.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lr11xx_driver/src/lr11xx_crypto_engine.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lr11xx_driver/src/lr11xx_driver_version.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lr11xx_driver/src/lr11xx_gnss.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lr11xx_driver/src/lr11xx_lr_fhss.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lr11xx_driver/src/lr11xx_radio_timings.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lr11xx_driver/src/lr11xx_radio.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lr11xx_driver/src/lr11xx_regmem.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lr11xx_driver/src/lr11xx_system.c
        ${CMAKE_CURRENT_SOURCE_DIR}/lr11xx_driver/src/lr11xx_wifi.c
    )
elseif(${RADIO} STREQUAL sx126x)
    list(APPEND ${PROJECT_NAME}_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/smtc_ral/src/ral_sx126x.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sx126x_driver/src/lr_fhss_mac.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sx126x_driver/src/sx126x_lr_fhss.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sx126x_driver/src/sx126x.c
    )
elseif(${RADIO} STREQUAL sx1272 OR ${RADIO} STREQUAL sx1276)
    list(APPEND ${PROJECT_NAME}_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/smtc_ral/src/ral_sx127x.c
        ${CMAKE_CURRENT_SOURCE_DIR}/sx127x_driver/src/sx127x.c
    )
else()
    message(FATAL_ERROR "Unsupported radio driver selected...")
endif()

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

add_dependencies(${PROJECT_NAME} board)

list(APPEND ${PROJECT_NAME}_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/smtc_ral/src
)

if(${RADIO} STREQUAL lr11xx )
    list(APPEND ${PROJECT_NAME}_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/lr11xx_driver/src
    )
elseif(${RADIO} STREQUAL sx126x)
    list(APPEND ${PROJECT_NAME}_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/sx126x_driver/src
    )
elseif(${RADIO} STREQUAL sx1272 OR ${RADIO} STREQUAL sx1276)
    list(APPEND ${PROJECT_NAME}_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/sx127x_driver/src
    )
else()
    message(FATAL_ERROR "Unsupported radio driver selected...")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${${PROJECT_NAME}_INCLUDES}
    $<TARGET_PROPERTY:board,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:system,INTERFACE_INCLUDE_DIRECTORIES>
)

if(${RADIO} STREQUAL lr11xx)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DLR11XX -DLR11XX_DISABLE_WARNINGS)
endif()

if(${RADIO} STREQUAL sx126x)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DSX126X)
endif()

if(${RADIO} STREQUAL sx1272)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DSX127X -DSX1272)
endif()

if(${RADIO} STREQUAL sx1276)
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DSX127X -DSX1276)
endif()

# Add define if LR-FHSS modulation is supported
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<BOOL:${LORAMAC_LR_FHSS_IS_ON}>:LORAMAC_LR_FHSS_IS_ON>)

option(USE_RADIO_DEBUG "Enable Radio Debug GPIO's" OFF)
target_compile_definitions(${PROJECT_NAME} PUBLIC  $<$<BOOL:${USE_RADIO_DEBUG}>:USE_RADIO_DEBUG>)
target_include_directories(${PROJECT_NAME} PUBLIC $<TARGET_PROPERTY:${BOARD},INTERFACE_INCLUDE_DIRECTORIES>)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)
