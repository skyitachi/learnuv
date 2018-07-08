cmake_minimum_required(VERSION 3.7)
include_directories(../common)
set(COMMON_SRC ../common/util.cpp)

add_executable(server ${COMMON_SRC} server.cc)
add_executable(client ${COMMON_SRC} client.cc)

target_link_libraries(server uv)
target_link_libraries(client uv)

