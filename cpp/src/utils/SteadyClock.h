#pragma once

#include <gtb/Time.h>

#include <chrono>

#include <stdint.h>

namespace gtb
{

/**
 * Acts like std::chrono::steady_clock but respects the mock time
 */
namespace SteadyClock
{
    struct TimePoint
    {
        // Time in microseconds since epoch
        uint64_t time = 0;

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
            auto count = micros.count();
            if (count > 0)
                time += static_cast<uint64_t>(count);
            else
                time -= static_cast<uint64_t>(count * -1);

            return (*this);
        }

        template<typename Rep, typename Period>
        TimePoint &operator-=(const std::chrono::duration<Rep, Period>& d)
        {
            auto micros = std::chrono::duration_cast<std::chrono::microseconds>(d);
            auto count = micros.count();
            if (count > 0)
                time -= static_cast<uint64_t>(count);
            else
                time += static_cast<uint64_t>(count * -1);

            return (*this);
        }
        #pragma GCC diagnostic pop
    };

    TimePoint now();

    void setMockTime(
        Time &time);

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
