cmake_minimum_required(VERSION 3.23)

project(loramac-radio C)

if (NOT TARGET ${PROJECT_NAME})

    set(LORAMAC_RADIO_LIST sx1272 sx1276 sx126x lr1110)
    set(LORAMAC_RADIO sx1272 CACHE STRING "Default radio is sx1272")
    set_property(CACHE LORAMAC_RADIO PROPERTY STRINGS ${LORAMAC_RADIO_LIST})
    set_property(CACHE LORAMAC_RADIO PROPERTY ADVANCED)

    add_library(${PROJECT_NAME} STATIC)

    if (${LORAMAC_RADIO} MATCHES lr1110)
        target_sources(
                ${PROJECT_NAME}
                PUBLIC
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_bootloader.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_bootloader_types.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_crypto_engine.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_crypto_engine_types.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_driver_version.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_gnss.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_gnss_types.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_radio.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_radio_types.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_regmem.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_system.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_system_types.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_wifi.h
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_wifi_types.h

                PRIVATE
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/radio.c
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_bootloader.c
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_crypto_engine.c
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_driver_version.c
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_gnss.c
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_radio.c
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_regmem.c
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_system.c
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src/lr1110_wifi.c
        )
        target_include_directories(
                ${PROJECT_NAME}
                PUBLIC
                ${LORAMAC_SOURCE_DIR}/radio/lr1110
                ${LORAMAC_SOURCE_DIR}/radio/lr1110/lr1110_driver/src

        )
    elseif (${LORAMAC_RADIO} MATCHES sx126x)
        target_sources(
                ${PROJECT_NAME}
                PUBLIC
                ${LORAMAC_SOURCE_DIR}/radio/sx126x/sx126x.h

                PRIVATE
                ${LORAMAC_SOURCE_DIR}/radio/sx126x/radio.c
                ${LORAMAC_SOURCE_DIR}/radio/sx126x/sx126x.c
        )
        target_include_directories(
                ${PROJECT_NAME}
                PUBLIC
                ${LORAMAC_SOURCE_DIR}/radio/sx126x
        )
    elseif (${LORAMAC_RADIO} MATCHES sx1272)
        target_sources(
                ${PROJECT_NAME}
                PUBLIC
                ${LORAMAC_SOURCE_DIR}/radio/sx1272/sx1272.h
                ${LORAMAC_SOURCE_DIR}/radio/sx1272/sx1272Regs-Fsk.h
                ${LORAMAC_SOURCE_DIR}/radio/sx1272/sx1272Regs-LoRa.h

                PRIVATE
                ${LORAMAC_SOURCE_DIR}/radio/sx1272/sx1272.c
        )
        target_include_directories(
                ${PROJECT_NAME}
                PUBLIC
                ${LORAMAC_SOURCE_DIR}/radio/sx1272
        )
    elseif (${LORAMAC_RADIO} MATCHES sx1276)
        target_sources(
                ${PROJECT_NAME}
                PUBLIC
                ${LORAMAC_SOURCE_DIR}/radio/radio.h
                ${LORAMAC_SOURCE_DIR}/radio/sx1276/sx1276.h
                ${LORAMAC_SOURCE_DIR}/radio/sx1276/sx1276Regs-Fsk.h
                ${LORAMAC_SOURCE_DIR}/radio/sx1276/sx1276Regs-LoRa.h

                PRIVATE
                ${LORAMAC_SOURCE_DIR}/radio/sx1276/sx1276.c
        )
        target_include_directories(
                ${PROJECT_NAME}
                PUBLIC
                ${LORAMAC_SOURCE_DIR}/radio/sx1276
        )
    else()
        message(FATAL_ERROR "Unsupported radio driver selected...")
    endif()

    option(LORAMAC_USE_RADIO_DEBUG "Enable Radio Debug GPIO's" OFF)
    target_compile_definitions(${PROJECT_NAME} PUBLIC $<$<BOOL:${LORAMAC_USE_RADIO_DEBUG}>:USE_RADIO_DEBUG>)

    target_include_directories(
            ${PROJECT_NAME} 
            PUBLIC 
            ${LORAMAC_SOURCE_DIR}/radio
    )
    
    target_link_libraries(
            ${PROJECT_NAME}
            PUBLIC
            loramac-system
    )
    
endif()
