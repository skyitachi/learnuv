cmake_minimum_required(VERSION 3.7)
include_directories(../common)

set(COMMON_SRC ../common/util.cpp)

add_executable(read_uv ${COMMON_SRC} libuv_readfile.cpp)

add_executable(read ${COMMON_SRC} system_read.cpp)

add_executable(read_uv_sync ${COMMON_SRC} libuv_readfile_sync.cpp)

target_link_libraries(read_uv uv)

target_link_libraries(read uv)

target_link_libraries(read_uv_sync uv)
