cmake_minimum_required(VERSION 3.23)

project(loramac-board C)

if (NOT TARGET ${PROJECT_NAME})

    add_library(${PROJECT_NAME} STATIC)

    target_sources(
            ${PROJECT_NAME}

            PUBLIC
            ${LORAMAC_SOURCE_DIR}/boards/adc-board.h
            ${LORAMAC_SOURCE_DIR}/boards/board.h
            ${LORAMAC_SOURCE_DIR}/boards/delay-board.h
            ${LORAMAC_SOURCE_DIR}/boards/display-board.h
            ${LORAMAC_SOURCE_DIR}/boards/eeprom-board.h
            ${LORAMAC_SOURCE_DIR}/boards/gpio-board.h
            ${LORAMAC_SOURCE_DIR}/boards/gps-board.h
            ${LORAMAC_SOURCE_DIR}/boards/i2c-board.h
            ${LORAMAC_SOURCE_DIR}/boards/lpm-board.h
            ${LORAMAC_SOURCE_DIR}/boards/lr1110-board.h
            ${LORAMAC_SOURCE_DIR}/boards/pinName-board.h
            ${LORAMAC_SOURCE_DIR}/boards/pinName-ioe.h
            ${LORAMAC_SOURCE_DIR}/boards/rtc-board.h
            ${LORAMAC_SOURCE_DIR}/boards/sx126x-board.h
            ${LORAMAC_SOURCE_DIR}/boards/sx1272-board.h
            ${LORAMAC_SOURCE_DIR}/boards/sx1276-board.h
            ${LORAMAC_SOURCE_DIR}/boards/uart-board.h
            ${LORAMAC_SOURCE_DIR}/boards/uart-usb-board.h
            ${LORAMAC_SOURCE_DIR}/boards/utilities.h

            PRIVATE
            ${LORAMAC_SOURCE_DIR}/boards/mcu/utilities.c
    )

    target_include_directories(
            ${PROJECT_NAME}
            PUBLIC
            ${LORAMAC_SOURCE_DIR}/boards
    )

endif()
