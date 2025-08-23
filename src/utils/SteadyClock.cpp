#include <gtb/SteadyClock.h>

using namespace gtb;

namespace
{

const Time *mockTime = nullptr;

}

SteadyClock::TimePoint SteadyClock::now()
{
    SteadyClock::TimePoint tp;
    if (mockTime)
        tp.time = mockTime->getTime();
    else
    {
        auto now = std::chrono::steady_clock::now();
        auto epoch = std::chrono::time_point_cast<std::chrono::microseconds>(now).time_since_epoch();
        tp.time = static_cast<uint64_t>(epoch.count());
    }

    return tp;
}

void SteadyClock::setMockTime(const Time &time)
{
    mockTime = &time;
}

bool SteadyClock::TimePoint::operator<(const SteadyClock::TimePoint &other) const {
    return time < other.time;
}

bool SteadyClock::TimePoint::operator>(const SteadyClock::TimePoint &other) const {
    return time > other.time;
}

bool SteadyClock::TimePoint::operator<=(const SteadyClock::TimePoint &other) const {
    return time <= other.time;
}

bool SteadyClock::TimePoint::operator>=(const SteadyClock::TimePoint &other) const {
    return time >= other.time;
}

bool SteadyClock::TimePoint::operator==(const SteadyClock::TimePoint &other) const {
    return time == other.time;
}

bool SteadyClock::TimePoint::operator!=(const SteadyClock::TimePoint &other) const {
    return time != other.time;
}

namespace gtb::SteadyClock
{
    std::chrono::microseconds operator-(const SteadyClock::TimePoint &lhs, const SteadyClock::TimePoint &rhs) {
        return std::chrono::microseconds(lhs.time - rhs.time);
    }
}
