//
// Created by skyitachi on 2018/9/12.
//

#ifndef LEARNUV_LOGGING_H
#define LEARNUV_LOGGING_H

#include "LogStream.h"
#include <cstring>

namespace util {
  class Logger {
  public:
    enum LogLevel {
      TRACE,
      DEBUG,
      INFO,
      WARN,
      ERROR,
      FATAL,
      NUM_LOG_LEVELS
    };

    //
    class SourceFile {
    public:
      template <int N>
      inline SourceFile(const char (&arr)[N]): data_(arr), size_(N-1) {
        // TODO: why
        const char *slash = strrchr(data_, '/');
        if (slash) {
          data_ = slash + 1;
          size_ -= static_cast<int>(data_ - arr);
        }
      }

      explicit SourceFile(const char *filename): data_(filename) {
        const char *slash = strrchr(data_, '/');
        if (slash) {
          data_ = slash + 1;
        }
        size_ -= static_cast<int>(strlen(data_));
      }

      const char* data_;
      int size_;
    };

    Logger(SourceFile file, int line);
    ~Logger();
    LogStream& stream() { return impl_.stream_; };
    static LogLevel logLevel();
    static void setLogLevel(LogLevel level);

    typedef void (*OutputFunc)(const char *msg, int len);
    typedef void (*FlushFunc)();

    static void setOutput(OutputFunc);
    static void setFlush(FlushFunc);

  private:
    class Impl {
    public:
      typedef Logger::LogLevel LogLevel;
      Impl(LogLevel level, int old_errno, const SourceFile& file, int line);
      void formatTime();
      void finish();

      LogStream stream_;
      LogLevel level_;
      int line_;
      SourceFile basename_;
    };
    Impl impl_;
  };


extern Logger::LogLevel g_logLevel;
inline Logger::LogLevel Logger::logLevel() {
  return g_logLevel;
}

#define LOG_INFO if (util::Logger::logLevel() <= util::Logger::INFO) \
  util::Logger(__FILE__, __LINE__).stream()

}


#endif //LEARNUV_LOGGING_H
