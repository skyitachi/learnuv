//
// Created by skyitachi on 2018/9/24.
//

#ifndef LEARNUV_CODEC_CODEC_H
#define LEARNUV_CODEC_CODEC_H

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <zlib.h>

#include <arpa/inet.h>
#include <string>
#include <stdint.h>
#include <sys/_endian.h>

const int kHeaderLen = sizeof(int32_t);

inline std::string encode(const google::protobuf::Message &message) {
  std::string result;
  result.resize(kHeaderLen);
  const std::string& typeName = message.GetTypeName();
  int32_t nameLen = static_cast<int32_t>(typeName.size() + 1);
  int32_t be32 = htonl(nameLen);
  result.append(reinterpret_cast<char *>(&be32), sizeof(be32));
  result.append(typeName.c_str(), nameLen);
  bool succeed = message.AppendToString(&result);
  if (succeed) {
    const char *begin = result.c_str() + kHeaderLen;
    int32_t checkSum = adler32(1, reinterpret_cast<const Bytef *>(begin), result.size() - kHeaderLen);
    int32_t be32 = htonl(checkSum);
    result.append(reinterpret_cast<char *>(&be32), sizeof(be32));
    int32_t len = htonl(result.size() - kHeaderLen);
    std::copy(reinterpret_cast<char *>(&len),
      reinterpret_cast<char *>(&len) + sizeof(len), result.begin()) ;
  } else {
    result.clear();
  }
  return result;
}

inline google::protobuf::Message* createMessage(const std::string &type_name) {
  google::protobuf::Message *message = NULL;
  const google::protobuf::Descriptor* descriptor =
    google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(type_name);
  if (descriptor) {
    const google::protobuf::Message *prototype =
      google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
    if (prototype) {
      message = prototype->New();
    }
  }
  return message;
}

#endif //LEARNUV_CODEC_CODEC_H
