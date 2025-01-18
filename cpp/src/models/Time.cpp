#include <gtb/Time.h>

#include <chrono>

using namespace gtb;

Time::Time()
{
    microseconds = 0;
}

uint64_t Time::getTime() const
{
    return microseconds;
}

void Time::setTime(uint64_t time)
{
    if (this->microseconds != time)
    {
        this->microseconds = time;
        updated();
    }
}

void Time::setNow()
{
    auto now = std::chrono::steady_clock::now();
    auto epoch = std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch();
    uint64_t microseconds = static_cast<uint64_t>(epoch.count());
    setTime(microseconds);
}
