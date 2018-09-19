//
// Created by skyitachi on 2018/9/12.
//

#include "Logging.h"

#include <cstdio>
#include <cstring>
#include <sstream>



namespace util {

  Logger::LogLevel initLogLevel() {
    if (::getenv("MUDUO_LOG_TRACE"))
      return Logger::TRACE;
    else if (::getenv("MUDUO_LOG_DEBUG"))
      return Logger::DEBUG;
    else
      return Logger::INFO;
  }

  Logger::LogLevel g_logLevel = initLogLevel();


  const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
  {
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
  };

  // TODO: why?
  // helper class for known string length at compile time
  class T
  {
  public:
    T(const char* str, unsigned len)
      :str_(str),
       len_(len)
    {
      assert(strlen(str) == len_);
    }

    const char* str_;
    const unsigned len_;
  };

  inline LogStream& operator << (LogStream &s, T v) {
    s.append(v.str_, v.len_);
    return s;
  }

  inline LogStream& operator << (LogStream &s, const Logger::SourceFile & v) {
    s.append(v.data_, v.size_);
    return s;
  }

  void defaultOutput(const char *msg, int len) {
    // TODO
    size_t n = fwrite(msg, 1, len, stdout);
  }

  void defaultFlush() {
    fflush(stdout);
  }

  Logger::OutputFunc g_output = defaultOutput;

  Logger::FlushFunc g_flush = defaultFlush;

  Logger::Impl::Impl(util::Logger::LogLevel level, int old_errno, const util::Logger::SourceFile &file, int line)
    : stream_(),
      level_(level),
      line_(line),
      basename_(file)
  {
    stream_ << T(LogLevelName[level], 6);
  }

  void Logger::Impl::finish() {
    stream_ << " - " << basename_ << ":" << "prefix" << "\n";
  }

  Logger::Logger(util::Logger::SourceFile file, int line): impl_(INFO, 0, file, line) {
  }
  Logger::Logger(util::Logger::SourceFile file, int line, LogLevel level): impl_(level, 0, file, line) {}

  Logger::~Logger() {
    impl_.finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());
  }
}
