#pragma once

namespace gtb::log
{
    void info(const char *, ...)
        __attribute__((format(printf, 1, 2)));
    void error(const char *, ...)
        __attribute__((format(printf, 1, 2)));
}
