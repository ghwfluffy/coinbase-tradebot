#include <gtb/Log.h>
#include <gtb/Variadic.h>

#include <stdio.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

using namespace gtb;

namespace
{

bool debugLoggingEnabled = false;
bool tradeLoggingEnabled = false;

uint64_t mockNowMicros = 0;
FILE *logFile = nullptr;

std::string getTime()
{
    std::chrono::system_clock::time_point now;
    if (mockNowMicros)
        now = std::chrono::system_clock::time_point(std::chrono::microseconds(mockNowMicros));
    else
        now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm local_tm;
    localtime_r(&now_time_t, &local_tm);

    std::ostringstream time_stream;
    time_stream << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S")
                << '.' << std::setfill('0') << std::setw(3) << now_ms.count()
                << ' ' << std::put_time(&local_tm, "(%a)");
    return time_stream.str();
}

void writeLog(
    const char *level,
    const std::string &msg,
    bool console = true)
{
    std::ostringstream out;
    out << '[' << getTime() << "] " << level << ' ' << msg;
    std::string line = out.str();

    if (console)
    {
        fputs(line.c_str(), stdout);
        fputc('\n', stdout);
        fflush(stdout);
    }

    if (logFile)
    {
        fputs(line.c_str(), logFile);
        fputc('\n', logFile);
        fflush(logFile);
    }
}

}

void log::info(const char *psz, ...)
{
    std::string msg;
    VARIADIC_STRING(psz, msg);
    writeLog("[  INFO  ]", msg);
}

void log::error(const char *psz, ...)
{
    std::string msg;
    VARIADIC_STRING(psz, msg);
    writeLog("[ ERROR  ]", msg);
}

void log::status(const char *psz, ...)
{
    std::string msg;
    VARIADIC_STRING(psz, msg);
    writeLog("[ STATUS ]", msg);
}

void log::trade(const char *psz, ...)
{
    if (!tradeLoggingEnabled)
        return;

    std::string msg;
    VARIADIC_STRING(psz, msg);
    writeLog("[ TRADE  ]", msg);
}

void log::setTradeLoggingEnabled(bool enabled)
{
    tradeLoggingEnabled = enabled;
}

bool log::isTradeLoggingEnabled()
{
    return tradeLoggingEnabled;
}

void log::debug(const char *psz, ...)
{
    if (!debugLoggingEnabled)
        return;

    std::string msg;
    VARIADIC_STRING(psz, msg);
    writeLog("[ DEBUG ]", msg, !logFile);
}

void log::setDebugLoggingEnabled(bool enabled)
{
    debugLoggingEnabled = enabled;
}

bool log::isDebugLoggingEnabled()
{
    return debugLoggingEnabled;
}

void log::setMockNowMicros(uint64_t micros)
{
    if (!mockNowMicros)
        info("Updating logs to use Mocked time.");
    mockNowMicros = micros;
}

void log::setLogFile(const std::string &path)
{
    if (logFile)
    {
        fclose(logFile);
        logFile = nullptr;
    }

    if (path.empty())
        return;

    logFile = fopen(path.c_str(), "a");
    if (!logFile)
        fprintf(stderr, "Failed to open log file '%s'.\n", path.c_str());
}
