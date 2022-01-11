##
## \file  gdb-helper.cmake
##
## \brief Collection of functions to generate different GDB debugging configurations
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

##
##   ______                              _
##  / _____)             _              | |
## ( (____  _____ ____ _| |_ _____  ____| |__
##  \____ \| ___ |    (_   _) ___ |/ ___)  _ \
##  _____) ) ____| | | || |_| ____( (___| | | |
## (______/|_____)_|_|_| \__)_____)\____)_| |_|
## (C)2013-2017 Semtech
##  ___ _____ _   ___ _  _____ ___  ___  ___ ___
## / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
## \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
## |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
## embedded.connectivity.solutions.==============
##
## License:  Revised BSD License, see LICENSE.TXT file included in the project
## Authors:  Johannes Bruder ( STACKFORCE ), Miguel Luis ( Semtech )
##
## Collection of functions to generate different GDB debugging configurations
##

# Get the path of this module
set(CURRENT_MODULE_DIR ${CMAKE_CURRENT_LIST_DIR})


#---------------------------------------------------------------------------------------
# Set tools
#---------------------------------------------------------------------------------------
set(GDB_BIN ${TOOLCHAIN_BIN_DIR}/${TOOLCHAIN}-gdb${TOOLCHAIN_EXT})
if(NOT OPENOCD_BIN)
        if(CMAKE_HOST_SYSTEM_NAME STREQUAL Linux)
            set(OPENOCD_BIN "/usr/bin/openocd" CACHE STRING "OpenOCD executable")
        elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Darwin)
            set(OPENOCD_BIN "/usr/local/bin/openocd" CACHE STRING "OpenOCD executable")
        elseif(CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
            set(OPENOCD_BIN "C:/openocd/bin/openocd.exe" CACHE STRING "OpenOCD executable")
        endif()
endif()

#---------------------------------------------------------------------------------------
# Generates a GDB run script for debugging with STLINKv1/v2/v2-1 programmer and texane stlink tool.
# More infos check: https://github.com/texane/stlink
#---------------------------------------------------------------------------------------
function(generate_run_gdb_stlink TARGET)
    get_target_property( TARGET_NAME ${TARGET} NAME )
    configure_file(${CURRENT_MODULE_DIR}/stlink-run.gdb.in ${PROJECT_BINARY_DIR}/stlink-run.gdb @ONLY)
endfunction()

#---------------------------------------------------------------------------------------
# Generates a GDB run script for debugging with any supported programmer and openOCD.
#---------------------------------------------------------------------------------------
function(generate_run_gdb_openocd TARGET)
    get_target_property( TARGET_NAME ${TARGET} NAME )
    configure_file(${CURRENT_MODULE_DIR}/openocd-run.gdb.in ${PROJECT_BINARY_DIR}/openocd-run.gdb @ONLY)
endfunction()

#---------------------------------------------------------------------------------------
# Generates a Visual Studio Code launch configuration for debugging with openOCD.
#---------------------------------------------------------------------------------------
function(generate_vscode_launch_openocd TARGET)
    get_target_property( TARGET_NAME ${TARGET} NAME )

    # Available OpenOCD interfaces
    # Use stlink-v2-1.cfg when stlink interface is built in. Otherwise use stlink-v2.cfg
    set(OPENOCD_INTERFACE_LIST stlink-v2-1.cfg stlink-v2.cfg)
    set(OPENOCD_INTERFACE stlink-v2.cfg CACHE STRING "Default OPENOCD Interface is stlink-v2.cfg")
    set_property(CACHE OPENOCD_INTERFACE PROPERTY STRINGS ${OPENOCD_INTERFACE_LIST})

    # Available OpenOCD targets
    set(OPENOCD_TARGET_LIST stm32l0.cfg stm32l1.cfg)
    set(OPENOCD_TARGET stm32l1.cfg CACHE STRING "Default OPENOCD Target is stm32l1.cfg")
    set_property(CACHE OPENOCD_TARGET PROPERTY STRINGS ${OPENOCD_TARGET_LIST})

    # Available OpenOCD targets
    set(OPENOCD_BOARD_LIST atmel_saml21_xplained_pro.cfg )
    set(OPENOCD_BOARD atmel_saml21_xplained_pro.cfg CACHE STRING "Default OPENOCD board is atmel_saml21_xplained_pro.cfg")
    set_property(CACHE OPENOCD_BOARD PROPERTY STRINGS ${OPENOCD_BOARD_LIST})

    # Set the OPENOCD_TARGET and OPENOCD_INTERFACE variables according to BOARD
    if(BOARD STREQUAL NAMote72 OR BOARD STREQUAL NucleoL152)
        set(OPENOCD_INTERFACE stlink-v2-1.cfg)
        set(OPENOCD_TARGET stm32l1.cfg)
    elseif(BOARD STREQUAL NucleoL073 OR BOARD STREQUAL B-L072Z-LRWAN1)
        set(OPENOCD_INTERFACE stlink-v2-1.cfg)
        set(OPENOCD_TARGET stm32l0.cfg)
    elseif(BOARD STREQUAL NucleoL476)
        set(OPENOCD_INTERFACE stlink-v2-1.cfg)
        set(OPENOCD_TARGET stm32l4x.cfg)
    elseif(BOARD STREQUAL SKiM880B OR BOARD STREQUAL SKiM980A)
        set(OPENOCD_INTERFACE stlink-v2.cfg)
        set(OPENOCD_TARGET stm32l1.cfg)
    elseif(BOARD STREQUAL SKiM881AXL)
        set(OPENOCD_INTERFACE stlink-v2.cfg)
        set(OPENOCD_TARGET stm32l0.cfg)
    elseif(BOARD STREQUAL SAMR34)
        set(OPENOCD_INTERFACE cmsis-dap.cfg)
        set(OPENOCD_TARGET at91samdXX.cfg)
    endif()


    configure_file(${CURRENT_MODULE_DIR}/launch.json.in ${CMAKE_SOURCE_DIR}/.vscode/launch.json @ONLY)
endfunction()
