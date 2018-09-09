//
// Created by skyitachi on 2018/9/4.
//

#include "connection.h"

void Connection::on_uv_read(const size_t nread, const uv_buf_t *buf) {
  buffer->append(buf->base, nread);
  codec_.onMessage(buffer);
}
