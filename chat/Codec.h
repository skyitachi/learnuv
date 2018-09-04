//
// Created by skyitachi on 2018/9/4.
//

#ifndef LEARNUV_CODEC_H
#define LEARNUV_CODEC_H

#define CODEC_BUF 8192

#include <uv.h>

#include "Callbacks.h"

class Codec{
public:
  enum Status {
    INITIAL,
    HEAD,
    BODY
  };
  Codec() {
    offset = 0;
    status = INITIAL;
    length = 0;
  }
  void handleRead(ssize_t nread, const uv_buf_t* b);
  void setMessageCallBack(MessageCallback cb) {
    cb_ = cb;
  }
private:
  char buf[CODEC_BUF];
  int offset;
  Status status;
  int length;
  MessageCallback cb_;
};


#endif //LEARNUV_CODEC_H
