//
// Created by skyitachi on 2018/9/19.
//

#ifndef LEARNUV_TIMEZONE_H
#define LEARNUV_TIMEZONE_H

#include <ctime>

namespace util {
  class TimeZone {
    
    TimeZone() {}
    TimeZone(int eastOfUTC, const char *tzname);
    explicit TimeZone(const char *zonefile);
    
    struct tm toLocalTime(time_t secondsSinceEpoch) const;
    time_t fromLocaltTime(const struct tm& ) const;
    
    static struct tm toUtcTime(time_t secondsSinceEpoch, bool yday = false);
    
    static time_t fromUtcTime(const struct tm&);
    
    static time_t fromUtcTime(int year, int month, int day, int hour, int minute, int seconds);
  };
}


#endif //LEARNUV_TIMEZONE_H
