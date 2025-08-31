#pragma once

#include <gtb/Time.h>
#include <gtb/IntLiterals.h>

#include <chrono>

namespace gtb
{

/**
 * Wrapper for std::chrono::steady_clock that allows taking the current
 * time from a static value (setMockTime) so steady timers and calculations
 * involving time can work in the mocked environment.
 */
namespace SteadyClock
{
    struct TimePoint
    {
        // Time in microseconds since epoch
        utime_t time;

        bool operator<(const TimePoint &) const;
        bool operator>(const TimePoint &) const;
        bool operator<=(const TimePoint &) const;
        bool operator>=(const TimePoint &) const;
        bool operator==(const TimePoint &) const;
        bool operator!=(const TimePoint &) const;

        // Arithmetic assignment with chrono durations
        // duplicated-branches and signed-conversion don't like eachothere right here
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wduplicated-branches"
        template<typename Rep, typename Period>
        TimePoint &operator+=(const std::chrono::duration<Rep, Period>& d)
        {
            auto micros = std::chrono::duration_cast<std::chrono::microseconds>(d);
            int64_t count = static_cast<int64_t>(micros.count());
            if (count > 0)
                time += utime_t(static_cast<uint64_t>(count));
            else
                time -= utime_t(static_cast<uint64_t>(count * -1L));

            return (*this);
        }

        template<typename Rep, typename Period>
        TimePoint &operator-=(const std::chrono::duration<Rep, Period>& d)
        {
            auto micros = std::chrono::duration_cast<std::chrono::microseconds>(d);
            int64_t count = static_cast<int64_t>(micros.count());
            if (count > 0)
                time -= utime_t(static_cast<uint64_t>(count));
            else
                time += utime_t(static_cast<uint64_t>(count * -1L));

            return (*this);
        }
        #pragma GCC diagnostic pop
    };

    TimePoint now();

    void setMockTime(
        const Time &time);

    // Non-member arithmetic operators
    template<typename Rep, typename Period>
    TimePoint operator+(const TimePoint &tp, const std::chrono::duration<Rep, Period> &d)
    {
        TimePoint result = tp;
        result += d;
        return result;
    }

    template<typename Rep, typename Period>
    TimePoint operator+(const std::chrono::duration<Rep, Period> &d, const TimePoint &tp)
    {
        return tp + d;
    }

    template<typename Rep, typename Period>
    TimePoint operator-(const TimePoint &tp, const std::chrono::duration<Rep, Period> &d)
    {
        TimePoint result = tp;
        result -= d;
        return result;
    }

    // Subtraction between two TimePoints returns a chrono duration in microseconds
    std::chrono::microseconds operator-(const TimePoint &lhs, const TimePoint &rhs);
}

}
