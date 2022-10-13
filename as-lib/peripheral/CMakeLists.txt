cmake_minimum_required(VERSION 3.23)

project(loramac-peripheral C)

if (NOT TARGET ${PROJECT_NAME})
    add_library(${PROJECT_NAME} STATIC)
    target_sources(
            ${PROJECT_NAME}
            PUBLIC
            ${LORAMAC_SOURCE_DIR}/peripherals/gpio-ioe.h
            ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/aes.h
            ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/cmac.h
            ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/se-identity.h
            ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/soft-se-hal.h
    
            PRIVATE
            ${LORAMAC_SOURCE_DIR}/peripherals/gpio-ioe.c
            ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/aes.c
            ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/cmac.c
            ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/soft-se.c
            ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/soft-se-hal.c
    )

    target_compile_definitions(
            ${PROJECT_NAME}
            PUBLIC
            SOFT_SE
            AES_DEC_PREKEYED
    )

    target_include_directories(
            ${PROJECT_NAME}
            PUBLIC
            ${LORAMAC_SOURCE_DIR}/peripherals/soft-se
            ${LORAMAC_SOURCE_DIR}/mac
    )

    target_link_libraries(
            ${PROJECT_NAME}
            PUBLIC
            loramac-board
            loramac-radio
            loramac-system
    )
endif()
