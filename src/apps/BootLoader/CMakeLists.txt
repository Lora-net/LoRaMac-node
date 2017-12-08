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
project(BootLoader)
cmake_minimum_required(VERSION 3.6)
enable_language(ASM)

#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------

file(GLOB ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}/*.c"
)

list(APPEND ${PROJECT_NAME}_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/${BOARD}/gpio-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/${BOARD}/i2c-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_cortex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_flash.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_flash_ex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_flash_ramfunc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_gpio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_i2c.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_pcd.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_pcd_ex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_pwr.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_pwr_ex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_rcc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32L1xx_HAL_Driver/Src/stm32l1xx_hal_rcc_ex.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32_USB_Device_Library/Core/Src/usbd_core.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/${BOARD}/usb/dfu/src/usbd_conf.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/${BOARD}/usb/dfu/src/usbd_desc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/${BOARD}/usb/dfu/src/usbd_dfu_flash.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32_USB_Device_Library/Class/DFU/Src/usbd_dfu.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/sysIrqHandlers.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/${BOARD}/cmsis/arm-gcc/startup_stm32l151xb.s"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/${BOARD}/cmsis/system_stm32l1xx.c"
)

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES}
                            $<TARGET_OBJECTS:system>
                            $<TARGET_OBJECTS:peripherals>
)

target_compile_definitions(${PROJECT_NAME} PUBLIC
    -DUSE_NO_TIMER
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:${BOARD},INTERFACE_COMPILE_DEFINITIONS>>
)

target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32_USB_Device_Library/Core/Inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/${BOARD}/usb/dfu/inc"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../boards/mcu/stm32/STM32_USB_Device_Library/Class/DFU/Inc"
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:system,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:peripherals,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:${BOARD},INTERFACE_INCLUDE_DIRECTORIES>>
)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)

target_link_libraries(${PROJECT_NAME} m)

#---------------------------------------------------------------------------------------
# Debugging and Binutils
#---------------------------------------------------------------------------------------

include(gdb-helper)
include(binutils-arm-none-eabi)

# Generate debugger configurations
generate_run_gdb_stlink(${PROJECT_NAME})
generate_run_gdb_openocd(${PROJECT_NAME})
generate_vscode_launch_openocd(${PROJECT_NAME})

# Print section sizes of target
print_section_sizes(${PROJECT_NAME})

# Create output in hex and binary format
create_bin_output(${PROJECT_NAME})
create_hex_output(${PROJECT_NAME})
