cmake_minimum_required(VERSION 3.7)
project(learnuv)

# set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.c common/buffer/Buffer.cpp common/buffer/Buffer.h common/log/LogStream.cpp)

add_executable(learnuv ${SOURCE_FILES})

add_subdirectory(socks)

add_subdirectory(echo)

add_subdirectory(fs)

add_subdirectory(pingpong)

add_subdirectory(simple)

add_subdirectory(chat)

add_subdirectory(common/log/tests)

