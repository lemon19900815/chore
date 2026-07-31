#ifndef __UTIL_INC_H__
#define __UTIL_INC_H__

#include "common.h"

namespace util
{
  class Time
  {
  public:
    Time() {
      time_ms_ = 0;
    }

    void update() {
      time_ms_ = ms();
    }

    // 当前系统的毫秒时间
    static uint64_t ms() {
      auto tp = std::chrono::system_clock::now();
      auto dur = tp.time_since_epoch();
      return std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    }

    // 当前系统的时间戳
    static uint64_t now() {
      return ms() / 1000;
    }

    // 指定时间的字符串描述
    static std::string asString(time_t unix_time) {
      auto ptm = std::localtime(&unix_time);

      char buf[256];
      snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d-%02d-%02d.%03d", ptm->tm_year + 1900,
        ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, (int)(ms() % 1000));

      return std::string(buf);
    }

    uint64_t current() {
      return time_ms_;
    }

  private:
    uint64_t time_ms_;
  };
}

#endif //  __UTIL_INC_H__
