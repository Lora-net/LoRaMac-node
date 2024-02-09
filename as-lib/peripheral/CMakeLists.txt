cmake_minimum_required(VERSION 3.23)

project(loramac-peripheral C)

if (NOT TARGET ${PROJECT_NAME})

    option(LORAMAC_SECURE_ELEMENT_PRE_PROVISIONED "Secure-element pre-provisioning" ON)

    set(LORAMAC_SECURE_ELEMENT_LIST SOFT_SE LR1110_SE ATECC608A_TNGLORA_SE)
    set(LORAMAC_SECURE_ELEMENT SOFT_SE CACHE STRING "Default secure element is SOFT_SE")
    set_property(CACHE LORAMAC_SECURE_ELEMENT PROPERTY STRINGS ${LORAMAC_SECURE_ELEMENT_LIST})

    add_library(${PROJECT_NAME} STATIC)

    if (LORAMAC_SECURE_ELEMENT MATCHES SOFT_SE)
        list(
                APPEND
                ${PROJECT_NAME}_HEADERS
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/aes.h
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/cmac.h
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/se-identity.h
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/soft-se-hal.h
        )
        list(
                APPEND
                ${PROJECT_NAME}_SOURCES
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/aes.c
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/cmac.c
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/soft-se.c
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/soft-se-hal.c
        )
        target_compile_definitions(
                ${PROJECT_NAME}
                PUBLIC
                SOFT_SE
        )
        target_include_directories(
                ${PROJECT_NAME}
                PUBLIC
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se
                ${LORAMAC_SOURCE_DIR}/mac
        )
    elseif(LORAMAC_SECURE_ELEMENT MATCHES LR1110_SE)
        if (LORAMAC_RADIO MATCHES lr1110)
            list(
                    APPEND
                    ${PROJECT_NAME}_HEADERS
                    ${LORAMAC_SOURCE_DIR}/peripherals/lr1110-se/lr1110-se-hal.h
                    ${LORAMAC_SOURCE_DIR}/peripherals/lr1110-se/se-identity.h
            )
            list(
                    APPEND
                    ${PROJECT_NAME}_SOURCES
                    ${LORAMAC_SOURCE_DIR}/peripherals/lr1110-se/lr1110-se.c
                    ${LORAMAC_SOURCE_DIR}/peripherals/lr1110-se/lr1110-se-hal.c
            )
            target_include_directories(
                    ${PROJECT_NAME} PUBLIC
                    ${LORAMAC_SOURCE_DIR}/peripherals/lr1110-se
            )
        else()
            message(FATAL_ERROR "LR1110_SE secure element can only be used when LR1110 radio is selected.")
        endif()
    elseif(LORAMAC_SECURE_ELEMENT MATCHES ATECC608A_TNGLORA_SE)
        file(
                GLOB
                ${PROJECT_NAME}_HEADERS
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/*.h
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/*.h
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/basic/*.h
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/crypto/*.h
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/crypto/hashes/*.h
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/hal/atca_hal.h
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/host/*.h
        )
        file(
                GLOB
                ${PROJECT_NAME}_SOURCES
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/*.c
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/*.c
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/basic/*.c
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/crypto/*.c
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/crypto/hashes/*.c
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/hal/atca_hal.c
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/host/*.c
        )
        target_include_directories(
                ${PROJECT_NAME} PUBLIC
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/basic
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/crypto
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/crypto/hashes
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/hal
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/cryptoauthlib/lib/host
        )
    else()
        message(FATAL_ERROR "LORAMAC_SECURE_ELEMENT not defined")
    endif()

    list(
            APPEND
            ${PROJECT_NAME}_HEADERS
            ${LORAMAC_SOURCE_DIR}/peripherals/gpio-ioe.h
            ${LORAMAC_SOURCE_DIR}/peripherals/mag3110.h
            ${LORAMAC_SOURCE_DIR}/peripherals/mma8451.h
            ${LORAMAC_SOURCE_DIR}/peripherals/mpl3115.h
            ${LORAMAC_SOURCE_DIR}/peripherals/pam7q.h
            ${LORAMAC_SOURCE_DIR}/peripherals/sx1509.h
            ${LORAMAC_SOURCE_DIR}/peripherals/sx9500.h
    )

    list(
            APPEND
            ${PROJECT_NAME}_SOURCES
            ${LORAMAC_SOURCE_DIR}/peripherals/gpio-ioe.c
            ${LORAMAC_SOURCE_DIR}/peripherals/mag3110.c
            ${LORAMAC_SOURCE_DIR}/peripherals/mma8451.c
            ${LORAMAC_SOURCE_DIR}/peripherals/mpl3115.c
            ${LORAMAC_SOURCE_DIR}/peripherals/pam7q.c
            ${LORAMAC_SOURCE_DIR}/peripherals/sx1509.c
            ${LORAMAC_SOURCE_DIR}/peripherals/sx9500.c
    )

    target_sources(
            ${PROJECT_NAME}
            PUBLIC
            ${${PROJECT_NAME}_HEADERS}

            PRIVATE
            ${${PROJECT_NAME}_SOURCES}
    )

    target_compile_definitions(
            ${PROJECT_NAME}
            PRIVATE
            $<$<BOOL:${LORAMAC_SECURE_ELEMENT_PRE_PROVISIONED}>:SECURE_ELEMENT_PRE_PROVISIONED>
    )

    target_link_libraries(
            ${PROJECT_NAME}
            PUBLIC
            loramac-board
            loramac-radio
            loramac-system
    )
endif()
