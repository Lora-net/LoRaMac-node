cmake_minimum_required(VERSION 3.7)

project(cpputest_example_pythagorean)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c pythagorean.c square_root.c)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable(cpputest_example_pythagorean ${SOURCE_FILES})

# link against math library.
target_link_libraries(cpputest_example_pythagorean m)
install(TARGETS cpputest_example_pythagorean DESTINATION bin)
