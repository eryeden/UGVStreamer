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


#SDL2 congfiguration
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

#ffmpeg configuration
set(FFMPEG_LIBS
        avcodec
        avresample
        avutil
        swscale
        )

#set(FFMPEG_LIBS
#        "-L/usr/local/Cellar/ffmpeg/HEAD/lib -lavcodec -lx264  -lavresample -lavutil"
#        "-L/usr/local/Cellar/ffmpeg/HEAD/lib -lswscale -lm -lavutil -lm"
#        )

set(LINK_LIBS ${SDL2_LIBRARIES} ${FFMPEG_LIBS})


add_subdirectory(Receiver)
























