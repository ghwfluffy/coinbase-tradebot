#include <gtb/Time.h>

#include <chrono>

using namespace gtb;

Time::Time()
{
}

utime_t Time::getTime() const
{
    return microseconds;
}

void Time::setTime(utime_t time)
{
    if (this->microseconds != time)
    {
        this->microseconds = time;
        updated();
    }
}

void Time::setNow()
{
    auto now = std::chrono::system_clock::now();
    auto epoch = std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch();
    uint64_t microseconds = static_cast<uint64_t>(epoch.count());
    setTime(utime_t(microseconds));
}
