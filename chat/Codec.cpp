//
// Created by skyitachi on 2018/9/4.
//

#include <util.h>
#include "Codec.h"

void Codec::parse(ssize_t nread, const uv_buf_t *b, ssize_t msg_offset) {
  ssize_t nleft = nread;
  while(nleft > 0) {
    if (nleft > 4) {
      length = (b->base[0 + msg_offset] << 24) + (b->base[1+msg_offset] << 16) + (b->base[2 + msg_offset] << 8) + b->base[3+msg_offset];
      printf("parsed length: %d\n", length);
      status = BODY;
      offset = 0;
    } else {
      status = HEAD;
      offset = nleft;
      memcpy(buf, b->base + msg_offset, nleft);
      break;
    }
    if (nleft >= length + 4) {
      status = BODY;
      memcpy(buf, b->base + msg_offset + 4, length);
      buf[length] = 0;
      cb_(buf);
      nleft -= length + 4;
      msg_offset += length + 4;
    } else {
      memcpy(buf, b->base + msg_offset + 4, nleft - 4);
      status = BODY;
      offset = nleft - 4;
      break;
    }
  }
}

void Codec::handleRead(ssize_t nread, const uv_buf_t* b) {
  if (nread < 0) {
    if (nread == UV_EOF) {
      printf("client closed\n");
      return;
    }
    log_error("handleRead error: ", nread);
    return;
  }
  printf("in the handleRead, read size: %zd, offset = %d, status=%d\n", nread, offset, status);
  if (nread == 0) {
    // TODO: connection callback
    printf("client closed\n");
    return;
  }
  if (status == INITIAL) {
    if (nread <= 4) {
      status = HEAD;
      offset += nread;
      memcpy(buf, b->base, nread);
    } else if (nread > 4) {
      parse(nread, b, 0);
    }
  } else if (status == HEAD) {
    if (nread + offset < 4) {
      memcpy(buf + offset, b->base, nread);
      offset += nread;
    } else {
      // 计算长度
      memcpy(buf + offset, b->base, 4 - offset);
      length = (buf[0] << 24) + (buf[1] << 16) + (buf[2] << 8) + buf[3];
      printf("handleRead parsed length: %d\n", length);
      status = BODY;
      ssize_t nleft = nread - (4 - offset);
      if (nleft < length) {
        memcpy(buf, b->base + (4 - offset), nleft);
        offset = nleft;
      } else {
        memcpy(buf, b->base + (4 - offset), length);
        buf[length] = 0;
        cb_(buf);
        offset = 0;
        parse(nleft - length, b, 4 - offset + length);
      }
    }
  } else if (status == BODY) {
    if (nread + offset < length) {
      memcpy(buf + offset, b->base, nread);
      offset += nread;
    } else {
      memcpy(buf+offset, b->base, length - offset);
      buf[length] = 0;
      cb_(buf);
      parse(nread - (length - offset), b, length - offset);
    }
  }
}

void Codec::onMessage(const Buffer *buffer) {
  while(buffer->readableBytes() > kHeaderLen) {
  
  }
}