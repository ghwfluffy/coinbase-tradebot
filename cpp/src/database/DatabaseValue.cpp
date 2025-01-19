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

std::string DatabaseValue::getString() const
{
    return value;
}
