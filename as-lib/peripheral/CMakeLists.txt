cmake_minimum_required(VERSION 3.23)

project(loramac-peripheral C)

if (NOT TARGET ${PROJECT_NAME})

    option(LORAMAC_SECURE_ELEMENT_PRE_PROVISIONED "Secure-element pre-provisioning" ON)
    option(LORAMAC_AES_DEC_PREKEYED "AES DEC Prekeyed" ON)

    set(LORAMAC_SECURE_ELEMENT_LIST SOFT_SE LR1110_SE ATECC608A_TNGLORA_SE)
    set(LORAMAC_SECURE_ELEMENT SOFT_SE CACHE STRING "Default secure element is SOFT_SE")
    set_property(CACHE LORAMAC_SECURE_ELEMENT PROPERTY STRINGS ${LORAMAC_SECURE_ELEMENT_LIST})

    add_library(${PROJECT_NAME} STATIC)

    if (LORAMAC_SECURE_ELEMENT MATCHES SOFT_SE)
        target_sources(
                ${PROJECT_NAME}
                PUBLIC
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/aes.h
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/cmac.h
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/se-identity.h
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/soft-se-hal.h

                PRIVATE
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/aes.c
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/cmac.c
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/soft-se.c
                ${LORAMAC_SOURCE_DIR}/peripherals/soft-se/soft-se-hal.c
        )
        target_compile_definitions(
                ${PROJECT_NAME}
                PUBLIC
                SOFT_SE
                $<$<BOOL:${LORAMAC_AES_DEC_PREKEYED}>:AES_DEC_PREKEYED>
        )
    elseif(LORAMAC_SECURE_ELEMENT MATCHES LR1110_SE)
        if (LORAMAC_RADIO MATCHES lr1110)
            target_sources(
                    ${PROJECT_NAME}
                    PUBLIC
                    ${LORAMAC_SOURCE_DIR}/peripherals/lr1110-se/lr1110-se-hal.h
                    ${LORAMAC_SOURCE_DIR}/peripherals/lr1110-se/se-identity.h

                    PRIVATE
                    ${LORAMAC_SOURCE_DIR}/peripherals/lr1110-se/lr1110-se.c
                    ${LORAMAC_SOURCE_DIR}/peripherals/lr1110-se/lr1110-se-hal.c
            )
        else()
            message(FATAL_ERROR "LR1110_SE secure element can only be used when LR1110 radio is selected.")
        endif()
    elseif(LORAMAC_SECURE_ELEMENT MATCHES ATECC608A_TNGLORA_SE)
        target_sources(
                ${PROJECT_NAME}
                PUBLIC
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/atca_config.h
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/atecc608a-tnglora-se-hal.h
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/se-identity.h

                PRIVATE
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/atecc608a-tnglora-se.c
                ${LORAMAC_SOURCE_DIR}/peripherals/atecc608a-tnglora-se/atecc608a-tnglora-se-hal.c
        )
    else()
        message(FATAL_ERROR "LORAMAC_SECURE_ELEMENT not defined")
    endif()

    target_compile_definitions(
            ${PROJECT_NAME}
            PRIVATE
            $<$<BOOL:${LORAMAC_SECURE_ELEMENT_PRE_PROVISIONED}>:SECURE_ELEMENT_PRE_PROVISIONED>
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
