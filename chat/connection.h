//
// Created by skyitachi on 2018/9/4.
//

#ifndef LEARNUV_CONNECTION_H
#define LEARNUV_CONNECTION_H
#include <memory>
#include <functional>
#include <uv.h>
#include "Codec.h"

#include "util.h"
#include "buffer/Buffer.h"

using namespace std::placeholders;

class Connection {
public:
  Connection(int id, uv_tcp_t* handle): id_(id),  handle_(handle) {}

  void send(const char* buf, size_t len) {
    uv_write_t* req = (uv_write_t *)safe_malloc(sizeof(uv_write_t));
    memcpy(sendBuf, buf, len);
    uv_buf_t ub = uv_buf_init(sendBuf, len);
    uv_write(req, (uv_stream_t*)handle_, &ub, 1, common_on_write_end);
  }

  ~Connection(){
    uv_close((uv_handle_t *)handle_, NULL);
    printf("in the connection destructor\n");
  }

  int id() {
    return id_;
  }

private:
  int id_;
  uv_tcp_t* handle_;
  char sendBuf[8192];
};


#endif //LEARNUV_CONNECTION_H
