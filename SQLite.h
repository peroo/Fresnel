#ifndef SQLITE_H
#define SQLITE_H

#include <string>

struct sqlite3;
struct sqlite3_stmt;

class SQLite {
    public:
        static bool selectDB(std::string filename);
    protected:
        void            insert(std::string query);
        void            query(std::string query);
        bool            step();
        int             last_insert_id();
        int             rows_affected();
        void            bindInt(int value);
        void            bindString(std::string value);
        int             getInt();
        double          getFloat();
        std::string     getString();
        int             ColCount();
        int             ColType(int index);
        std::string     ColName(int index);
    private:
        static sqlite3 *db;
        sqlite3_stmt *statement;
        int paramIndex;
        int colIndex;
};

#endif
