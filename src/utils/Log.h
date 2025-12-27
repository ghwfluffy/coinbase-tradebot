#pragma once

namespace gtb::log
{
    void info(const char *, ...)
        __attribute__((format(printf, 1, 2)));
    void error(const char *, ...)
        __attribute__((format(printf, 1, 2)));

    // Trade-level logging (can be toggled at runtime).
    void trade(const char *, ...)
        __attribute__((format(printf, 1, 2)));

    // Enable/disable trade-level logging.
    void setTradeLoggingEnabled(bool enabled);
}
