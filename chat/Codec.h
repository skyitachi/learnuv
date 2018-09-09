//
// Created by skyitachi on 2018/9/4.
//

#ifndef LEARNUV_CODEC_H
#define LEARNUV_CODEC_H

#define CODEC_BUF 8192

#include <uv.h>
#include "buffer/Buffer.h"
#include "Callbacks.h"

class Codec{
public:
  const int kHeaderLen = 4;
  const int kMaxMessageSize = 4096;
  Codec() {
    buffer = new Buffer(8192);
  }
  void setMessageCallback(MessageCallback cb) {
    cb_ = cb;
  }
  void onMessage(Buffer* buffer);
  int encode(Buffer* buffer, const char *msg);
  void on_uv_read(const size_t nread, const uv_buf_t* buf);
  Buffer* buffer;
private:
  MessageCallback cb_;
};


#endif //LEARNUV_CODEC_H
