cmake_minimum_required(VERSION 3.23)

if (NOT DEFINED LORAMAC_SUFFIX)
    set(LORAMAC_SUFFIX "" CACHE STRING "Default suffix for LoRaMac Static Library builds")
endif()

project(loramac${LORAMAC_SUFFIX} C)

if (NOT TARGET ${PROJECT_NAME})

    set(LORAMAC_REGION_LIST EU868 US915 CN779 EU433 AU915 AS923 CN470 KR920 IN865 RU864)

    add_library(${PROJECT_NAME} STATIC)
    target_sources(
            ${PROJECT_NAME}
            PUBLIC
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMac.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacAdr.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacClassB.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacClassBConfig.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacClassBNvm.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacCommands.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacConfirmQueue.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacCrypto.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacCryptoNvm.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacHeaderTypes.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacMessageTypes.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacParser.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacSerializer.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacTest.h
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacTypes.h
            ${LORAMAC_SOURCE_DIR}/mac/secure-element.h
            ${LORAMAC_SOURCE_DIR}/mac/secure-element-nvm.h
            ${LORAMAC_SOURCE_DIR}/mac/region/Region.h
            ${LORAMAC_SOURCE_DIR}/mac/region/RegionCommon.h
            $<$<OR:$<BOOL:LORAMAC_REGION_AU915>,$<BOOL:LORAMAC_REGION_US915>>:${LORAMAC_SOURCE_DIR}/mac/region/RegionBaseUS.h>
            $<$<BOOL:${LORAMAC_REGION_CN470}>:
                ${LORAMAC_SOURCE_DIR}/mac/region/RegionCN470A20.h
                ${LORAMAC_SOURCE_DIR}/mac/region/RegionCN470B20.h
                ${LORAMAC_SOURCE_DIR}/mac/region/RegionCN470A26.h
                ${LORAMAC_SOURCE_DIR}/mac/region/RegionCN470B26.h
            >

            PRIVATE
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMac.c
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacAdr.c
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacClassB.c
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacCommands.c
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacConfirmQueue.c
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacCrypto.c
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacParser.c
            ${LORAMAC_SOURCE_DIR}/mac/LoRaMacSerializer.c
            ${LORAMAC_SOURCE_DIR}/mac/region/Region.c
            ${LORAMAC_SOURCE_DIR}/mac/region/RegionCommon.c
            $<$<OR:$<BOOL:${LORAMAC_REGION_AU915}>,$<BOOL:${LORAMAC_REGION_US915}>>:${LORAMAC_SOURCE_DIR}/mac/region/RegionBaseUS.c>
            $<$<BOOL:${LORAMAC_REGION_CN470}>:
                ${LORAMAC_SOURCE_DIR}/mac/region/RegionCN470A20.c
                ${LORAMAC_SOURCE_DIR}/mac/region/RegionCN470B20.c
                ${LORAMAC_SOURCE_DIR}/mac/region/RegionCN470A26.c
                ${LORAMAC_SOURCE_DIR}/mac/region/RegionCN470B26.c
            >
    )

    set(ACTIVE_REGION_COUNT 0)

    foreach(REGION ${LORAMAC_REGION_LIST})
        if (${LORAMAC_REGION_${REGION}})

            MATH(EXPR ACTIVE_REGION_COUNT "${ACTIVE_REGION_COUNT}+1")

            target_sources(
                    ${PROJECT_NAME}
                    PUBLIC
                    ${LORAMAC_SOURCE_DIR}/mac/region/Region${REGION}.h

                    PRIVATE
                    ${LORAMAC_SOURCE_DIR}/mac/region/Region${REGION}.c
            )

            target_compile_definitions(
                    ${PROJECT_NAME}
                    PUBLIC
                    $<$<BOOL:${LORAMAC_REGION_${REGION}}>:REGION_${REGION}>
            )

        endif()
    endforeach()

    if (${ACTIVE_REGION_COUNT} LESS 1)
        message(FATAL_ERROR "No LORAMAC_REGION_xxx's specified")
    endif()

    target_compile_definitions(
            ${PROJECT_NAME}
            PUBLIC
    )

    target_include_directories(
            ${PROJECT_NAME}
            PUBLIC
            ${LORAMAC_SOURCE_DIR}/mac
            ${LORAMAC_SOURCE_DIR}/mac/region
    )

    target_link_libraries(
            ${PROJECT_NAME}
            PUBLIC
            loramac-board
            loramac-system
            loramac-radio
            loramac-peripheral
    )

endif()