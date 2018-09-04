//
// Created by skyitachi on 2018/9/4.
//

#ifndef LEARNUV_CONNECTION_H
#define LEARNUV_CONNECTION_H
#include <memory>
#include <functional>
#include <uv.h>
#include "Codec.h"

using namespace std::placeholders;

class Connection {
public:
  Connection(int id, uv_tcp_t* handle): id_(id),  handle_(handle) {}
  void setMessageCallback(MessageCallback cb) {
    codec_.setMessageCallBack(std::bind(&Connection::onMessage, this, _1));
  }
  void onMessage(const char *buf) {
    printf("conn.id %d receive message: %s\n", id_, buf);
  }
  Codec codec_;

  ~Connection(){
    printf("in the connection destructor\n");
  }

private:
  int id_;
  uv_tcp_t* handle_;
};


#endif //LEARNUV_CONNECTION_H
