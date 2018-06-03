cmake_minimum_required(VERSION 3.7)
include_directories(../common)

set(COMMON_SRC ../common/util.cpp)

add_executable(readline ${COMMON_SRC} libuv_readfile.cpp)

add_executable(read ${COMMON_SRC} system_read.cpp)

target_link_libraries(readline uv)

target_link_libraries(read uv)
