include_directories(../common)

set(COMMON_SRC ../common/util.cpp ../common/buffer/Buffer.cpp ../common/time/Timestamp.cpp ../common/log/Logging.cpp ../common/log/LogStream.cpp)

add_executable(d_test ${COMMON_SRC} descriptor_test.cpp query.pb.cc)

target_link_libraries(d_test protobuf uv)

add_executable(c_test ${COMMON_SRC} protobuf_codec.cpp query.pb.cc)

target_link_libraries(c_test protobuf uv zlib)

