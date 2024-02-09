cmake_minimum_required(VERSION 3.23)

project(loramac-system C)

if (NOT TARGET ${PROJECT_NAME})

    add_library(${PROJECT_NAME} STATIC)

    target_sources(
            ${PROJECT_NAME}
            PUBLIC
            ${LORAMAC_SOURCE_DIR}/system/adc.h
            ${LORAMAC_SOURCE_DIR}/system/delay.h
            ${LORAMAC_SOURCE_DIR}/system/fifo.h
            ${LORAMAC_SOURCE_DIR}/system/gpio.h
            ${LORAMAC_SOURCE_DIR}/system/gps.h
            ${LORAMAC_SOURCE_DIR}/system/i2c.h
            ${LORAMAC_SOURCE_DIR}/system/nvmm.h
            ${LORAMAC_SOURCE_DIR}/system/serial.h
            ${LORAMAC_SOURCE_DIR}/system/spi.h
            ${LORAMAC_SOURCE_DIR}/system/systime.h
            ${LORAMAC_SOURCE_DIR}/system/timer.h
            ${LORAMAC_SOURCE_DIR}/system/uart.h

            PRIVATE
            ${LORAMAC_SOURCE_DIR}/system/adc.c
            ${LORAMAC_SOURCE_DIR}/system/delay.c
            ${LORAMAC_SOURCE_DIR}/system/fifo.c
            ${LORAMAC_SOURCE_DIR}/system/gpio.c
            ${LORAMAC_SOURCE_DIR}/system/gps.c
            ${LORAMAC_SOURCE_DIR}/system/i2c.c
            ${LORAMAC_SOURCE_DIR}/system/nvmm.c
            ${LORAMAC_SOURCE_DIR}/system/systime.c
            ${LORAMAC_SOURCE_DIR}/system/timer.c
            ${LORAMAC_SOURCE_DIR}/system/uart.c
    )

    target_include_directories(
            ${PROJECT_NAME}
            PUBLIC
            ${LORAMAC_SOURCE_DIR}/system
    )

    target_link_libraries(
            ${PROJECT_NAME}
            PUBLIC
            loramac-board
    )
endif()
