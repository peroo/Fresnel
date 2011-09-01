#ifndef SQLITE_H
#define SQLITE_H

#include <string>
#include <map>

struct sqlite3;
struct sqlite3_stmt;

class SQLite {
    public:
        SQLite() : used(false) {}
        ~SQLite();
        static bool selectDB(std::string filename);
    protected:
        void            insert(std::string query);
        void            query(std::string query);
        bool            step();
        int             last_insert_id();
        int             rows_affected();
        void            bindInt(int value);
        void            bindInt64(uint64_t value);
        void            bindString(std::string value);
        int             getInt();
        uint64_t        getInt64();
        double          getFloat();
        std::string     getString();
        void            getVoid();
        int             ColCount();
        int             ColType(int index);
        std::string     ColName(int index);
    private:
        static sqlite3 *db;
        std::map<std::string, sqlite3_stmt*> statements;
        sqlite3_stmt *statement;
        int paramIndex;
        int colIndex;
        bool used;
};

#endif
