//
// Created by skyitachi on 2018/9/7.
//

#include "Buffer.h"

void Buffer::readInt32() {
  assert(readableBytes() >= sizeof(int32_t));
}
