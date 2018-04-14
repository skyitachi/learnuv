cmake_minimum_required(VERSION 3.7)
project(learnuv)

# set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.c)

add_executable(learnuv ${SOURCE_FILES})

add_subdirectory(socks)

TARGET_LINK_LIBRARIES(learnuv uv)
