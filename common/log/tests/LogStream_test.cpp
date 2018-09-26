//
// Created by skyitachi on 2018/9/11.
//

#include <iostream>
#include <cstring>
#include "log/Logging.h"
#include "log/LogStream.h"
#include "time/Timestamp.h"

using namespace util;

int main() {

  FixedBuffer<kSmallBuffer> sBuffer;
  const char *data = "hello world";
  sBuffer.append(data, strlen(data));
  std::cout << sBuffer.toString() << std::endl;

  LOG_INFO << "Hello world " << "one line log";

  LOG_INFO << "new logger";

  LOG_INFO << "Hello world "  << 100;

  LOG_INFO << "Hello world "  << 100.1;
  
  LOG_ERROR << "Error Log test";

}



