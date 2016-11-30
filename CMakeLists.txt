#For Stand Alone

cmake_minimum_required(VERSION 2.8)
project(SDLtest)

#For auto-complete
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#output make commands
set(CMAKE_VERBOSE_MAKEFILE ON)

#COMPILE COMMANDS configuration
if (APPLE)
    set(CMAKE_C_COMPILER /usr/local/bin/gcc-6)
    set(CMAKE_CXX_COMPILER /usr/local/bin/g++-6)
endif ()

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

#source files
file(GLOB SRCS ./*.cpp)

add_executable(sdl_test ${SRCS})
target_link_libraries(sdl_test ${SDL2_LIBRARIES})




















