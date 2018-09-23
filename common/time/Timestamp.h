//
// Created by skyitachi on 2018/9/19.
//

#ifndef LEARNUV_TIMESTAMP_H
#define LEARNUV_TIMESTAMP_H


#include <cstdint>
#include <string>

namespace util {
  
  class Timestamp {

  public:
    static const int kMircoSecondsPerSecond = 1000 * 1000;

    Timestamp(int64_t microSecondsSinceEpoch) : microSecondsSinceEpoch_(microSecondsSinceEpoch) {}
    
    Timestamp() : microSecondsSinceEpoch_(0) {}
    int64_t microSecondsSinceEpoch() {
      return microSecondsSinceEpoch_;
    }
    std::string toString() const;

    static Timestamp now();
  
  
  private:
    int64_t microSecondsSinceEpoch_;
  };
  
}
#endif //LEARNUV_TIMESTAMP_H
