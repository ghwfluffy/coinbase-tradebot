#pragma once

#include <gtb/DatabaseConnection.h>

#include <list>
#include <mutex>

namespace gtb
{

class Database
{
    public:
        Database();
        Database(Database &&) = delete;
        Database(const Database &) = delete;
        Database &operator=(Database &&) = delete;
        Database &operator=(const Database &) = delete;
        ~Database();

        DatabaseConnection getConn();

        void init(
            std::string dbFile,
            std::string schemaFile);

    private:
        DatabaseConnection newConn();

        void releaseDb(
            DatabaseConnection conn);

        std::mutex mtx;
        std::string dbFile;
        std::list<DatabaseConnection> pool;
};

}
