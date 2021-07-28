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
project(loramac-node)
cmake_minimum_required(VERSION 3.6)




if(UNIT_TESTING) 
message(STATUS ">>> Compiling for Unittesting")

project(cpputest_example)

include(CTest)

add_subdirectory(mocking)
add_subdirectory(testtools)
add_subdirectory(tests)
add_subdirectory(src_copy)


else()
message(STATUS ">>> Compiling for target")

# stuff that should happen not on Linux 

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/src)

endif()