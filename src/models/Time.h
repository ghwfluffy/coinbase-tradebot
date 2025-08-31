#pragma once

#include <gtb/DataModel.h>
#include <gtb/IntLiterals.h>

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

        utime_t getTime() const;
        void setTime(utime_t time);

        void setNow();

    private:
        utime_t microseconds;
};

}
