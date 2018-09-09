//
// Created by skyitachi on 2018/9/7.
//

#ifndef LEARNUV_BUFFER_H
#define LEARNUV_BUFFER_H
#include <vector>
#include <assert.h>
#include <string>
#include <algorithm>

class Buffer{
public:
  static const size_t kInitialSize = 1024;
  
  explicit Buffer(size_t initialSize = kInitialSize)
    : buffer_(initialSize),
      readerIndex_(0),
      writerIndex_(0) {
  }
  
  void swap(Buffer &rhs) {
    buffer_.swap(rhs.buffer_);
    std::swap(readerIndex_, rhs.readerIndex_);
    std::swap(writerIndex_, rhs.writerIndex_);
  }
  
  size_t readableBytes() const {
    return writerIndex_ - readerIndex_;
  }
  
  size_t writeableBytes() const {
    return buffer_.size() - writerIndex_;
  }
  
  size_t prependableBytes() const {
    return readerIndex_;
  }
  
  void retrieve(size_t len) {
    assert(len <= readableBytes());
    if (len < readableBytes()) {
      readerIndex_ += len;
    } else {
      retrieveAll();
    }
  }
  
  void retrieveAll() {
    readerIndex_ = 0;
    writerIndex_ = 0;
  }
  
  const char *peek() const {
    return begin() + readerIndex_;
  }
  
  std::string retrieveAsString(size_t len) {
    assert(len <= readableBytes());
    std::string ret(peek(), len);
    retrieve(len);
    return ret;
  }
  
  // allocate first
  void append(const char *data, size_t len) {
    ensureWriableBytes(len);
    std::copy(data, data + len, beginWrite());
    writerIndex_ += len;
  }
  
  void ensureWriableBytes(size_t len) {
    if (writeableBytes() < len) {
      makeSpace(len);
    }
    assert(writeableBytes() >= len);
  }
  
  uint32_t readUInt32();
  uint32_t peekUInt32();
  void retrieveUInt32() {
    retrieve(sizeof(uint32_t));
  }

  void writeUInt32(uint32_t hn) {
    uint32_t nn = htonl(hn);
    append((const char *)&nn, sizeof(uint32_t));
  }

private:
  char *begin() {
    return &*buffer_.begin();
  }
  const char* begin() const {
    return &*buffer_.begin();
  }
  char *beginWrite() {
    return begin() + writerIndex_;
  }
  const char *beginWrite() const {
    return begin() + writerIndex_;
  }
  
  void makeSpace(size_t len) {
    if (writeableBytes() + prependableBytes() < len) {
      // Note: should move readable bytes to the front?
      buffer_.resize(writerIndex_ + len);
    } else {
      // move data to the front
      size_t readable = readableBytes();
      // 有可能异常?
      std::copy(begin() + readerIndex_, begin() + writerIndex_, begin());
      readerIndex_ = 0;
      writerIndex_ -= readerIndex_;
      assert(readable == readableBytes());
    }
  }
  
private:
  std::vector<char> buffer_;
  size_t readerIndex_;
  size_t writerIndex_;
};


#endif //LEARNUV_BUFFER_H
