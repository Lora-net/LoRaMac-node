cmake_minimum_required(VERSION 3.23)

project(loramac${LORAMAC_SUFFIX} C)

message(STATUS ${PROJECT_NAME})

if (NOT TARGET ${PROJECT_NAME})
    add_library(${PROJECT_NAME} STATIC)
    target_sources(
            ${PROJECT_NAME}
            PUBLIC
            ${LORAMAC_SOURCE_DIR}/apps/LoRaMac/common/LmHandler/LmHandler.h
            ${LORAMAC_SOURCE_DIR}/apps/LoRaMac/common/LmHandler/LmHandlerTypes.h
            ${LORAMAC_SOURCE_DIR}/apps/LoRaMac/common/LmHandler/packages/LmhPackage.h
            ${LORAMAC_SOURCE_DIR}/apps/LoRaMac/common/NvmDataMgmt.h
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
            $<$<BOOL:REGION_EU868>:${LORAMAC_SOURCE_DIR}/mac/region/RegionEU868.h>
            $<$<BOOL:REGION_AS923>:${LORAMAC_SOURCE_DIR}/mac/region/RegionAS923.h>
            $<$<BOOL:REGION_AU915>:${LORAMAC_SOURCE_DIR}/mac/region/RegionAU915.h>
            $<$<BOOL:REGION_US915>:${LORAMAC_SOURCE_DIR}/mac/region/RegionUS915.h>
            $<$<OR:$<BOOL:REGION_AU915>,$<BOOL:REGION_US915>>:${LORAMAC_SOURCE_DIR}/mac/region/RegionBaseUS.h>

            PRIVATE
            ${LORAMAC_SOURCE_DIR}/apps/LoRaMac/common/LmHandler/LmHandler.c
            ${LORAMAC_SOURCE_DIR}/apps/LoRaMac/common/NvmDataMgmt.c
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
            $<$<BOOL:REGION_EU868>:${LORAMAC_SOURCE_DIR}/mac/region/RegionEU868.c>
            $<$<BOOL:REGION_AS923>:${LORAMAC_SOURCE_DIR}/mac/region/RegionAS923.c>
            $<$<BOOL:REGION_AU915>:${LORAMAC_SOURCE_DIR}/mac/region/RegionAU915.c>
            $<$<BOOL:REGION_US915>:${LORAMAC_SOURCE_DIR}/mac/region/RegionUS915.c>

            $<$<OR:$<BOOL:REGION_AU915>,$<BOOL:REGION_US915>>:${LORAMAC_SOURCE_DIR}/mac/region/RegionBaseUS.c>
    )

    target_compile_definitions(
            ${PROJECT_NAME}
            PUBLIC
            $<$<BOOL:REGION_EU868>:REGION_EU868>
            $<$<BOOL:REGION_AS923>:REGION_AS923>
            $<$<BOOL:REGION_AU915>:REGION_AU915>
            $<$<BOOL:REGION_US915>:REGION_US915>
            ACTIVE_REGION                               # TODO: remove from lmHandler.c?
    )

    target_include_directories(
            ${PROJECT_NAME}
            PUBLIC
            ${LORAMAC_SOURCE_DIR}/apps/LoRaMac/common/LmHandler
            ${LORAMAC_SOURCE_DIR}/apps/LoRaMac/common/LmHandler/packages
            ${LORAMAC_SOURCE_DIR}/apps/LoRaMac/common
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