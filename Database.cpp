#include "Database.h"
#include "Resource.h"
#include "Audio/Audio.h"
#include "Audio/Metadata.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <sqlite3.h>

#include <iostream>
#include <string>
#include <map>


namespace fs = boost::filesystem;

Database::~Database() 
{
    //sqlite3_finalize(statement);
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
    query("SELECT path || filename FROM resource JOIN path USING (path_id) WHERE id = ?");
    bindInt(id);
    step();

    return getString();
}

int Database::insertAudio(const fs::path &file, int path)
{
    int id = insertFile(file, path, AUDIO);
    if(id <= 0) {
        std::cout << "File not inserted properly, aborting audio insert" << std::endl;
        return -1;
    }

    Audio audio = Audio();
    audio.init(file);
    Metadata *meta = audio.getMetadata();

    query("INSERT INTO audio_track VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    bindInt(id);
    bindString(meta->title);
    bindString(meta->artist);
    bindString(meta->album);
    bindString(meta->albumartist);
    bindInt(atoi(meta->tracknumber.c_str()));
    bindInt(meta->length);
    bindInt(meta->bitrate);
    bindString(meta->musicbrainz_trackid);
    step();

    delete meta;

    return last_insert_id();
}

int Database::insertImage(const fs::path &file, int path)
{
    int id = insertFile(file, path, IMAGE);
    if(id <= 0) {
        std::cout << "File not inserted properly, aborting image insert" << std::endl;
        return -1;
    }

    query("INSERT INTO image (id, title) VALUES (?, ?)");
    bindInt(id);
    bindString("bilde");
    step();

    return last_insert_id();
}

int Database::insertDir(const fs::path &path, int parent, int type)
{
    query("INSERT INTO path (path, parent) VALUES (?, ?)");

    fs::path dir = path.leaf() == "." ? path : path / "/";
    bindString(dir.string());
    bindInt(parent);
    bindInt(type);

    step();

    return last_insert_id();
}

int Database::insertFile(const fs::path &file, int path, int type)
{
    query("INSERT INTO resource (path_id, filename, type, size) VALUES (?, ?, ?, ?)");

    bindInt(path);
    bindString(file.leaf());
    bindInt(type);
    bindInt(fs::file_size(file));

    step();

    return last_insert_id();
}

bool Database::createTables()
{
    // TODO: Remove "IF NOT EXISTS" in favour of proper db checking
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
        album_artist TEXT, \
        tracknumber INTEGER, \
        length INTEGER, \
        bitrate INTEGER, \
        mbid_tid TEXT)";
    std::string image = "CREATE TABLE IF NOT EXISTS image ( \
        id INTEGER PRIMARY KEY, \
        title TEXT)";

    insert(path);
    insert(resource);
    insert(type);
    insert(category);
    insert(audioTrack);
    insert(image);

    return true;
}

