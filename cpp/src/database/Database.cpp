#include <gtb/Database.h>
#include <gtb/Log.h>

#include <sqlite3.h>

#include <atomic>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace gtb;

namespace
{

std::atomic<int> refs = 0;

void initSqlite()
{
    if (refs++ == 0)
        sqlite3_initialize();
}

void cleanupSqlite()
{
    if (--refs == 0)
        sqlite3_shutdown();
}

}

Database::Database()
{
    initSqlite();
}

Database::~Database()
{
    cleanupSqlite();
}

DatabaseConnection Database::getConn()
{
    // Find existing connection in pool
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!pool.empty())
        {
            DatabaseConnection conn = std::move(pool.front());
            pool.pop_front();
            return DatabaseConnection(
                std::move(conn),
                std::bind(&Database::releaseDb, this, std::placeholders::_1));
        }
    }

    // Else new connection
    return newConn();
}

DatabaseConnection Database::newConn()
{
    if (dbFile.empty())
    {
        log::error("Database not initialized.");
        return DatabaseConnection();
    }

    sqlite3 *conn = nullptr;
    int res = sqlite3_open(dbFile.c_str(), &conn);
    if (res != SQLITE_OK)
    {
        log::error("Failed to open SQLite database '%s': %s",
            dbFile.c_str(),
            sqlite3_errmsg(conn));

        if (conn)
        {
            sqlite3_close(conn);
            conn = nullptr;
        }
    }

    // Set busy timeout to 1 second (1000 milliseconds)
    if (conn)
        sqlite3_busy_timeout(conn, 1'000);

    return DatabaseConnection(conn, std::bind(&Database::releaseDb, this, std::placeholders::_1));
}

void Database::init(
    std::string dbFileIn,
    std::string schemaFile)
{
    this->dbFile = std::move(dbFileIn);

    std::ifstream schemaStream(schemaFile);
    if (!schemaStream.is_open())
    {
        log::error("Failed to open database schema file '%s'.",
            schemaFile.c_str());
        return;
    }

    std::stringstream schema;
    schema << schemaStream.rdbuf();

    DatabaseConnection conn = newConn();

    log::info("Initializing database '%s' with schema '%s'.",
        dbFile.c_str(),
        schemaFile.c_str());

    if (!conn.execute(schema.str()))
        log::error("Failed to initialize database schema.");
}

void Database::releaseDb(
    DatabaseConnection conn)
{
    std::lock_guard<std::mutex> lock(mtx);
    pool.push_back(std::move(conn));
}
