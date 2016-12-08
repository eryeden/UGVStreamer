cmake_minimum_required(VERSION 2.8)

set(BN "Streamer")

file(GLOB Main_SRCS ./*.cpp)

include_directories(.)

set(SOURCE_FILES ${Main_SRCS})
add_executable(${BN} ${SOURCE_FILES})

target_link_libraries(${BN} ${LINK_LIBS})