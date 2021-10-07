cmake_minimum_required(VERSION 3.7)


# Get the test framework Cpputest
include(FetchContent)
FetchContent_Declare(
    cpputest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG v4.0
)
# Set this to ON if you want to have the CppUTests in your project as well.
set(TESTS OFF CACHE BOOL "Switch off CppUTest Test build")


set(CPPUTEST_CXX_FLAGS "${CPPUTEST_CXX_FLAGS}  -m32")
set(CPPUTEST_LD_FLAGS "${CPPUTEST_LD_FLAGS}  -m32")

FetchContent_MakeAvailable(cpputest)

add_definitions(
    -DREGION_EU868
    -DREGION_US915
    -DREGION_AU915
    -DREGION_CN470
    -DREGION_AS923
    -DREGION_KR920
    -DREGION_IN865
    -DREGION_RU864
    -DSOFT_SE -DSTM32L072xx -DUSE_HAL_DRIVER -DUNITTESTING_LORA)


add_definitions(
    -DREGION_AS923_DEFAULT_CHANNEL_PLAN=CHANNEL_PLAN_GROUP_AS923_1

)


include_directories(${CPPUTEST_SRC_DIR})
include_directories(${CPPUTESTEXT_SRC_DIR})

# Test includes
include_directories(${PROJECT_SOURCE_DIR}/mocking)
include_directories(${PROJECT_SOURCE_DIR}/tests/mocks/)


