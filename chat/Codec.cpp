//
// Created by skyitachi on 2018/9/4.
//

#include <string>
#include "util.h"
#include "Codec.h"

void Codec::on_uv_read(const size_t nread, const uv_buf_t *buf) {
  buffer->append(buf->base, nread);
  onMessage(buffer);
}

void Codec::onMessage(Buffer *buffer) {
  while(buffer->readableBytes() > kHeaderLen) {
    int len = buffer->peekUInt32();
    if (len > kMaxMessageSize || len < 0) {
      fprintf(stderr, "message too long\n");
      break;
    } else if (len + kHeaderLen <= buffer->readableBytes()) {
      buffer->retrieveUInt32();
      std::string msg = buffer->retrieveAsString(len);
      cb_(msg);
    } else {
      break;
    }
  }
}

int Codec::encode(Buffer *buffer, const char *msg) {
  int len = strlen(msg);
  buffer->writeUInt32(len);
  buffer->append(msg, len);
  return len + kHeaderLen;
}
