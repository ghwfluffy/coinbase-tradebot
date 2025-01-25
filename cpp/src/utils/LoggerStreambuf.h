#pragma once

#include <mutex>
#include <memory>
#include <string>
#include <iostream>
#include <streambuf>
#include <functional>

namespace gtb
{

/**
 * An ostream that writes to a callback
 */
class LoggerStreambuf : public std::streambuf
{
    public:
        LoggerStreambuf();
        LoggerStreambuf(std::function<void(std::string)> callback);
        LoggerStreambuf(LoggerStreambuf &&);
        LoggerStreambuf(const LoggerStreambuf &) = delete;
        LoggerStreambuf &operator=(LoggerStreambuf &&);
        LoggerStreambuf &operator=(const LoggerStreambuf &) = delete;

        operator std::ostream *();

    protected:
        int sync() final;
        int overflow(int c) final;

    private:
        std::mutex mtx;
        std::string buffer;
        std::ostream writeStream;
        std::function<void(std::string)> callback;
};

}
