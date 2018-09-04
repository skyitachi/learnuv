//
// Created by skyitachi on 2018/9/4.
//

#include "Codec.h"

void Codec::handleRead(ssize_t nread, const uv_buf_t* b) {
  printf("in the handleRead\n");
  if (status == INITIAL) {
    if (nread < 4) {
      status = HEAD;
      offset += nread;
      memcpy(buf, b->base, nread);
    } else if (nread >= 4) {
      status = BODY;
      length = (b->base[0] << 24) + (b->base[1] << 16) + (b->base[2] << 8) + b->base[3];
      if (length + 4 == nread) { //
        if (length > CODEC_BUF) {
          fprintf(stderr, "message length > CODEC_BUF");
          return;
        }
        memcpy(buf, b->base, nread);
        cb_(buf);
      }
    }
  }
}