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

int Database::getArtistId(std::string name, std::string sortname)
{
    int id = artistCache[name];
    if(id)
        return id;

    query("SELECT id FROM artist \
            WHERE name=?");
    bindString(name);

    if(step()) {
        id = getInt();
    }
    else {
        query("INSERT INTO artist (name, sortname) VALUES (?, ?)");
        bindString(name);
        bindString(sortname);
        step();
        id = last_insert_id();
    }

    artistCache[name] = id;
    return id;
}

int Database::getAlbumId(std::string title, std::string date, int artist)
{
    int id = albumCache[title];
    if(id)
        return id;

    query("SELECT id FROM audio_album \
            WHERE title=?");
    bindString(title);

    if(step()) {
        id = getInt();
    }
    else {
        query("INSERT INTO audio_album (title, artist, date) VALUES (?, ?, ?)");
        bindString(title);
        bindInt(artist);
        bindString(date);
        step();
        id = last_insert_id();
    }

    albumCache[title] = id;
    return id;
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

    int artistId = getArtistId(meta->artist, meta->artist_sort);
    int albumId = getAlbumId(meta->album, meta->date, getArtistId(meta->albumartist, meta->albumartist_sort));

    query("INSERT INTO audio_track VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    bindInt(id);
    bindString(meta->title);
    bindInt(artistId);
    bindInt(albumId);
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
        id INTEGER PRIMARY KEY, \
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
        artist INTEGER, \
        album INTEGER, \
        tracknumber INTEGER, \
        length INTEGER, \
        bitrate INTEGER, \
        mbid_tid TEXT)";
    std::string audioAlbum = "CREATE TABLE IF NOT EXISTS audio_album ( \
        id INTEGER PRIMARY KEY, \
        title TEXT, \
        artist INTEGER, \
        date TEXT)";
    std::string artist = "CREATE TABLE IF NOT EXISTS artist ( \
        id INTEGER PRIMARY KEY, \
        name TEXT, \
        sortname TEXT)";
    std::string image = "CREATE TABLE IF NOT EXISTS image ( \
        id INTEGER PRIMARY KEY, \
        title TEXT)";

    insert(path);
    insert(resource);
    insert(type);
    insert(category);
    insert(audioTrack);
    insert(audioAlbum);
    insert(artist);
    insert(image);

    return true;
}

