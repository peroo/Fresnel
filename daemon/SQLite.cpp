#include "SQLite.h"

#include <sqlite3.h>

#include <iostream>
#include <string>
#include <map>
#include <assert.h>

SQLite::~SQLite() {
    for(auto iter = statements.begin(); iter != statements.end(); iter++) {
        sqlite3_finalize(iter->second);
    }
    sqlite3_close(db);
}

void SQLite::init()
{
    std::string filename = "db.sqlite";
    int result = sqlite3_open_v2(
            filename.c_str(),
            &db, 
            SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE |  SQLITE_OPEN_NOMUTEX,
            NULL
    );
    if(result != SQLITE_OK) {
        throw("sqlite error");
    }

    insert("PRAGMA foreign_keys = true;");
    insert("PRAGMA journal_mode=WAL;");
    insert("PRAGMA synchronous=NORMAL;");
}

void SQLite::query(const std::string &query)
{
    auto iter = statements.find(query);
    if(iter != statements.end()) {
        statement = iter->second;
        sqlite3_reset(statement);
        paramIndex = 0;
        return;
    }

    paramIndex = 0;

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

    statements[query] = statement;
}

void SQLite::bindInt(int value)
{
    int result = sqlite3_bind_int(statement, ++paramIndex, value);
    assert(result == SQLITE_OK);
}
void SQLite::bindInt64(uint64_t value)
{
    int result = sqlite3_bind_int64(statement, ++paramIndex, value);
    assert(result == SQLITE_OK);
}
void SQLite::bindString(const std::string &value)
{
    int result = sqlite3_bind_text(statement, ++paramIndex, value.c_str(), 
                                    value.size(), SQLITE_STATIC);
    assert(result == SQLITE_OK);
}
void SQLite::bindNull()
{
    int result = sqlite3_bind_null(statement, ++paramIndex);
    assert(result == SQLITE_OK);
}
int SQLite::getInt()
{
    return sqlite3_column_int(statement, colIndex++);
}
uint64_t SQLite::getInt64()
{
    return sqlite3_column_int64(statement, colIndex++);
}
double SQLite::getFloat()
{
    return sqlite3_column_double(statement, colIndex++);
}
std::string SQLite::getString()
{
    const unsigned char* text = sqlite3_column_text(statement, colIndex);
    int bytes = sqlite3_column_bytes(statement, colIndex++);
    std::string str = std::string((const char*)text, bytes);
    return str;
}
void SQLite::getVoid()
{
    ++colIndex;
}

bool SQLite::step()
{
    colIndex = 0;
    int result = sqlite3_step(statement);
    if(result == SQLITE_ROW)
        return true;
    else if(result == SQLITE_DONE)
        return false;
    else {
        std::cout << "DB step failed: Error #" << result << std::endl;
        std::cout << sqlite3_errmsg(db) << std::endl;
        return false;
    }
}

void SQLite::insert(const std::string &_query)
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
    return sqlite3_column_count(statement);
}

int SQLite::ColType(int index)
{
    return sqlite3_column_type(statement, index);
}

std::string SQLite::ColName(int index)
{
    return std::string(sqlite3_column_name(statement, index));
}