# Loramac original header files
include_directories(${PROJECT_SOURCE_DIR}/src/peripherals/)
include_directories(${PROJECT_SOURCE_DIR}/src/mac/)
include_directories(${PROJECT_SOURCE_DIR}/src/mac/region/)
include_directories(${PROJECT_SOURCE_DIR}/src/system/)
include_directories(${PROJECT_SOURCE_DIR}/src/boards/)
include_directories(${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/)
include_directories(${PROJECT_SOURCE_DIR}/src/peripherals/soft-se/)
include_directories(${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/LmHandler/)
include_directories(${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/LmHandler/packages/)
include_directories(${PROJECT_SOURCE_DIR}/src/radio/)
include_directories(${PROJECT_SOURCE_DIR}/src/boards/B-L072Z-LRWAN1/)
include_directories(${PROJECT_SOURCE_DIR}/src/boards/B-L072Z-LRWAN1/cmsis/)
include_directories(${PROJECT_SOURCE_DIR}/src/boards/mcu/stm32/STM32L0xx_HAL_Driver/Inc/)
include_directories(${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/periodic-uplink-lpp/B-L072Z-LRWAN1/)
include_directories(${PROJECT_SOURCE_DIR}/src/radio/sx1276/)




# Unittests
set(UNIT_TESTS 
    main.cpp
    geofence_ut.cpp
    playback_ut.cpp
    lorawan_credentials_ut.cpp
    NvmDataMgmt_ut.cpp
    app_ut.cpp
    bsp_ut.cpp
    print_utils_ut.cpp
    soft_se_ut.cpp
    LoRaWAN_config_switcher_ut.cpp
)

# Mocks
set(MOCKS 
    sx1276-board-mock.c
    cli-mock.c
    board-mock.c
    sx1276-mock.c
    timer-mock.cpp
    bsp_mock.cpp
    eeprom-board-mock.cpp
    # loramac-mock.cpp
    systime-mock.cpp
    ublox-mock.cpp
    nvm_images.cpp
    gps_mock_utils.cpp
    delay-board-mock.c
    gpio_mock.c
)

set(LZ4_LIB_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src/system/)


# CLI sources
set(LZ4_SOURCES
  "${LZ4_LIB_SOURCE_DIR}/lz4.c"
  "${LZ4_LIB_SOURCE_DIR}/lz4hc.c"
  "${LZ4_LIB_SOURCE_DIR}/lz4.h"
  "${LZ4_LIB_SOURCE_DIR}/lz4hc.h"
  "${LZ4_LIB_SOURCE_DIR}/lz4frame.c"
  "${LZ4_LIB_SOURCE_DIR}/lz4frame.h"
  "${LZ4_LIB_SOURCE_DIR}/xxhash.c")


# Put in compile files(all logic so no hardware connection)    
set(FILES_UNDER_TEST 
    ${PROJECT_SOURCE_DIR}/src/peripherals/config.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/geofence.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/playback.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/bsp.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/struct.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/print_utils.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/LoRaWAN_config_switcher.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/linear_congruential_generator.c
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/periodic-uplink-lpp/B-L072Z-LRWAN1/main.c
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/LmHandlerMsgDisplay.c
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/LmHandler/LmHandler.c
    ${PROJECT_SOURCE_DIR}/src/system/delay.c
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/callbacks.c

   

    # Compile region files
    ${PROJECT_SOURCE_DIR}/src/mac/region/RegionAU915.c
    ${PROJECT_SOURCE_DIR}/src/mac/region/RegionUS915.c
    ${PROJECT_SOURCE_DIR}/src/mac/region/RegionAS923.c
    ${PROJECT_SOURCE_DIR}/src/mac/region/RegionCN470.c
    ${PROJECT_SOURCE_DIR}/src/mac/region/RegionEU868.c
    ${PROJECT_SOURCE_DIR}/src/mac/region/RegionIN865.c
    ${PROJECT_SOURCE_DIR}/src/mac/region/RegionKR920.c
    ${PROJECT_SOURCE_DIR}/src/mac/region/RegionRU864.c
    ${PROJECT_SOURCE_DIR}/src/mac/region/RegionBaseUS.c
    ${PROJECT_SOURCE_DIR}/src/mac/region/RegionCommon.c
    ${PROJECT_SOURCE_DIR}/src/mac/region/Region.c


    ${PROJECT_SOURCE_DIR}/src/mac/LoRaMac.c
    ${PROJECT_SOURCE_DIR}/src/mac/LoRaMacAdr.c
    ${PROJECT_SOURCE_DIR}/src/mac/LoRaMacClassB.c
    ${PROJECT_SOURCE_DIR}/src/mac/LoRaMacCommands.c
    ${PROJECT_SOURCE_DIR}/src/mac/LoRaMacConfirmQueue.c
    ${PROJECT_SOURCE_DIR}/src/mac/LoRaMacCrypto.c
    ${PROJECT_SOURCE_DIR}/src/mac/LoRaMacParser.c
    ${PROJECT_SOURCE_DIR}/src/mac/LoRaMacSerializer.c 

    ${PROJECT_SOURCE_DIR}/src/peripherals/soft-se/soft-se.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/soft-se/cmac.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/soft-se/aes.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/soft-se/soft-se-hal.c

    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/CayenneLpp.c 
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/LmHandlerMsgDisplay.c 
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/NvmDataMgmt.c 
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/LmHandler/LmHandler.c 

    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/LmHandler/packages/FragDecoder.c 
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/LmHandler/packages/LmhpClockSync.c 
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/LmHandler/packages/LmhpCompliance.c 
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/LmHandler/packages/LmhpFragmentation.c 
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/LmHandler/packages/LmhpRemoteMcastSetup.c 

    ${PROJECT_SOURCE_DIR}/src/boards/mcu/utilities.c
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/NvmDataMgmt.c
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/region_nvm.c

    
    ${PROJECT_SOURCE_DIR}/src/system/nvmm.c

)

set(SOURCE_FILES
    ${UNIT_TESTS}
    ${MOCKS}
    ${FILES_UNDER_TEST}
    ${LZ4_SOURCES}
)

add_executable(picotracker_lora_tests ${SOURCE_FILES})
target_compile_options(picotracker_lora_tests PRIVATE -fshort-enums)
# target_compile_options(picotracker_lora_tests PRIVATE -Wall -Wextra -Wpedantic)
target_link_libraries(picotracker_lora_tests PRIVATE CppUTest CppUTestExt)


# only build 32-bit binaries
target_compile_options(picotracker_lora_tests PRIVATE -m32)
target_link_options(picotracker_lora_tests PRIVATE -m32)

# Add tests
add_test(picotracker_lora_tests picotracker_lora_tests)
