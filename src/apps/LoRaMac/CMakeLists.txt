##
## \file  CMakeLists.txt
##
## \brief Examples CMakeLists.txt
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
project(LoRaMac)
cmake_minimum_required(VERSION 3.6)

#---------------------------------------------------------------------------------------
# Options
#---------------------------------------------------------------------------------------

# Allow switching of sub projects
set(SUB_PROJECT_LIST periodic-uplink-lpp fuota-test-01)
set(SUB_PROJECT periodic-uplink-lpp CACHE STRING "Default sub project is periodic-uplink-lpp")
set_property(CACHE SUB_PROJECT PROPERTY STRINGS ${SUB_PROJECT_LIST})

# Allow LORAWAN_DEFAULT_CLASS overriding for periodic-uplink-lpp project
if(SUB_PROJECT STREQUAL periodic-uplink-lpp)
    set(LORAWAN_DEFAULT_CLASS_LIST CLASS_A CLASS_B CLASS_C)
    set(LORAWAN_DEFAULT_CLASS CLASS_A CACHE STRING "Default LoRaWAN class is ClassA")
    set_property(CACHE LORAWAN_DEFAULT_CLASS PROPERTY STRINGS ${LORAWAN_DEFAULT_CLASS_LIST})
endif()

# Allow switching of active region
set(ACTIVE_REGION_LIST   LORAMAC_REGION_EU868 LORAMAC_REGION_US915 LORAMAC_REGION_CN779
    LORAMAC_REGION_EU433 LORAMAC_REGION_AU915 LORAMAC_REGION_AS923 LORAMAC_REGION_CN470
    LORAMAC_REGION_KR920 LORAMAC_REGION_IN865 LORAMAC_REGION_RU864
)
set(ACTIVE_REGION LORAMAC_REGION_EU868 CACHE STRING "Default active region is EU868")
set_property(CACHE ACTIVE_REGION PROPERTY STRINGS ${ACTIVE_REGION_LIST})

if((SUB_PROJECT STREQUAL periodic-uplink-lpp OR SUB_PROJECT STREQUAL fuota-test-01) AND NOT CLASSB_ENABLED )
    message(FATAL_ERROR "Please turn on Class B support of LoRaMac ( CLASSB_ENABLED=ON ) to use periodic-uplink-lpp, fuota-test-01 sub projects")
endif()

# Allow selection of secure-element provisioning method
option(SECURE_ELEMENT_PRE_PROVISIONED "Secure-element pre-provisioning" ON)

if(SUB_PROJECT STREQUAL periodic-uplink-lpp)

    #---------------------------------------------------------------------------------------
    # Application common features handling
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_COMMON
        "${CMAKE_CURRENT_LIST_DIR}/common/CayenneLpp.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/cli.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandlerMsgDisplay.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/NvmDataMgmt.c"
    )

    #---------------------------------------------------------------------------------------
    # Application LoRaMac handler
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_LMH
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/LmHandler.c"
    )

    #---------------------------------------------------------------------------------------
    # LoRaMac handler applicative packages
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_LMHP
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/FragDecoder.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpClockSync.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpCompliance.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpFragmentation.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpRemoteMcastSetup.c"
    )

elseif(SUB_PROJECT STREQUAL fuota-test-01)

    #---------------------------------------------------------------------------------------
    # Application common features handling
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_COMMON
        "${CMAKE_CURRENT_LIST_DIR}/common/cli.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandlerMsgDisplay.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/NvmDataMgmt.c"
    )

    #---------------------------------------------------------------------------------------
    # Application LoRaMac handler
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_LMH
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/LmHandler.c"
    )

    #---------------------------------------------------------------------------------------
    # LoRaMac handler applicative packages
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_LMHP
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/FragDecoder.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpClockSync.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpCompliance.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpFragmentation.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpRemoteMcastSetup.c"
    )

else()
    message(FATAL_ERROR "Unknown SUB_PROJECT")
endif()

#---------------------------------------------------------------------------------------
# Application
#---------------------------------------------------------------------------------------
file(GLOB ${PROJECT_NAME}_SOURCES "${CMAKE_CURRENT_LIST_DIR}/${SUB_PROJECT}/${BOARD}/*.c")

add_executable(${PROJECT_NAME}-${SUB_PROJECT}
                            ${${PROJECT_NAME}_COMMON}
                            ${${PROJECT_NAME}_LMH}
                            ${${PROJECT_NAME}_LMHP}
                            ${${PROJECT_NAME}_SOURCES}
                            $<TARGET_OBJECTS:mac>
                            $<TARGET_OBJECTS:system>
                            $<TARGET_OBJECTS:radio>
                            $<TARGET_OBJECTS:peripherals>
                            $<TARGET_OBJECTS:${BOARD}>
)

target_compile_definitions(${PROJECT_NAME}-${SUB_PROJECT} PRIVATE $<$<BOOL:${CLASSB_ENABLED}>:LORAMAC_CLASSB_ENABLED>)
target_compile_definitions(${PROJECT_NAME}-${SUB_PROJECT} PRIVATE ACTIVE_REGION=${ACTIVE_REGION})
if(SUB_PROJECT STREQUAL periodic-uplink-lpp)
    target_compile_definitions(${PROJECT_NAME}-${SUB_PROJECT} PRIVATE LORAWAN_DEFAULT_CLASS=${LORAWAN_DEFAULT_CLASS})
endif()
if(${SECURE_ELEMENT_PRE_PROVISIONED} MATCHES ON)
    target_compile_definitions(${PROJECT_NAME}-${SUB_PROJECT} PRIVATE -DSECURE_ELEMENT_PRE_PROVISIONED)
endif()
if(${SECURE_ELEMENT} MATCHES SOFT_SE)
    target_compile_definitions(${PROJECT_NAME}-${SUB_PROJECT} PRIVATE -DSOFT_SE)
endif()
target_compile_definitions(${PROJECT_NAME}-${SUB_PROJECT}  PUBLIC
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:mac,INTERFACE_COMPILE_DEFINITIONS>>
)

target_include_directories(${PROJECT_NAME}-${SUB_PROJECT} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/common/LmHandler
    ${CMAKE_CURRENT_SOURCE_DIR}/common/LmHandler/packages
    ${CMAKE_CURRENT_SOURCE_DIR}/${SUB_PROJECT}/${BOARD}
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:mac,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:system,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:radio,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:peripherals,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:${BOARD},INTERFACE_INCLUDE_DIRECTORIES>>
)

set_property(TARGET ${PROJECT_NAME}-${SUB_PROJECT} PROPERTY C_STANDARD 11)

target_link_libraries(${PROJECT_NAME}-${SUB_PROJECT} m)

#---------------------------------------------------------------------------------------
# Debugging and Binutils
#---------------------------------------------------------------------------------------

include(gdb-helper)
include(binutils-arm-none-eabi)

# Generate debugger configurations
generate_run_gdb_stlink(${PROJECT_NAME}-${SUB_PROJECT})
generate_run_gdb_openocd(${PROJECT_NAME}-${SUB_PROJECT})
generate_vscode_launch_openocd(${PROJECT_NAME}-${SUB_PROJECT})

# Print section sizes of target
print_section_sizes(${PROJECT_NAME}-${SUB_PROJECT})

# Create output in hex and binary format
create_bin_output(${PROJECT_NAME}-${SUB_PROJECT})
create_hex_output(${PROJECT_NAME}-${SUB_PROJECT})
