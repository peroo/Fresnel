#ifndef SQLITE_H
#define SQLITE_H

#include <string>
#include <map>
#include <memory>

struct sqlite3;
struct sqlite3_stmt;

class SQLite {
    public:
        void            init();
    protected:
        SQLite() {}
        ~SQLite();
        void            insert(const std::string &query);
        void            query(const std::string &query);
        bool            step();
        int             last_insert_id();
        int             rows_affected();
        void            bindInt(int value);
        void            bindInt64(uint64_t value);
        void            bindString(const std::string &value);
        void            bindNull();
        int             getInt();
        uint64_t        getInt64();
        double          getFloat();
        std::string     getString();
        void            getVoid();
        int             ColCount();
        int             ColType(int index);
        std::string     ColName(int index);
    private:
        sqlite3 *db;
        std::map<std::string, sqlite3_stmt*> statements;
        sqlite3_stmt *statement;
        int paramIndex;
        int colIndex;
};

#endif
