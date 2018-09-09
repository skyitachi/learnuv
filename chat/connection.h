//
// Created by skyitachi on 2018/9/4.
//

#ifndef LEARNUV_CONNECTION_H
#define LEARNUV_CONNECTION_H
#include <memory>
#include <functional>
#include <uv.h>
#include "Codec.h"
#include "buffer/Buffer.h"

using namespace std::placeholders;

class Connection {
public:
  Connection(int id, uv_tcp_t* handle): id_(id),  handle_(handle) {
    buffer = new Buffer();
  }
  void setMessageCallback() {
    codec_.setMessageCallBack(std::bind(&Connection::onMessage, this, _1));
  }
  void onMessage(const char *buf) {
    printf("conn.id %d receive message: %s\n", id_, buf);
  }
  void on_uv_read(const size_t nread, const uv_buf_t* buf);
  Codec codec_;
  Buffer* buffer;

  ~Connection(){
    printf("in the connection destructor\n");
  }

private:
  int id_;
  uv_tcp_t* handle_;
};


#endif //LEARNUV_CONNECTION_H
