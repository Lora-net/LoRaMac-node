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

# Allow switching of device class
set(CLASS_LIST classA classB classC)
set(CLASS classA CACHE STRING "Default device is Class A")
set_property(CACHE CLASS PROPERTY STRINGS ${CLASS_LIST})

# Allow switching of active region
set(ACTIVE_REGION_LIST   LORAMAC_REGION_EU868 LORAMAC_REGION_US915 LORAMAC_REGION_CN779
    LORAMAC_REGION_EU433 LORAMAC_REGION_AU915 LORAMAC_REGION_AS923 LORAMAC_REGION_CN470
    LORAMAC_REGION_KR920 LORAMAC_REGION_IN865 LORAMAC_REGION_US915_HYBRID
)
set(ACTIVE_REGION LORAMAC_REGION_EU868 CACHE STRING "Default active region is EU868")
set_property(CACHE ACTIVE_REGION PROPERTY STRINGS ${ACTIVE_REGION_LIST})

#---------------------------------------------------------------------------------------
# Target
#---------------------------------------------------------------------------------------

file(GLOB ${PROJECT_NAME}_SOURCES "${CMAKE_CURRENT_LIST_DIR}/${CLASS}/${BOARD}/*.c")

add_executable(${PROJECT_NAME}-${CLASS}
                            ${${PROJECT_NAME}_SOURCES}
                            $<TARGET_OBJECTS:mac>
                            $<TARGET_OBJECTS:system>
                            $<TARGET_OBJECTS:radio>
                            $<TARGET_OBJECTS:peripherals>
                            $<TARGET_OBJECTS:${BOARD}>
)

target_compile_definitions(${PROJECT_NAME}-${CLASS} PRIVATE ACTIVE_REGION=${ACTIVE_REGION})

target_compile_definitions(${PROJECT_NAME}-${CLASS}  PUBLIC
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:mac,INTERFACE_COMPILE_DEFINITIONS>>
)

target_include_directories(${PROJECT_NAME}-${CLASS} PUBLIC
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:mac,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:system,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:radio,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:peripherals,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:${BOARD},INTERFACE_INCLUDE_DIRECTORIES>>
)

set_property(TARGET ${PROJECT_NAME}-${CLASS} PROPERTY C_STANDARD 11)

target_link_libraries(${PROJECT_NAME}-${CLASS} m)

#---------------------------------------------------------------------------------------
# Debugging and Binutils
#---------------------------------------------------------------------------------------

include(gdb-helper)
include(binutils-arm-none-eabi)

# Generate debugger configurations
generate_run_gdb_stlink(${PROJECT_NAME}-${CLASS})
generate_run_gdb_openocd(${PROJECT_NAME}-${CLASS})
generate_vscode_launch_openocd(${PROJECT_NAME}-${CLASS})

# Print section sizes of target
print_section_sizes(${PROJECT_NAME}-${CLASS})

# Create output in hex and binary format
create_bin_output(${PROJECT_NAME}-${CLASS})
create_hex_output(${PROJECT_NAME}-${CLASS})
