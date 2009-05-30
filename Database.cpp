#include "Database.h"
#include "Resource.h"
#include "Audio/Audio.h"
#include "Audio/Metadata.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <sqlite3.h>
#include <iostream>
#include <map>

sqlite3 *Database::db;

namespace fs = boost::filesystem;

Database::~Database() {
    sqlite3_finalize(statement);
}

bool Database::selectDB(std::string filename)
{
    return sqlite3_open(filename.c_str(), &db) == SQLITE_OK;
}

int Database::getResourceType(int id)
{
    query("SELECT type FROM resource WHERE id = ?");
    bindInt(id);
    step();

    return getInt();
}

std::string Database::getResourcePath(int id)
{
    query("SELECT path || filename FROM resource JOIN path WHERE id = ?");
    bindInt(id);
    step();

    return getString();
}

int Database::insertAudio(const fs::path &file, int path)
{
    int id = insertFile(file, path);
    if(id <= 0) {
        std::cout << "File not inserted properly, aborting audio insert" << std::endl;
        return -1;
    }

    Audio *audio = Audio();
    audio->init(file);
    Metadata *meta = audio->getMetadata();

    query("INSERT INTO audio_track VALUES (?, ?, ?, ?, ?, ?)");
    bindInt(id);
    bindString(meta->title);
    bindString(meta->artist);
    bindString(meta->album);
    bindInt(atoi(meta->tracknumber.c_str()));
    bindString(meta->musicbrainz_trackid);
    step();

    delete meta;

    return sqlite3_last_insert_rowid(db);
}

int Database::insertDir(const fs::path &path, int parent, int type)
{
    query("INSERT INTO path (path, parent) VALUES (?, ?)");

    bindString(parent ? path.leaf() : path.file_string());
    bindInt(parent);
    bindInt(type);

    step();

    return sqlite3_last_insert_rowid(db); 
}

int Database::insertFile(const fs::path &file, int path)
{
    query("INSERT INTO resource (path_id, filename, type, size) VALUES (?, ?, ?, ?)");

    bindInt(1);
    bindString(file.parent_path().string());
    bindInt(1);
    bindInt(fs::file_size(file));

    step();

    return sqlite3_last_insert_rowid(db); 
}

bool Database::createTables()
{
    // TODO: Remove "IF NOT EXISTS" in favour of proper db checking
    // TODO: Remove drop-tables when indexing synchronises properly instead of duplicating
    std::string resource = "CREATE TABLE IF NOT EXISTS resource ( \
        id INTEGER PRIMARY KEY AUTOINCREMENT, \
        path_id INTEGER, \
        filename TEXT, \
        size INTEGER, \
        type INTEGER, \
        hash TEXT)";

    std::string path = "CREATE TABLE IF NOT EXISTS path ( \
        path_id INTEGER PRIMARY KEY, \
        path TEXT NOT NULL, \
        parent INTEGER)";

    std::string type = "CREATE TABLE IF NOT EXISTS type ( \
        id INTEGER PRIMARY KEY, \
        extension TEXT NOT NULL, \
        category INTEGER)";

    std::string category = "CREATE TABLE IF NOT EXISTS category ( \
        id INTEGER PRIMARY KEY, \
        name TEXT)";
    
    std::string audioTrack = "CREATE TABLE IF NOT EXISTS audio_track ( \
        id INTEGER PRIMARY KEY, \
        title TEXT, \
        artist TEXT, \
        album TEXT, \
        tracknumber INTEGER, \
        mbid_tid TEXT)";

    insert(path);
    insert(resource);
    insert(type);
    insert(category);
    insert(audioTrack);

    return true;
}

void Database::query(std::string query)
{
    if(statement != NULL)
        sqlite3_finalize(statement);

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
        return false;
    }
}

void Database::bindInt(int value)
{
    sqlite3_bind_int(statement, ++paramIndex, value);
}
void Database::bindString(std::string value)
{
    sqlite3_bind_text(statement, ++paramIndex, value.c_str(), value.size() + 1, SQLITE_TRANSIENT);
}
int Database::getInt()
{
    sqlite3_column_int(statement, ++colIndex);
}
std::string Database::getString()
{
    return std::string(sqlite3_column_text(statement, ++colIndex), sqlite3_column_bytes(statement, colIndex));
}

bool Database::step()
{
    colIndex = 0;
    int result = sqlite3_step(statement);
    if(result == SQLITE_OK)
        return true;
    else if(result == SQLITE_DONE)
        return false;
    else {
        std::cout << "DB step failed: Error #" << result << std::endl << query.c_str() << std::endl;
        std::cout << sqlite3_errmsg(db) << std::endl;
        return false;
    }
}

void Database::insert(std::string _query)
{
    query(_query);
    step();
}
