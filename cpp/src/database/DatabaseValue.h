#pragma once

#include <string>

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
        std::string getString() const;

    private:
        std::string value;
};

}
