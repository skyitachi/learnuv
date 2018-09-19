//
// Created by skyitachi on 2018/9/19.
//

#include "Timestamp.h"
#include <inttypes.h>
#include <sys/time.h>
namespace util {
  
  std::string Timestamp::toString() const {
    char buf[32] = {0};
    int64_t seconds = microSecondsSinceEpoch_ / kMircoSecondsPerSecond;
    int64_t microseconds = microSecondsSinceEpoch_ % kMircoSecondsPerSecond;
    snprintf(buf, sizeof(buf) - 1, "%" PRId64 ".%06" PRId64 "", seconds, microseconds);
    return std::string(buf);
  }
  
  Timestamp Timestamp::now() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return Timestamp(seconds * kMircoSecondsPerSecond + tv.tv_usec);
  }
}