#pragma once

#include <gtb/DatabaseConnection.h>

namespace gtb
{

class Database
{
    public:
        Database() = default;
        Database(Database &&) = default;
        Database(const Database &) = default;
        Database &operator=(Database &&) = default;
        Database &operator=(const Database &) = default;
        ~Database() = default;

        DatabaseConnection newConn();

        void init(
            std::string dbFile,
            std::string schemaFile);

    private:
        std::string dbFile;
};

}
