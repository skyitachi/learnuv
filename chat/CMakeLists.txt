include_directories(../common)
set(COMMON_SRC ../common/util.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(chat ${COMMON_SRC} chat.cpp connection.cpp Codec.cpp)

target_link_libraries(chat uv)