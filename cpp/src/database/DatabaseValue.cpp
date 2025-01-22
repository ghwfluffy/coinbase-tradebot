#include <gtb/DatabaseValue.h>

using namespace gtb;

DatabaseValue::DatabaseValue(
    std::string value)
        : value(std::move(value))
{
}

int DatabaseValue::getInt() const
{
    return atoi(value.c_str());
}

uint32_t DatabaseValue::getUInt32() const
{
    int64_t val = std::strtoll(value.c_str(), nullptr, 10);
    return val > 0xFFFFFFFFU ? 0xFFFFFFFFU :
        val > 0 ? static_cast<uint32_t>(val) : 0;
}

uint64_t DatabaseValue::getUInt64() const
{
    int64_t val = std::strtoll(value.c_str(), nullptr, 10);
    return val > 0 ? static_cast<uint64_t>(val) : 0;
}

std::string DatabaseValue::getString() const
{
    return value;
}
