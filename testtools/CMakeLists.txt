cmake_minimum_required(VERSION 3.7)



include(FetchContent)
FetchContent_Declare(
    cpputest
    GIT_REPOSITORY https://github.com/cpputest/cpputest.git
    GIT_TAG v4.0
)

FetchContent_MakeAvailable(cpputest)
