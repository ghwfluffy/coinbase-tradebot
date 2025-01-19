#include <gtb/DatabaseResult.h>

using namespace gtb;

DatabaseResult::DatabaseResult()
{
    this->stmt = nullptr;
}

DatabaseResult::DatabaseResult(sqlite3_stmt *stmt)
{
    this->stmt = stmt;
}

DatabaseResult::DatabaseResult(DatabaseResult &&rhs)
{
    this->stmt = rhs.stmt;
    rhs.stmt = nullptr;
}

DatabaseResult &DatabaseResult::operator=(DatabaseResult &&rhs)
{
    if (this != &rhs)
    {
        cleanup();
        this->stmt = rhs.stmt;
        rhs.stmt = nullptr;
    }

    return (*this);
}

DatabaseResult::~DatabaseResult()
{
    cleanup();
}

void DatabaseResult::cleanup()
{
    if (stmt)
    {
        sqlite3_finalize(stmt);
        stmt = nullptr;
    }
}

DatabaseResult::operator bool() const
{
    return stmt != nullptr;
}

unsigned int DatabaseResult::numResults() const
{
    return stmt ? sqlite3_data_count(stmt) : 0;
}

unsigned int DatabaseResult::numColumns() const
{
    return stmt ? sqlite3_column_count(stmt) : 0;
}

bool DatabaseResult::next()
{
    if (!stmt)
        return false;

    int res = sqlite3_step(stmt);
    return res == SQLITE_ROW;
}

DatabaseValue DatabaseResult::operator[](size_t index)
{
    if (!stmt || index >= numColumns())
        return DatabaseValue();

    const char *text = reinterpret_cast<const char *>(sqlite3_column_text(stmt, index));
    return DatabaseValue(text ? text : "");
}
