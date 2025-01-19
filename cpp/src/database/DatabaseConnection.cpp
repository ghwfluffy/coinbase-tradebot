#include <gtb/DatabaseConnection.h>
#include <gtb/Log.h>

using namespace gtb;

DatabaseConnection::DatabaseConnection()
{
    conn = nullptr;
}

DatabaseConnection::DatabaseConnection(sqlite3 *conn)
{
    this->conn = conn;
}

DatabaseConnection::DatabaseConnection(DatabaseConnection &&rhs)
{
    this->conn = rhs.conn;
    rhs.conn = nullptr;
}

DatabaseConnection &DatabaseConnection::operator=(DatabaseConnection &&rhs)
{
    if (this != &rhs)
    {
        close();
        this->conn = rhs.conn;
        rhs.conn = nullptr;
    }

    return (*this);
}

DatabaseConnection::~DatabaseConnection()
{
    close();
}

void DatabaseConnection::close()
{
    if (conn)
    {
        sqlite3_close(conn);
        conn = nullptr;
    }
}

DatabaseResult DatabaseConnection::query(
    const std::string &str)
{
    if (!conn)
    {
        log::error("Failed to execute query: Database not connected.");
        return DatabaseResult();
    }

    if (str.rfind("SELECT", 0) != 0)
    {
        log::error("Failed to execute query: Invalid SELECT statement. Use execute().");
        return DatabaseResult();
    }

    sqlite3_stmt *stmt = nullptr;
    int res = sqlite3_prepare_v2(conn, str.c_str(), -1, &stmt, nullptr);
    if (res != SQLITE_OK)
    {
        log::error("Failed to execute query: %s", sqlite3_errmsg(conn));
        if (stmt)
        {
            sqlite3_finalize(stmt);
            stmt = nullptr;
        }
    }

    return DatabaseResult(stmt);
}

bool DatabaseConnection::execute(
    const std::string &str)
{
    if (!conn)
    {
        log::error("Failed to execute query: Database not connected.");
        return DatabaseResult();
    }

    int res = sqlite3_exec(conn, str.c_str(), nullptr, nullptr, nullptr);
    if (res != SQLITE_OK)
    {
        log::error("Failed to execute query: %s", sqlite3_errmsg(conn));
        return false;
    }

    return true;
}
