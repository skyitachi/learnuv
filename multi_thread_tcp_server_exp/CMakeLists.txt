set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_executable(multi_thread_server multi_thread_server.cpp)
target_link_libraries(multi_thread_server uv)