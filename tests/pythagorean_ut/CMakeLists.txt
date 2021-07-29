cmake_minimum_required(VERSION 3.7)

include_directories(${CPPUTEST_SRC_DIR})
include_directories(${CPPUTESTEXT_SRC_DIR})
include_directories(${PROJECT_SOURCE_DIR}/src_copy)
include_directories(${PROJECT_SOURCE_DIR}/mocking)
include_directories(${PROJECT_SOURCE_DIR}/src/peripherals/)
include_directories(${PROJECT_SOURCE_DIR}/src/mac/)
include_directories(${PROJECT_SOURCE_DIR}/src/system/)


set(SOURCE_FILES
    main.cpp
    pythagorean_ut.cpp
    geofence_ut.cpp
    lorawan_credentials_ut.cpp
    ${PROJECT_SOURCE_DIR}/src_copy/pythagorean.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/geofence.c
    ${PROJECT_SOURCE_DIR}/src/peripherals/region_settings.c
    square_root_mock.cpp)

add_executable(pythagorean_ut ${SOURCE_FILES})
target_link_libraries(pythagorean_ut CppUTest CppUTestExt)

add_test(pythagorean_test pythagorean_ut)