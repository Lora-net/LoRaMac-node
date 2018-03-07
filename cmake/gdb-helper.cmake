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
    if(BOARD STREQUAL LoRaMote OR  BOARD STREQUAL SensorNode OR BOARD STREQUAL SK-iM880A)
        set(OPENOCD_INTERFACE stlink-v2.cfg)
        set(OPENOCD_TARGET stm32l1.cfg)
    elseif(BOARD STREQUAL NAMote72 OR BOARD STREQUAL NucleoL152)
        set(OPENOCD_INTERFACE stlink-v2-1.cfg)
        set(OPENOCD_TARGET stm32l1.cfg)
    elseif(BOARD STREQUAL MoteII OR BOARD STREQUAL NucleoL073)
        set(OPENOCD_INTERFACE stlink-v2-1.cfg)
        set(OPENOCD_TARGET stm32l0.cfg)
    elseif(BOARD STREQUAL SAML21)
        set(OPENOCD_INTERFACE cmsis-dap.cfg)
        set(OPENOCD_TARGET at91samdXX.cfg)
    endif()


    configure_file(${CURRENT_MODULE_DIR}/launch.json.in ${CMAKE_SOURCE_DIR}/.vscode/launch.json @ONLY)
endfunction()
