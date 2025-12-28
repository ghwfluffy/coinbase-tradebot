#pragma once

#include <cstdint>
#include <string>

namespace gtb::log
{
    void info(const char *, ...)
        __attribute__((format(printf, 1, 2)));
    void error(const char *, ...)
        __attribute__((format(printf, 1, 2)));
    void status(const char *, ...)
        __attribute__((format(printf, 1, 2)));

    // Trade-level logging (can be toggled at runtime).
    void trade(const char *, ...)
        __attribute__((format(printf, 1, 2)));

    // Enable/disable trade-level logging.
    void setTradeLoggingEnabled(bool enabled);
    bool isTradeLoggingEnabled();

    // Debug-level logging (can be toggled at runtime).
    void debug(const char *, ...)
        __attribute__((format(printf, 1, 2)));
    void setDebugLoggingEnabled(bool enabled);
    bool isDebugLoggingEnabled();

    // Allow mocking the timestamp source (e.g., mock time).
    void setMockNowMicros(uint64_t micros);

    // Duplicate logs to a file path.
    void setLogFile(const std::string &path);
}
