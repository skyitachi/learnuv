include_directories(../)
set(CMAKE_CXX_STANDARD 11)
set(SRC ../LogStream.cpp ../Logging.cpp)

add_executable(logstream_test ${SRC} LogStream_test.cpp)

