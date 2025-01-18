#include <gtb/Log.h>
#include <gtb/Variadic.h>

#include <stdio.h>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>

using namespace gtb;

namespace
{

std::string getTime()
{
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm local_tm;
    localtime_r(&now_time_t, &local_tm);

    std::ostringstream time_stream;
    time_stream << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
    return time_stream.str();
}

}

void log::info(const char *psz, ...)
{
    std::string msg;
    VARIADIC_STRING(psz, msg);
    printf("[%s] [ INFO  ] %s\n", getTime().c_str(), msg.c_str());
}

void log::error(const char *psz, ...)
{
    std::string msg;
    VARIADIC_STRING(psz, msg);
    printf("[%s] [ ERROR ] %s\n", getTime().c_str(), msg.c_str());
}
