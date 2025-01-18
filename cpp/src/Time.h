#pragma once

#include <gtb/DataModel.h>

#include <stdint.h>

namespace gtb
{

class Time : public DataModel
{
    public:
        Time();
        Time(Time &&) = default;
        Time(const Time &) = delete;
        Time &operator=(Time &&) = delete;
        Time &operator=(const Time &) = delete;
        ~Time() final = default;

        uint64_t getTime() const;
        void setTime(uint64_t time);

    private:
        uint64_t microseconds;
};

}
