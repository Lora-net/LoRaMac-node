cmake_minimum_required(VERSION 3.7)


# Get the test framework Cpputest
include(FetchContent)
FetchContent_Declare(
    cpputest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG v4.0
)

FetchContent_MakeAvailable(cpputest)


# Add in the include directories
include_directories(${CPPUTEST_SRC_DIR})
include_directories(${CPPUTESTEXT_SRC_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src_copy)
include_directories(${PROJECT_SOURCE_DIR}/mocking)
include_directories(${PROJECT_SOURCE_DIR}/src/peripherals/)
include_directories(${PROJECT_SOURCE_DIR}/src/mac/)
include_directories(${PROJECT_SOURCE_DIR}/src/mac/region/)
include_directories(${PROJECT_SOURCE_DIR}/src/system/)
include_directories(${PROJECT_SOURCE_DIR}/src/boards/)
include_directories(${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/)
include_directories(${PROJECT_SOURCE_DIR}/src/radio/)



# Unittests
set(UNIT_TESTS 
    main.cpp
    pythagorean_ut.cpp
    geofence_ut.cpp
    playback_ut.cpp
    lorawan_credentials_ut.cpp
    NvmDataMgmt_ut.cpp
    bsp_ut.cpp
    print_utils_ut.cpp
)

# Mocks
set(MOCKS 
    bsp_mock.cpp
    square_root_mock.cpp
    eeprom-board-mock.cpp
    loramac-mock.cpp
    systime-mock.cpp
    nvmm-mock.c
)

set(FILES_UNDER_TEST 
    ${PROJECT_SOURCE_DIR}/src_copy/pythagorean.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/geofence.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/region_settings.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/playback.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/bsp.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/print_utils.c
    ${PROJECT_SOURCE_DIR}/src/boards/mcu/utilities.c
    ${PROJECT_SOURCE_DIR}/src/apps/LoRaMac/common/NvmDataMgmt.c
)
set(SOURCE_FILES
    ${UNIT_TESTS}
    ${MOCKS}
    ${FILES_UNDER_TEST}
)

add_executable(pythagorean_ut ${SOURCE_FILES})
target_link_libraries(pythagorean_ut CppUTest CppUTestExt)
add_test(pythagorean_test pythagorean_ut)