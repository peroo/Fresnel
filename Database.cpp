#include "Database.h"
#include "Audio/Audio.h"
#include "Audio/Metadata.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <sqlite3.h>
#include <iostream>

sqlite3 *Database::db;

namespace fs = boost::filesystem;

bool Database::init(const char *filename)
{
    return sqlite3_open(filename, &db) == SQLITE_OK;
}

bool Database::createTables()
{
    // TODO: Remove "IF NOT EXISTS" in favour of proper db checking
    // TODO: Remove drop-tables when indexing synchronises properly instead of duplicating
    std::string file = "CREATE TABLE IF NOT EXISTS file ( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        path INTEGER, \
        filename TEXT, \
        size INTEGER, \
        type INTEGER, \
        hash TEXT)";
    std::string dropFile = "DROP TABLE file";

    std::string path = "CREATE TABLE IF NOT EXISTS path ( \
        id INTEGER PRIMARY KEY, \
        path TEXT NOT NULL, \
        parent INTEGER)";
    std::string dropPath = "DROP TABLE path";

    std::string type = "CREATE TABLE IF NOT EXISTS type ( \
        id INTEGER PRIMARY KEY, \
        extension TEXT NOT NULL, \
        category INTEGER)";
    std::string dropType = "DROP TABLE type";

    std::string category = "CREATE TABLE IF NOT EXISTS category ( \
        id INTEGER PRIMARY KEY, \
        name TEXT)";
    std::string dropCategory = "DROP TABLE category";
    
    std::string audioTrack = "CREATE TABLE IF NOT EXISTS audio_track ( \
        id INTEGER PRIMARY KEY, \
        title TEXT, \
        artist TEXT, \
        album TEXT, \
        tracknumber INTEGER, \
        mbid_tid TEXT)";
    std::string dropAudioTrack = "DROP TABLE audio_track";

    insert(dropPath);
    insert(path);
    insert(dropFile);
    insert(file);
    insert(dropType);
    insert(type);
    insert(dropCategory);
    insert(category);
    insert(dropAudioTrack);
    insert(audioTrack);

    return true;
}

bool Database::insertAudio(const fs::path &file, int path)
{
    int id = insertFile(file, path);
    if(id <= 0)
        return false;

    Audio *audio = new Audio();
    audio->init(path);
    Metadata *meta = audio->getMetadata();

    std::string query = "INSERT INTO audio_track VALUES (?, ?, ?, ?, ?, ?)";

    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(
        db,
        query.c_str(),
        -1,
        &statement,
        0
    );
    if(result != SQLITE_OK) {
        std::cout << "Query preparation failed: Error #" << result << std::endl << query.c_str() << std::endl;
        std::cout << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(statement, 1, id);
    sqlite3_bind_text(statement, 2, meta->title.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, meta->artist.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, meta->album.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 5, atoi(meta->tracknumber.c_str()));
    sqlite3_bind_text(statement, 6, meta->musicbrainz_trackid.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(statement);
    sqlite3_finalize(statement);

    return sqlite3_last_insert_rowid(db);

    delete meta;
    delete audio;

    return true;
}

int Database::insertDir(const fs::path &path, int parent, int type)
{
    std::string query = "INSERT INTO path (path, parent) VALUES (?, ?)";

    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(
        db,
        query.c_str(),
        -1,
        &statement,
        0
    );
    if(result != SQLITE_OK) {
        std::cout << "Query preparation failed: Error #" << result << std::endl << query.c_str() << std::endl;
        std::cout << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(statement, 1, parent ? path.leaf().c_str() : path.file_string().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 2, parent);
    sqlite3_bind_int(statement, 3, type);

    sqlite3_step(statement);
    sqlite3_finalize(statement);

    return sqlite3_last_insert_rowid(db); 
}

int Database::insertFile(const fs::path &file, int path)
{
    std::string query = "INSERT INTO file (path, filename, type, size) VALUES (?, ?, ?, ?)";

    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(
        db,
        query.c_str(),
        -1,
        &statement,
        0
    );
    if(result != SQLITE_OK) {
        std::cout << "Query preparation failed: Error #" << result << std::endl << query.c_str() << std::endl;
        std::cout << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(statement, 1, 1);
    sqlite3_bind_text(statement, 2, file.leaf().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 3, 1);
    sqlite3_bind_int(statement, 4, fs::file_size(file));

    sqlite3_step(statement);
    sqlite3_finalize(statement);

    return sqlite3_last_insert_rowid(db); 
}

bool Database::insert(std::string query)
{
    sqlite3_stmt *statement;
    int result = sqlite3_prepare_v2(
        db, 
        query.c_str(), 
        query.size() * sizeof(std::string::value_type),
        &statement,
        NULL
    );

    if(result != SQLITE_OK) {
        std::cout << "Query preparation failed: Error #" << result << std::endl << query.c_str() << std::endl;
        std::cout << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    result = sqlite3_step(statement);
    if(result != SQLITE_DONE) {
        std::cout << "DB insert failed: Error #" << result << std::endl << query.c_str() << std::endl;
        std::cout << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    result = sqlite3_finalize(statement);
    if(result != SQLITE_OK) {
        std::cout << "Something failed: Error #" << result << std::endl << query.c_str() << std::endl;
        return false;
    }
}
