//
// Created by skyitachi on 2018/9/7.
//

#include "Buffer.h"

uint32_t Buffer::peekUInt32() {
  assert(readableBytes() >= sizeof(int32_t));
  uint32_t ret = 0;
  memcpy(&ret, peek(), sizeof(uint32_t));
  return ntohl(ret);
}

uint32_t Buffer::readUInt32() {
  uint32_t ret = peekUInt32();
  retrieveUInt32();
  return ret;
}

