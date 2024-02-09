cmake_minimum_required(VERSION 3.23)

cmake_path(SET LORAMAC_SOURCE_DIR NORMALIZE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

add_subdirectory(board)
add_subdirectory(system)
add_subdirectory(radio)
add_subdirectory(peripheral)
add_subdirectory(mac)
