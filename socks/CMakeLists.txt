cmake_minimum_required(VERSION 3.7)
add_executable(socks_proxy socks_server.cpp)
target_link_libraries(socks_proxy uv)