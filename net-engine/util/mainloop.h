#ifndef __MAIN_LOOP_INC_H__
#define __MAIN_LOOP_INC_H__

#include "timehelp.h"

#include <functional>
#include <cassert>
#include <algorithm>

extern util::Time gtime;

namespace util
{
  // 默认为10ms
  constexpr uint64_t kDefaultFrameTime = 10;

  // 主循环控制：控制执行间隔的时间
  class MainLoop
  {
  public:
    MainLoop() {
      frame_count_ = 0;
      last_ms_ = 0;
      schema_ = nullptr;
      frame_time_ = kDefaultFrameTime;
    }

    void registerSchema(uint64_t frame_time, std::function<void()> schema) {
      assert(schema);
      schema_ = schema;

      // 每帧调度时间
      frame_time_ = std::max<uint64_t>(frame_time, kDefaultFrameTime);
    }

    void update() {
      auto elapsed = gtime.current() - last_ms_;
      if (elapsed >= frame_time_)
      {
        if (schema_) schema_();

        ++frame_count_;
        last_ms_ = gtime.current();
      }
    }

    uint64_t frame_count() {
      return frame_count_;
    }

  private:
    // 每帧运行间隔时间
    uint64_t frame_time_;
    // 主循环控制器
    std::function<void()> schema_;

    // 服务器运行的帧数
    uint64_t frame_count_;
    // 上次记录的时间
    uint64_t last_ms_;
  };
}

#endif // !__MAIN_LOOP_INC_H__
