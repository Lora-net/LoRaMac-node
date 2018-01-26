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
## Authors:  Johannes Bruder (STACKFORCE), Miguel Luis (Semtech)
##
project(lora-mac)
cmake_minimum_required(VERSION 3.6)

#---------------------------------------------------------------------------------------
# Options
#---------------------------------------------------------------------------------------

# Allow switching of target platform
set(BOARD_LIST LoRaMote MoteII NAMote72 SensorNode SK-iM880A SAML21)
set(BOARD LoRaMote CACHE STRING "Default target platform is LoRaMote")
set_property(CACHE BOARD PROPERTY STRINGS ${BOARD_LIST})

# Allow switching of Applications
set(APPLICATION_LIST LoRaMac ping-pong rx-sensi tx-cw BootLoader)
set(APPLICATION LoRaMac CACHE STRING "Default Application is LoRaMac")
set_property(CACHE APPLICATION PROPERTY STRINGS ${APPLICATION_LIST})

# Switch for USB-Uart support, enable it for some Applications who needs it.
option(USE_USB_CDC "Use USB-Uart" OFF)

# Switch for bootloader support.
option(USE_BOOTLOADER "Use Bootloader" OFF)

# Switch for debugger support.
option(USE_DEBUGGER "Use Debugger" ON)

#---------------------------------------------------------------------------------------
# Target Boards
#---------------------------------------------------------------------------------------

if(BOARD STREQUAL LoRaMote)
    # Configure toolchain for LoRaMote
    if(APPLICATION STREQUAL BootLoader)
        set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/LoRaMote/cmsis/arm-gcc/stm32l151xb_flash_bootloader.ld)
    elseif(USE_BOOTLOADER)
        set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/SensorNode/cmsis/arm-gcc/stm32l151xb_flash_offset.ld)
    else()
        set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/LoRaMote/cmsis/arm-gcc/stm32l151xb_flash.ld)
    endif()

    include(stm32l1)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/LoRaMote)

    # Configure radio
    set(RADIO sx1272 CACHE INTERNAL "Radio sx1272 selected")

elseif(BOARD STREQUAL MoteII)
    # Configure toolchain for MoteII
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/MoteII/cmsis/arm-gcc/stm32l051xx_flash.ld)
    include(stm32l0)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/MoteII)

    # Configure radio
    set(RADIO sx1272 CACHE INTERNAL "Radio sx1272 selected")

elseif(BOARD STREQUAL SensorNode)
    # Configure toolchain for SensorNode
    if(APPLICATION STREQUAL BootLoader)
        set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/SensorNode/cmsis/arm-gcc/stm32l151xb_flash_bootloader.ld)
    elseif(USE_BOOTLOADER)
        set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/SensorNode/cmsis/arm-gcc/stm32l151xb_flash_offset.ld)
    else()
        set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/SensorNode/cmsis/arm-gcc/stm32l151xb_flash.ld)
    endif()
    include(stm32l1)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/SensorNode)

    # Configure radio
    set(RADIO sx1276 CACHE INTERNAL "Radio sx1276 selected")

elseif(BOARD STREQUAL SK-iM880A)
    # Configure toolchain for LoRaMote
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/SK-iM880A/cmsis/arm-gcc/stm32l151xb_flash.ld)
    include(stm32l1)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/SK-iM880A)

    # Configure radio
    set(RADIO sx1272 CACHE INTERNAL "Radio sx1272 selected")

elseif(BOARD STREQUAL NAMote72)
    # Configure toolchain for LoRaMote
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/NAMote72/cmsis/arm-gcc/stm32l152xc_flash.ld)
    include(stm32l1)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/NAMote72)

    # Configure radio
    set(RADIO sx1272 CACHE INTERNAL "Radio sx1272 selected")

elseif(BOARD STREQUAL SAML21)
    # Configure toolchain for SAML21
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/mcu/saml21/saml21b/gcc/gcc/saml21j18b_flash.ld)
    include(saml21)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/SAML21)

    # Configure radio
    set(RADIO sx1276 CACHE INTERNAL "Radio sx1276 selected")

endif()

#---------------------------------------------------------------------------------------
# General Components
#---------------------------------------------------------------------------------------

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/radio)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/system)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/mac)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/peripherals)

#---------------------------------------------------------------------------------------
# Applications
#---------------------------------------------------------------------------------------

if(APPLICATION STREQUAL LoRaMac)

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apps/LoRaMac)

elseif(APPLICATION STREQUAL ping-pong)

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apps/ping-pong)

elseif(APPLICATION STREQUAL rx-sensi)

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apps/rx-sensi)

elseif(APPLICATION STREQUAL tx-cw)

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apps/tx-cw)

elseif(APPLICATION STREQUAL BootLoader)

    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/apps/BootLoader)

endif()
