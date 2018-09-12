//
// Created by skyitachi on 2018/9/11.
//

#ifndef LEARNUV_LOGSTREAM_H
#define LEARNUV_LOGSTREAM_H

#include <cstdio>
#include <cstring>
#include <string>

namespace util {
  const int kSmallBuffer = 4096;
  const int kLargeBuffer = 4000 * 1000;
  template<int SIZE>
  class FixedBuffer {
  public:
    FixedBuffer(): cur_(data_) {}
    FixedBuffer(const FixedBuffer &) = delete;

    void append(const char *buf, size_t len) {
      // Note: 溢出的情况?
      if (static_cast<size_t>(avail()) > len) {
        memcpy(cur_, buf, len);
        cur_ += len;
      }
    }

    const char *data() const { return data_; }
    char *current() { return cur_; }
    int length() const { return static_cast<int>(cur_ - data_); }
    int avail() const { return static_cast<int>(end() - cur_); }
    void add(size_t len) { cur_ += len; }
    void reset() { cur_ = data_; }
    void bzero() { ::bzero(data_, sizeof data_); }
    std::string toString() const { return std::string(data_, length()); }

  private:
    const char * end() const { return data_ + sizeof data_; }
    char data_[SIZE];
    char *cur_;
  };

  class LogStream {
    typedef LogStream self;
  public:
    typedef FixedBuffer<kSmallBuffer> Buffer;
    void append(const char* data, int len) {
      buffer_.append(data, static_cast<size_t>(len));
    }

    self& operator << (const char * str) {
      if (str) {
        buffer_.append(str, strlen(str));
      } else {
        buffer_.append("(null)", 6);
      }
      return *this;
    }

    const Buffer& buffer() const { return buffer_; }
  private:
    Buffer buffer_;
//    LogStream(const LogStream &) = delete;
  };
}


#endif //LEARNUV_LOGSTREAM_H
