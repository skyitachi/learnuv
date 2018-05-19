cmake_minimum_required(VERSION 3.7)
include_directories(../common)
set(COMMON_SRC ../common/util.cpp)
add_executable(echo_client ${COMMON_SRC} echo_client.cpp)

add_executable(echo_server echo_server.cpp)

target_link_libraries(echo_client uv)
target_link_libraries(echo_server uv)
