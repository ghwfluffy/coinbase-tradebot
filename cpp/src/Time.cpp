#include <gtb/Time.h>

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
    this->microseconds = time;
    updated();
}
