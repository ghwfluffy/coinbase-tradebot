#include <gtb/LoggerStreambuf.h>

using namespace gtb;

LoggerStreambuf::LoggerStreambuf()
    : writeStream(this)
{
}

LoggerStreambuf::LoggerStreambuf(
    std::function<void(std::string)> callback)
        : writeStream(this)
        , callback(std::move(callback))
{
}

LoggerStreambuf::LoggerStreambuf(
    LoggerStreambuf &&rhs)
        : writeStream(this)
        , callback(std::move(rhs.callback))
{
}

LoggerStreambuf &LoggerStreambuf::operator=(LoggerStreambuf &&rhs)
{
    if (this != &rhs)
    {
        buffer = std::move(rhs.buffer);
        callback = std::move(rhs.callback);
    }

    return (*this);
}

LoggerStreambuf::operator std::ostream *()
{
    return &writeStream;
}

int LoggerStreambuf::sync()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (!buffer.empty())
    {
        callback(std::move(buffer));
        buffer.clear();
    }

    return 0;
}

int LoggerStreambuf::overflow(int c)
{
    if (c != EOF && c != '\n')
    {
        std::lock_guard<std::mutex> lock(mtx);
        buffer += static_cast<char>(c);
    }

    if (c == '\n')
        sync();

    return c;
}
