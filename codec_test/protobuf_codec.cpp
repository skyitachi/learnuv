//
// Created by skyitachi on 2018/9/24.
//
#include "protobuf_codec.h"
#include "query.pb.h"

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <zlib.h>

#include <string>
#include <stdint.h>

int main() {
  learnuv::Query query;
  query.set_id(1);
}

