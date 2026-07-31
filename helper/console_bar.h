#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <iostream>

// 进度条长度
#define BAR_LEN 100

// 控制台进度条
class ConsoleBar final {
public:
    explicit ConsoleBar(std::string name, int32_t total) {
        last_ = -1;
        total_ = total;
        name_ = name;
        memset(buf_, ' ', sizeof(buf_));
        buf_[BAR_LEN] = '\0';

        Process(0);
    }
    ~ConsoleBar() = default;

    void Process(int32_t count) {
        if (total_ <= 0)
            return;

        auto bar = count * BAR_LEN / total_;
        if (last_ != bar) {
            last_ = bar;
            memset(buf_, '=', last_);

            std::cout << "\r" << name_ <<
                "[" << buf_ << "] " << last_ << "%...";
            std::cout.flush();
        }

        if (last_ == BAR_LEN)
            std::cout << std::endl;
    }

private:
    int32_t total_;
    int32_t last_;
    std::string name_;
    char buf_[BAR_LEN + 1];
};

