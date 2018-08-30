cmake_minimum_required(VERSION 3.7)
include_directories(../common)
set(COMMON_SRC ../common/util.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(chargen ${COMMON_SRC} chargen.cpp)

add_executable(fs ${COMMON_SRC} filetransfer.cpp)

target_link_libraries(chargen uv pthread)

target_link_libraries(fs uv)

