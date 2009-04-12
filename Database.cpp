#include "database.h"

bool Datahase::init(const char *filename)
{
    return sqlite3_open(filename, &db) == SQLITE_OK;
}

bool Database::createTables()
{
    std::string files = "CREATE TABLE file (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        path INTEGER,
        filename TEXT,
        size INTEGER,
        type INTEGER,
        hash TEXT)";

    std::string path = "CREATE TABLE path (
        id INTEGER PRIMARY KEY,
        path TEXT NOT NULL,
        parent INTEGER)";

    std::string type = "CREATE TABLE type (
        id INTEGER PRIMARY KEY,
        extension TEXT NOT NULL,
        category INTEGER)";

    std::string category = "CREATE TABLE category (
        id INTEGER PRIMARY KEY,
        name TEXT)";
    
    std::string audioTrack = "CREATE TABLE audio_track (
        id INTEGER PRIMARY KEY,
        title TEXT,
        artist TEXT,
        album TEXT,
        tracknumber INTEGER,
        mbid_tid TEXT)";

    if(insert(file) && insert(path) && insert(type) && insert(category) && insert(audioTrack))
        return true;
    else
        return false;
}

bool insert(std::string query)
{
    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(
        db, 
        query.c_str(), 
        query.size() * sizeof(std::string::value_type),
        statement,
        NULL
    );

    if(result != SQLITE_OK) {
        std::cout << "Query preparation failed: Error #" << ret << std::endl << query.c_str() << std::endl;
        return false;
    }

    result = sqlite3_step(statement);
    if(result != SQLITE_OK) {
        std::cout << "DB insert failed: Error #" << ret << std::endl << query.c_str() << std::endl;
        return false;
    }

    ret = sqlite3_finalize(statement);
    if(result != SQLITE_OK) {
        std::cout << "Something failed: Error #" << ret << std::endl << query.c_str() << std::endl;
        return false;
    }
}
