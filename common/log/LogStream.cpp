//
// Created by skyitachi on 2018/9/11.
//

#include "LogStream.h"


namespace util {

  const char digits[] = "9876543210123456789";
  const char *zero = digits + 9;

  // TODO: int to string
  template <typename T>
  size_t convert(char buf[], T value) {
    T i = value;
    char *p = buf;
    do {
      int lsd = static_cast<int> (i % 10);
      i /= 10;
      *p++ = zero[lsd];
    } while (i != 0);
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
  }

  template <typename T>
  void LogStream::formatInteger(T v) {

    if (buffer_.avail() >= kMaxNumericSize) {
      size_t len = convert(buffer_.current(), v);
      buffer_.add(len);
    }
  }

  LogStream& LogStream::operator<<(int v) {
    formatInteger(v);
    return *this;
  }

  LogStream& LogStream::operator<<(double v) {
    if (buffer_.avail() >= kMaxNumericSize) {
      int len = snprintf(buffer_.current(), kMaxNumericSize, "%.12g", v);
      buffer_.add(len);
    }
    return *this;
  }
}
