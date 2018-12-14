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
set(BOARD_LIST NAMote72 NucleoL073 NucleoL152 NucleoL476 SAML21 SKiM880B SKiM980A SKiM881AXL B-L072Z-LRWAN1)
set(BOARD NucleoL073 CACHE STRING "Default target platform is NucleoL073")
set_property(CACHE BOARD PROPERTY STRINGS ${BOARD_LIST})

# Allow switching of MBED shields
set(MBED_RADIO_SHIELD_LIST SX1272MB2DAS SX1276MB1LAS SX1276MB1MAS SX1261MBXBAS SX1262MBXCAS SX1262MBXDAS )
set(MBED_RADIO_SHIELD SX1261MBXBAS CACHE STRING "Default MBED radio shield is SX1261MBXBAS")
set_property(CACHE MBED_RADIO_SHIELD PROPERTY STRINGS ${MBED_RADIO_SHIELD_LIST})

# Allow switching of Applications
set(APPLICATION_LIST LoRaMac ping-pong rx-sensi tx-cw )
set(APPLICATION LoRaMac CACHE STRING "Default Application is LoRaMac")
set_property(CACHE APPLICATION PROPERTY STRINGS ${APPLICATION_LIST})

# Switch for USB-Uart support, enable it for some Applications who needs it.
option(USE_USB_CDC "Use USB-Uart" OFF)

# Switch for debugger support.
option(USE_DEBUGGER "Use Debugger" ON)

# Switch for Class B support of LoRaMac.
option(CLASSB_ENABLED "Class B support of LoRaMac" OFF)

#---------------------------------------------------------------------------------------
# Target Boards
#---------------------------------------------------------------------------------------

if(BOARD STREQUAL NAMote72)
    # Configure toolchain for NAMote72
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/NAMote72/cmsis/arm-gcc/stm32l152xc_flash.ld)
    include(stm32l1)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/NAMote72)

    # Configure radio
    set(RADIO sx1272 CACHE INTERNAL "Radio sx1272 selected")

elseif(BOARD STREQUAL NucleoL073)
    # Configure toolchain for NucleoL073
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/NucleoL073/cmsis/arm-gcc/stm32l073xx_flash.ld)
    include(stm32l0)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/NucleoL073)

    # Configure radio
    if(MBED_RADIO_SHIELD STREQUAL SX1272MB2DAS)
        set(RADIO sx1272 CACHE INTERNAL "Radio sx1272 selected")
    elseif(MBED_RADIO_SHIELD STREQUAL SX1276MB1LAS OR MBED_RADIO_SHIELD STREQUAL SX1276MB1MAS)
        set(RADIO sx1276 CACHE INTERNAL "Radio sx1276 selected")
    elseif(MBED_RADIO_SHIELD STREQUAL SX1261MBXBAS OR MBED_RADIO_SHIELD STREQUAL SX1262MBXCAS OR MBED_RADIO_SHIELD STREQUAL SX1262MBXDAS)
        set(RADIO sx126x CACHE INTERNAL "Radio sx126x selected")
    else()
        message(STATUS "Please specify the MBED_RADIO_SHIELD!\nPossible values are: SX1272MB2DAS, SX1276MB1LAS, SX1276MB1MAS, SX1261MBXBAS, SX1262MBXCAS and SX1262MBXDAS.")
    endif()

elseif(BOARD STREQUAL NucleoL152)
    # Configure toolchain for NucleoL152
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/NucleoL152/cmsis/arm-gcc/stm32l152xe_flash.ld)
    include(stm32l1)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/NucleoL152)

    # Configure radio
    if(MBED_RADIO_SHIELD STREQUAL SX1272MB2DAS)
        set(RADIO sx1272 CACHE INTERNAL "Radio sx1272 selected")
    elseif(MBED_RADIO_SHIELD STREQUAL SX1276MB1LAS OR MBED_RADIO_SHIELD STREQUAL SX1276MB1MAS)
        set(RADIO sx1276 CACHE INTERNAL "Radio sx1276 selected")
    elseif(MBED_RADIO_SHIELD STREQUAL SX1261MBXBAS OR MBED_RADIO_SHIELD STREQUAL SX1262MBXCAS OR MBED_RADIO_SHIELD STREQUAL SX1262MBXDAS)
        set(RADIO sx126x CACHE INTERNAL "Radio sx126x selected")
    else()
        message(STATUS "Please specify the MBED_RADIO_SHIELD!\nPossible values are: SX1272MB2DAS, SX1276MB1LAS, SX1276MB1MAS, SX1261MBXBAS, SX1262MBXCAS and SX1262MBXDAS.")
    endif()

elseif(BOARD STREQUAL NucleoL476)
    # Configure toolchain for NucleoL476
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/NucleoL476/cmsis/arm-gcc/stm32l476rgtx_flash.ld)
    include(stm32l4)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/NucleoL476)

    # Configure radio
    if(MBED_RADIO_SHIELD STREQUAL SX1272MB2DAS)
        set(RADIO sx1272 CACHE INTERNAL "Radio sx1272 selected")
    elseif(MBED_RADIO_SHIELD STREQUAL SX1276MB1LAS OR MBED_RADIO_SHIELD STREQUAL SX1276MB1MAS)
        set(RADIO sx1276 CACHE INTERNAL "Radio sx1276 selected")
    elseif(MBED_RADIO_SHIELD STREQUAL SX1261MBXBAS OR MBED_RADIO_SHIELD STREQUAL SX1262MBXCAS OR MBED_RADIO_SHIELD STREQUAL SX1262MBXDAS)
        set(RADIO sx126x CACHE INTERNAL "Radio sx126x selected")
    else()
        message(STATUS "Please specify the MBED_RADIO_SHIELD!\nPossible values are: SX1272MB2DAS, SX1276MB1LAS, SX1276MB1MAS, SX1261MBXBAS, SX1262MBXCAS and SX1262MBXDAS.")
    endif()

elseif(BOARD STREQUAL SAML21)
    # Configure toolchain for SAML21
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/mcu/saml21/saml21b/gcc/gcc/saml21j18b_flash.ld)
    include(saml21)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/SAML21)

    # Configure radio
    set(RADIO sx1276 CACHE INTERNAL "Radio sx1276 selected")

elseif(BOARD STREQUAL SKiM880B)
    # Configure toolchain for SKiM881AXL
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/SKiM880B/cmsis/arm-gcc/stm32l151xba_flash.ld)
    include(stm32l1)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/SKiM880B)

    # Configure radio
    set(RADIO sx1272 CACHE INTERNAL "Radio sx1272 selected")

elseif(BOARD STREQUAL SKiM980A)
    # Configure toolchain for SKiM881AXL
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/SKiM980A/cmsis/arm-gcc/stm32l151xba_flash.ld)
    include(stm32l1)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/SKiM980A)

    # Configure radio
    set(RADIO sx1272 CACHE INTERNAL "Radio sx1272 selected")

elseif(BOARD STREQUAL SKiM881AXL)
    # Configure toolchain for SKiM881AXL
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/SKiM881AXL/cmsis/arm-gcc/stm32l081xx_flash.ld)
    include(stm32l0)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/SKiM881AXL)

    # Configure radio
    set(RADIO sx1272 CACHE INTERNAL "Radio sx1272 selected")

elseif(BOARD STREQUAL B-L072Z-LRWAN1)
    # Configure toolchain for B-L072Z-LRWAN1
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/boards/B-L072Z-LRWAN1/cmsis/arm-gcc/stm32l072xx_flash.ld)
    include(stm32l0)

    # Build platform specific board implementation
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/boards/B-L072Z-LRWAN1)

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

endif()
