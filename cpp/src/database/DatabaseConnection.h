#pragma once

#include <gtb/DatabaseResult.h>

#include <sqlite3.h>

#include <string>
#include <functional>

namespace gtb
{

class DatabaseConnection
{
    public:
        DatabaseConnection();
        DatabaseConnection(
            sqlite3 *conn,
            std::function<void(DatabaseConnection)> release =
                std::function<void(DatabaseConnection)>());
        DatabaseConnection(DatabaseConnection &&rhs,
            std::function<void(DatabaseConnection)> release =
                std::function<void(DatabaseConnection)>());
        DatabaseConnection(const DatabaseConnection &) = delete;
        DatabaseConnection &operator=(DatabaseConnection &&);
        DatabaseConnection &operator=(const DatabaseConnection &) = delete;
        ~DatabaseConnection();

        DatabaseResult query(
            const std::string &str);

        bool execute(
            const std::string &str);

    private:
        void close();

        sqlite3 *conn;
        std::function<void(DatabaseConnection)> release;
};

}
