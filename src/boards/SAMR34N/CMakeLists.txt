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
## Authors:  Johannes Bruder (STACKFORCE), Miguel Luis (Semtech) and 
##           Marten Lootsma(TWTG) on behalf of Microchip/Atmel (c)2017
##
project(SAMR34N)
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
    "${CMAKE_CURRENT_SOURCE_DIR}/sx1276-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/uart-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/i2c-board.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/utilities.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/sercom/i2c/i2c_sam0/i2c_master.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/sercom/i2c/i2c_sam0/i2c_master_interrupt.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/common2/services/delay/sam0/systick_counter.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/common/utils/interrupt/interrupt_sam_nvic.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/boards/samr34_xplained_pro/board_init.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/adc/adc_sam_l_c/adc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/adc/adc_sam_l_c/adc_callback.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/extint/extint_callback.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/extint/extint_sam_l_c/extint.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/nvm/nvm.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/port/port.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/rtc/rtc_sam_l_c/rtc_count.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/rtc/rtc_sam_l_c/rtc_count_interrupt.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/sercom/sercom.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/sercom/sercom_interrupt.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/sercom/spi/spi.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/sercom/usart/usart.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/sercom/usart/usart_interrupt.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/clock/clock_samr34/clock.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/clock/clock_samr34/gclk.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/interrupt/system_interrupt.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/pinmux/pinmux.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/system.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/tc/tc_interrupt.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/tc/tc_sam_l_c/tc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/services/eeprom/emulator/rwwee_array/rww_eeprom.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/utils/cmsis/samr34/source/gcc/startup_samr34.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/utils/cmsis/samr34/source/system_samr34.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/utils/stdio/read.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/utils/stdio/write.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/utils/syscalls/gcc/syscalls.c"
)

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${${PROJECT_NAME}_SOURCES})

target_compile_definitions(${PROJECT_NAME} PUBLIC -D__SAMR34J18B__ -DARM_MATH_CM0PLUS=true -DRTC_COUNT_ASYNC=true -DSYSTICK_MODE -DADC_CALLBACK_MODE=true -DEVENTS_INTERRUPT_HOOKS_MODE=true -DTC_ASYNC=true -D_DEBUG_=0 -DBOARD=SAMR34_XPLAINED_PRO -DEXTINT_CALLBACK_MODE=true -DUSART_CALLBACK_MODE=true -DSPI_CALLBACK_MODE=false -DEDBG_EUI_READ=0 -DI2C_MASTER_CALLBACK_MODE=true)

# Add define if radio debug pins support is enabled
target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<BOOL:${USE_RADIO_DEBUG}>:USE_RADIO_DEBUG>)

# For debug builds set the symbol DEBUG
set(CMAKE_C_FLAGS_DEBUG -DDEBUG)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/config
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/common/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/common/boards
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/common/services/serial
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/common2/services/delay
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/common2/services/delay/sam0
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/boards
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/boards/samr34_xplained_pro
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/utils/cmsis/samr34/source
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/utils/header_files
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/utils/preprocessor
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/utils/cmsis/samr34/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/adc
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/adc/adc_sam_l_c
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/dma
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/events
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/events/events_sam_l_c
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/extint
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/nvm
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/port
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/sercom
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/sercom/i2c
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/sercom/i2c/i2c_sam0
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/sercom/spi
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/sercom/usart
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/rtc
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/clock/clock_samr34
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/clock
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/interrupt
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/interrupt/system_interrupt_samr34
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/pinmux
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/power/power_sam_l
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/power
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/reset/reset_sam_l
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/system/reset
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/drivers/tc
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/services/eeprom/emulator/rwwee_array
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/sam0/utils/stdio/stdio_serial
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/thirdparty/CMSIS/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../mcu/samr34/ASF/thirdparty/CMSIS/Lib/GCC
    $<TARGET_PROPERTY:board,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:system,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:radio,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:peripherals,INTERFACE_INCLUDE_DIRECTORIES>
)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)
