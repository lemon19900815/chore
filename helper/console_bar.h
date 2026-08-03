#pragma once

#include <cstdint>
#include <string>
#include <utility>
#include <iostream>

// 控制台进度条
class ConsoleBar final {
public:
    // 进度条长度；同时充当百分比刻度，故固定为 100
    static constexpr int32_t kBarLen = 100;

    explicit ConsoleBar(std::string name, int32_t total)
        : total_(total), last_(-1), name_(std::move(name)) {
    }
    ~ConsoleBar() = default;

    // 刷新进度；count 超出 [0, total] 时按边界截断
    void Process(int32_t count) {
        if (total_ <= 0)
            return;

        if (count < 0)
            count = 0;
        else if (count > total_)
            count = total_;

        // 中间结果用 64 位，避免 count * kBarLen 溢出 int32
        auto bar = static_cast<int32_t>(
            static_cast<int64_t>(count) * kBarLen / total_);
        if (last_ == bar)
            return;

        last_ = bar;
        std::cout << "\r" << name_ << "["
            << std::string(last_, '=')
            << std::string(kBarLen - last_, ' ')
            << "] " << last_ << "%...";

        // 进度到顶时换行收尾（endl 自带 flush）
        if (last_ == kBarLen)
            std::cout << std::endl;
        else
            std::cout.flush();
    }

private:
    int32_t total_;
    int32_t last_;
    std::string name_;
};

