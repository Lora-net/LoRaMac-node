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
project(mac)
cmake_minimum_required(VERSION 3.6)

#---------------------------------------------------------------------------------------
# Options
#---------------------------------------------------------------------------------------

# Allow selection of region
option(REGION_EU868 "Region EU868" ON)
option(REGION_US915 "Region US915" OFF)
option(REGION_CN779 "Region CN779" OFF)
option(REGION_EU433 "Region EU433" OFF)
option(REGION_AU915 "Region AU915" OFF)
option(REGION_AS923 "Region AS923" OFF)
option(REGION_CN470 "Region CN470" OFF)
option(REGION_KR920 "Region KR920" OFF)
option(REGION_IN865 "Region IN865" OFF)
option(REGION_RU864 "Region RU864" OFF)
set(REGION_LIST REGION_EU868 REGION_US915 REGION_CN779 REGION_EU433 REGION_AU915 REGION_AS923 REGION_CN470 REGION_KR920 REGION_IN865 REGION_RU864)

# AS923 Channel Plan
set(REGION_AS923_DEFAULT_CHANNEL_PLAN_LIST CHANNEL_PLAN_GROUP_AS923_1 CHANNEL_PLAN_GROUP_AS923_2 CHANNEL_PLAN_GROUP_AS923_3 CHANNEL_PLAN_GROUP_AS923_1_JP)
set(REGION_AS923_DEFAULT_CHANNEL_PLAN CHANNEL_PLAN_GROUP_AS923_1 CACHE STRING "Default channel plan for AS923 is CHANNEL_PLAN_GROUP_AS923_1")
set_property(CACHE REGION_AS923_DEFAULT_CHANNEL_PLAN PROPERTY STRINGS ${REGION_AS923_DEFAULT_CHANNEL_PLAN_LIST})

# CN470 Channel Plan
set(REGION_CN470_DEFAULT_CHANNEL_PLAN_LIST CHANNEL_PLAN_20MHZ_TYPE_A CHANNEL_PLAN_20MHZ_TYPE_B CHANNEL_PLAN_26MHZ_TYPE_A CHANNEL_PLAN_26MHZ_TYPE_B)
set(REGION_CN470_DEFAULT_CHANNEL_PLAN CHANNEL_PLAN_20MHZ_TYPE_A CACHE STRING "Default channel plan for CN470 is CHANNEL_PLAN_20MHZ_TYPE_A")
set_property(CACHE REGION_CN470_DEFAULT_CHANNEL_PLAN PROPERTY STRINGS ${REGION_CN470_DEFAULT_CHANNEL_PLAN_LIST})


#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------
set( MAC_BUILD_SOURCES
     ${CMAKE_CURRENT_SOURCE_DIR}/region/RegionAS923.c
     ${CMAKE_CURRENT_SOURCE_DIR}/region/RegionCN779.c
     ${CMAKE_CURRENT_SOURCE_DIR}/region/RegionEU433.c
     ${CMAKE_CURRENT_SOURCE_DIR}/region/RegionEU868.c
     ${CMAKE_CURRENT_SOURCE_DIR}/region/RegionIN865.c
     ${CMAKE_CURRENT_SOURCE_DIR}/region/RegionRU864.c
     ${CMAKE_CURRENT_SOURCE_DIR}/region/RegionKR920.c
     ${CMAKE_CURRENT_SOURCE_DIR}/region/RegionBaseUS.c
     ${CMAKE_CURRENT_SOURCE_DIR}/region/RegionCommon.c
     ${CMAKE_CURRENT_SOURCE_DIR}/region/Region.c
     ${CMAKE_CURRENT_SOURCE_DIR}/LoRaMac.c
     ${CMAKE_CURRENT_SOURCE_DIR}/LoRaMacAdr.c
     ${CMAKE_CURRENT_SOURCE_DIR}/LoRaMacClassB.c
     ${CMAKE_CURRENT_SOURCE_DIR}/LoRaMacCommands.c
     ${CMAKE_CURRENT_SOURCE_DIR}/LoRaMacConfirmQueue.c
     ${CMAKE_CURRENT_SOURCE_DIR}/LoRaMacCrypto.c
     ${CMAKE_CURRENT_SOURCE_DIR}/LoRaMacParser.c
     ${CMAKE_CURRENT_SOURCE_DIR}/LoRaMacSerializer.c )

if(REGION_US915 STREQUAL ON)
set( MAC_BUILD_SOURCES
     ${MAC_BUILD_SOURCES}
     "${CMAKE_CURRENT_SOURCE_DIR}/region/RegionUS915.c" )
endif()

if(REGION_AU915 STREQUAL ON)
set( MAC_BUILD_SOURCES
     ${MAC_BUILD_SOURCES}
     "${CMAKE_CURRENT_SOURCE_DIR}/region/RegionAU915.c" )
endif()

if(REGION_CN470 STREQUAL ON)
set( MAC_BUILD_SOURCES
     ${MAC_BUILD_SOURCES}
     "${CMAKE_CURRENT_SOURCE_DIR}/region/RegionCN470.c"
     "${CMAKE_CURRENT_SOURCE_DIR}/region/RegionCN470A20.c"
     "${CMAKE_CURRENT_SOURCE_DIR}/region/RegionCN470B20.c"
     "${CMAKE_CURRENT_SOURCE_DIR}/region/RegionCN470A26.c"
     "${CMAKE_CURRENT_SOURCE_DIR}/region/RegionCN470B26.c" )
endif()

add_library(${PROJECT_NAME} OBJECT EXCLUDE_FROM_ALL ${MAC_BUILD_SOURCES})

# Loops through all regions and add compile time definitions for the enabled ones.
foreach( REGION ${REGION_LIST} )
    if(${REGION})
        target_compile_definitions(${PROJECT_NAME} PUBLIC -D"${REGION}")
    endif()
endforeach()

# Applies AS923 channel plan
target_compile_definitions(${PROJECT_NAME} PRIVATE -DREGION_AS923_DEFAULT_CHANNEL_PLAN=${REGION_AS923_DEFAULT_CHANNEL_PLAN})

# Applies CN470 channel plan
target_compile_definitions(${PROJECT_NAME} PRIVATE -DREGION_CN470_DEFAULT_CHANNEL_PLAN=${REGION_CN470_DEFAULT_CHANNEL_PLAN})


# Add define if class B is supported
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<BOOL:${CLASSB_ENABLED}>:LORAMAC_CLASSB_ENABLED>)

# SecureElement NVM
if(${SECURE_ELEMENT} MATCHES SOFT_SE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DSOFT_SE)
endif()

add_dependencies(${PROJECT_NAME} board)

target_include_directories( ${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/region
    $<TARGET_PROPERTY:system,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:radio,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:board,INTERFACE_INCLUDE_DIRECTORIES>
)

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 11)
