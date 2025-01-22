#pragma once

#include <string>

#include <stdint.h>

namespace gtb
{

class DatabaseValue
{
    public:
        DatabaseValue() = default;
        DatabaseValue(
            std::string value);
        DatabaseValue(DatabaseValue &&) = default;
        DatabaseValue(const DatabaseValue &) = default;
        DatabaseValue &operator=(DatabaseValue &&) = default;
        DatabaseValue &operator=(const DatabaseValue &) = default;
        ~DatabaseValue() = default;

        int getInt() const;
        uint32_t getUInt32() const;
        uint64_t getUInt64() const;
        std::string getString() const;

    private:
        std::string value;
};

}
