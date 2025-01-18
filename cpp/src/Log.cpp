#include <gtb/Log.h>
#include <gtb/Variadic.h>

#include <stdio.h>

using namespace gtb;

void log::info(const char *psz, ...)
{
    std::string msg;
    VARIADIC_STRING(psz, msg);
    printf("[INFO ] %s\n", msg.c_str());
}

void log::error(const char *psz, ...)
{
    std::string msg;
    VARIADIC_STRING(psz, msg);
    printf("[ERROR] %s\n", msg.c_str());
}
