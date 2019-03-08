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
project(LoRaMac)
cmake_minimum_required(VERSION 3.6)

#---------------------------------------------------------------------------------------
# Options
#---------------------------------------------------------------------------------------

# Allow switching of sub projects
set(SUB_PROJECT_LIST classA classB classC periodic-uplink-lpp fuota-test-01)
set(SUB_PROJECT classA CACHE STRING "Default sub project is Class A")
set_property(CACHE SUB_PROJECT PROPERTY STRINGS ${SUB_PROJECT_LIST})

# Allow switching of active region
set(ACTIVE_REGION_LIST   LORAMAC_REGION_EU868 LORAMAC_REGION_US915 LORAMAC_REGION_CN779
    LORAMAC_REGION_EU433 LORAMAC_REGION_AU915 LORAMAC_REGION_AS923 LORAMAC_REGION_CN470
    LORAMAC_REGION_KR920 LORAMAC_REGION_IN865 LORAMAC_REGION_RU864
)
set(ACTIVE_REGION LORAMAC_REGION_EU868 CACHE STRING "Default active region is EU868")
set_property(CACHE ACTIVE_REGION PROPERTY STRINGS ${ACTIVE_REGION_LIST})

if((SUB_PROJECT STREQUAL classB OR SUB_PROJECT STREQUAL periodic-uplink-lpp OR SUB_PROJECT STREQUAL fuota-test-01) AND NOT CLASSB_ENABLED )
    message(FATAL_ERROR "Please turn on Class B support of LoRaMac ( CLASSB_ENABLED=ON ) to use Class B, periodic-uplink-lpp, fuota-test-01 sub projects")
endif()

if(SUB_PROJECT STREQUAL periodic-uplink-lpp)

    #---------------------------------------------------------------------------------------
    # Application common features handling
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_COMMON
        "${CMAKE_CURRENT_LIST_DIR}/common/CayenneLpp.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandlerMsgDisplay.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/NvmCtxMgmt.c"
    )

    #---------------------------------------------------------------------------------------
    # Application LoRaMac handler
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_LMH
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/LmHandler.c"
    )

    #---------------------------------------------------------------------------------------
    # LoRaMac handler applicative packages
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_LMHP
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/FragDecoder.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpClockSync.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpCompliance.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpFragmentation.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpRemoteMcastSetup.c"
    )

elseif(SUB_PROJECT STREQUAL fuota-test-01)

    #---------------------------------------------------------------------------------------
    # Application common features handling
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_COMMON
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandlerMsgDisplay.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/NvmCtxMgmt.c"
    )

    #---------------------------------------------------------------------------------------
    # Application LoRaMac handler
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_LMH
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/LmHandler.c"
    )

    #---------------------------------------------------------------------------------------
    # LoRaMac handler applicative packages
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_LMHP
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/FragDecoder.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpClockSync.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpCompliance.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpFragmentation.c"
        "${CMAKE_CURRENT_LIST_DIR}/common/LmHandler/packages/LmhpRemoteMcastSetup.c"
    )

else() #if(SUB_PROJECT STREQUAL classA OR SUB_PROJECT STREQUAL classB OR SUB_PROJECT STREQUAL classC)

    #---------------------------------------------------------------------------------------
    # Application common features handling
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_COMMON
        "${CMAKE_CURRENT_LIST_DIR}/common/NvmCtxMgmt.c"
    )

    #---------------------------------------------------------------------------------------
    # Application LoRaMac handler
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_LMH
    )

    #---------------------------------------------------------------------------------------
    # LoRaMac handler applicative packages
    #---------------------------------------------------------------------------------------
    list(APPEND ${PROJECT_NAME}_LMHP
    )

endif()

#---------------------------------------------------------------------------------------
# Application
#---------------------------------------------------------------------------------------
file(GLOB ${PROJECT_NAME}_SOURCES "${CMAKE_CURRENT_LIST_DIR}/${SUB_PROJECT}/${BOARD}/*.c")

add_executable(${PROJECT_NAME}-${SUB_PROJECT}
                            ${${PROJECT_NAME}_COMMON}
                            ${${PROJECT_NAME}_LMH}
                            ${${PROJECT_NAME}_LMHP}
                            ${${PROJECT_NAME}_SOURCES}
                            $<TARGET_OBJECTS:mac>
                            $<TARGET_OBJECTS:system>
                            $<TARGET_OBJECTS:radio>
                            $<TARGET_OBJECTS:peripherals>
                            $<TARGET_OBJECTS:${BOARD}>
)

target_compile_definitions(${PROJECT_NAME}-${SUB_PROJECT} PRIVATE $<$<BOOL:${CLASSB_ENABLED}>:LORAMAC_CLASSB_ENABLED>)
target_compile_definitions(${PROJECT_NAME}-${SUB_PROJECT} PRIVATE ACTIVE_REGION=${ACTIVE_REGION})

target_compile_definitions(${PROJECT_NAME}-${SUB_PROJECT}  PUBLIC
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:mac,INTERFACE_COMPILE_DEFINITIONS>>
)

target_include_directories(${PROJECT_NAME}-${SUB_PROJECT} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/common
    ${CMAKE_CURRENT_SOURCE_DIR}/common/LmHandler
    ${CMAKE_CURRENT_SOURCE_DIR}/common/LmHandler/packages
    ${CMAKE_CURRENT_SOURCE_DIR}/${SUB_PROJECT}/${BOARD}
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:mac,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:system,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:radio,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:peripherals,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:${BOARD},INTERFACE_INCLUDE_DIRECTORIES>>
)

set_property(TARGET ${PROJECT_NAME}-${SUB_PROJECT} PROPERTY C_STANDARD 11)

target_link_libraries(${PROJECT_NAME}-${SUB_PROJECT} m)

#---------------------------------------------------------------------------------------
# Debugging and Binutils
#---------------------------------------------------------------------------------------

include(gdb-helper)
include(binutils-arm-none-eabi)

# Generate debugger configurations
generate_run_gdb_stlink(${PROJECT_NAME}-${SUB_PROJECT})
generate_run_gdb_openocd(${PROJECT_NAME}-${SUB_PROJECT})
generate_vscode_launch_openocd(${PROJECT_NAME}-${SUB_PROJECT})

# Print section sizes of target
print_section_sizes(${PROJECT_NAME}-${SUB_PROJECT})

# Create output in hex and binary format
create_bin_output(${PROJECT_NAME}-${SUB_PROJECT})
create_hex_output(${PROJECT_NAME}-${SUB_PROJECT})
