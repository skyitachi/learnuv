include_directories(../common)
set(COMMON_SRC ../common/util.cpp ../common/buffer/Buffer.cpp ../common/time/Timestamp.cpp ../common/log/Logging.cpp ../common/log/LogStream.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(chat_server ${COMMON_SRC} ChatServer.cpp connection.cpp Codec.cpp)

target_link_libraries(chat_server uv)

add_executable(chat_client ${COMMON_SRC} Codec.cpp ChatClient.cpp)

target_link_libraries(chat_client uv)
