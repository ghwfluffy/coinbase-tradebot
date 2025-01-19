#include <gtb/DatabaseValue.h>

#include <sqlite3.h>

namespace gtb
{

class DatabaseResult
{
    public:
        DatabaseResult();
        DatabaseResult(sqlite3_stmt *stmt);
        DatabaseResult(DatabaseResult &&);
        DatabaseResult(const DatabaseResult &) = delete;
        DatabaseResult &operator=(DatabaseResult &&);
        DatabaseResult &operator=(const DatabaseResult &) = delete;
        ~DatabaseResult();

        operator bool() const;

        unsigned int numResults() const;
        unsigned int numColumns() const;

        bool next();

        DatabaseValue operator[](size_t index);

    private:
        void cleanup();

        sqlite3_stmt *stmt;
};

}
