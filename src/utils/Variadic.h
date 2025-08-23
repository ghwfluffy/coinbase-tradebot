#pragma once

#include <string>
#include <stdarg.h>

/**
 * Convert variadic arguments to std::string.
 * You should probably be using libfmt.
 *
 * @param format The char* format parameter
 * @param output The destination std::string
 */
#define VARIADIC_STRING(format, output) \
    { \
        va_list varArgs; \
            va_start(varArgs, format); \
        int size = vsnprintf(nullptr, 0, format, varArgs); \
        va_end(varArgs); \
 \
        if (size > 0) \
        { \
            output.resize(static_cast<size_t>(size)); \
            va_start(varArgs, format); \
            vsnprintf(&output[0], output.size(), format, varArgs); \
            va_end(varArgs); \
        } \
    }
