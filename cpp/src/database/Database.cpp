#include <gtb/Database.h>
#include <gtb/Log.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace gtb;

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

    return DatabaseConnection(conn);
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
