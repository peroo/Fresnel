#include "SQLite.h"

#include <sqlite3.h>

#include <iostream>
#include <string>
#include <map>

sqlite3 *SQLite::db;

bool SQLite::selectDB(std::string filename)
{
    return sqlite3_open(filename.c_str(), &db) == SQLITE_OK;
}

void SQLite::query(std::string query)
{
    paramIndex = 0;
    /*if(statement != NULL)
        sqlite3_finalize(statement);*/

    int result = sqlite3_prepare_v2(
        db,
        query.c_str(),
        query.size() + 1,
        &statement,
        0
    );
    if(result != SQLITE_OK) {
        std::cout << "Query preparation failed: Error #" << result << std::endl << query.c_str() << std::endl;
        std::cout << sqlite3_errmsg(db) << std::endl;
    }
}

void SQLite::bindInt(int value)
{
    int result = sqlite3_bind_int(statement, ++paramIndex, value);
}
void SQLite::bindString(std::string value)
{
    sqlite3_bind_text(statement, ++paramIndex, value.c_str(), value.size(), SQLITE_TRANSIENT);
}
int SQLite::getInt()
{
    return sqlite3_column_int(statement, colIndex++);
}
double SQLite::getFloat()
{
    return sqlite3_column_double(statement, colIndex++);
}
std::string SQLite::getString()
{
    const unsigned char* text = sqlite3_column_text(statement, colIndex);
    int bytes = sqlite3_column_bytes(statement, colIndex++);
    return std::string((const char*)text, bytes);
}
void SQLite::getVoid()
{
    ++colIndex;
}

int SQLite::step()
{
    colIndex = 0;
    int result = sqlite3_step(statement);
    if(result == SQLITE_ROW)
        return 1;
    else if(result == SQLITE_DONE)
        return 0;
    else {
        //std::cout << "DB step failed: Error #" << result << std::endl;
        //std::cout << sqlite3_errmsg(db) << std::endl;
        return -1;
    }
}

void SQLite::insert(std::string _query)
{
    query(_query);
    step();
}

int SQLite::last_insert_id()
{
    return sqlite3_last_insert_rowid(db);
}
int SQLite::rows_affected()
{
    return sqlite3_changes(db);
}

int SQLite::ColCount()
{
    sqlite3_column_count(statement);
}

int SQLite::ColType(int index)
{
    sqlite3_column_type(statement, index);
}

std::string SQLite::ColName(int index)
{
    return std::string(sqlite3_column_name(statement, index));
}

