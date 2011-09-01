#include "Database.h"
#include "ResFile.h"
#include "Resource.h"
#include "Audio/Metadata.h"

#include <sqlite3.h>

#include <iostream>
#include <string>
#include <map>

std::map<std::string, int> Database::artistCache;
std::map<std::string, int> Database::albumCache;

void Database::startTransaction()
{
    query("BEGIN TRANSACTION");
    step();
}
void Database::commitTransaction()
{
    query("COMMIT TRANSACTION");
    step();
}

int Database::getResourceType(int id)
{
    query("SELECT type FROM resource WHERE id = ?");
    bindInt(id);
    bool ret = step();

    return ret ? getInt() : -1;
}

std::string Database::getResourcePath(int id)
{
    query("SELECT path, filename FROM resource JOIN path USING (path_id) WHERE id = ?");
    bindInt(id);
    step();

    std::string path = getString();
    path += '/' + getString();

    return path;
}

int Database::getArtistId(std::string name, std::string sortname)
{
    int id = artistCache[name];
    if(id)
        return id;

    query("SELECT id, sortname FROM artist \
            WHERE name LIKE ?");
    bindString(name);

    if(step()) {
        id = getInt();
        std::string sort = getString();
        // Update artist if the first occurrence didn't contain a sortname
        if(sort.empty() && !sortname.empty()) {
            query("UPDATE artist SET sortname=? WHERE id=?");
            bindString(sortname);
            bindInt(id);
            step();
            artistCache[name] = id;
        }
    }
    else {
        query("INSERT INTO artist (name, sortname) VALUES (?, ?)");
        bindString(name);
        bindString(sortname);
        step();
        id = last_insert_id();
        if(!sortname.empty()) {
            artistCache[name] = id;
        }
    }

    return id;
}

int Database::getAlbumId(std::string title, std::string date, int artist)
{
    int id = albumCache[title + date];
    if(id)
        return id;

    query("SELECT id FROM audio_album \
            WHERE title=? \
            AND date=?");
    bindString(title);
    bindString(date);

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

    albumCache[title+date] = id;
    return id;
}

int Database::insertAudio(ResFile *file)
{
    //TODO: Assumes album_artist tag exists
    int id = insertFile(file);
    if(id <= 0) {
        std::cout << "File not inserted properly, aborting audio insert" << std::endl;
        return -1;
    }

    Metadata meta = Metadata();
    meta.loadData(file->path() + "/" + file->name());

    int artistId = getArtistId(meta.artist, meta.artist_sort);
    int albumId = getAlbumId(meta.album, meta.date, getArtistId(meta.albumartist, meta.albumartist_sort));

    query("INSERT INTO audio_track VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
    bindInt(id);
    bindString(meta.title);
    bindInt(artistId);
    bindInt(albumId);
    bindString(meta.tracknumber.c_str());
    bindInt(meta.length);
    bindInt(meta.bitrate);
    bindString(meta.musicbrainz_trackid);
    step();

    return last_insert_id();
}
void Database::updateAudio(ResFile *file)
{
    updateFile(file);

    Metadata meta = Metadata();
    meta.loadData(file->path() + '/' + file->name());

    int artistId = getArtistId(meta.artist, meta.artist_sort);
    int albumId = getAlbumId(meta.album, meta.date, getArtistId(meta.albumartist, meta.albumartist_sort));

    query("UPDATE audio_track SET title=?, artist=?, album=?, tracknumber=?, length=?, bitrate=?, mbid_tid=? WHERE id=?");
    bindString(meta.title);
    bindInt(artistId);
    bindInt(albumId);
    bindString(meta.tracknumber.c_str());
    bindInt(meta.length);
    bindInt(meta.bitrate);
    bindString(meta.musicbrainz_trackid);
    bindInt(file->id());
    step();
}

int Database::insertImage(ResFile *file)
{
    int id = insertFile(file);
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

void Database::updateImage(ResFile *file)
{
    updateFile(file);
    return;
}

int Database::insertDir(const std::string &path, int parent)
{
    query("INSERT INTO path (path, parent) VALUES (?, ?)");

    bindString(path);
    bindInt(parent);

    step();

    return last_insert_id();
}

bool Database::removeDir(int id)
{
    // TODO: Child directories are *not* removed due to possible recursive limitations of sqlite
    query("DELETE FROM path WHERE path_id=?");
    bindInt(id);
    if(step())
        return true;
    else
        return false;
}

int Database::insertFile(ResFile *file)
{
    query("INSERT INTO resource (path_id, filename, type, size, modified) VALUES (?, ?, ?, ?, ?)");

    bindInt(file->pathId());
    bindString(file->name());
    bindInt(file->type());
    bindInt(file->size());
    bindInt64(file->modified());

    step();

    return last_insert_id();
}

void Database::updateFile(ResFile *file)
{
    query("UPDATE resource SET size=?, modified=? WHERE id=?");
    bindInt(file->size());
    bindInt64(file->modified());
    bindInt(file->id());

    step();
    return;
}
void Database::removeFile(int id)
{
    query("DELETE FROM resource WHERE id=?");
    bindInt(id);
    step();
    return;
}

int Database::getPathID(const std::string &path)
{
    query ("SELECT path_id FROM path WHERE path=?");

    bindString(path);

    if(step())
        return getInt();
    else
        return -1;
}

int Database::dirFileCount(const std::string &path)
{
    query("SELECT COUNT(*) FROM resource NATURAL JOIN path WHERE path LIKE ?");

    bindString(path + "%");
    step();
    return getInt();
}

std::map<std::string, int> Database::getPathChildren(int32_t id)
{
    query("SELECT path, path_id FROM path WHERE parent=?");

    bindInt(id);

    std::map<std::string, int32_t> paths;
    while(step()) {
        paths[getString()] = getInt();
    }

    return paths;
}


std::map<std::string, ResFile> Database::getFiles(int32_t path_id)
{
    std::map<std::string, ResFile> files;

    query("SELECT id, filename, modified, type, path FROM resource JOIN path USING(path_id) WHERE path_id=?");
    bindInt(path_id);

    while(step()) {
        int id = getInt();
        std::string filename = getString();
        time_t modified(getInt64());
        int type(getInt());
        std::string path = getString();

        files[filename] = ResFile(id, modified, type, path);
    }

    return files;
}

/*ResFile Database::getFile(int id)
{
    query("SELECT path, path.path_id, filename, size, modified, type FROM resource JOIN path USING path_id WHERE id=?");
    bindInt(id);

    if(!step()) {
        throw("DB Error");
    }

    std::string path = getString();
    int pathId = getInt();
    std::string filename = getString();
    int size = getInt();
    std::time_t modified(getInt());
    int type = getInt();

    ResFile file(id, pathId, path, filename, size, modified, type);
    return file;
}*/

bool Database::createTables()
{
    // TODO: Remove "IF NOT EXISTS" in favour of proper db checking
    std::string path = "CREATE TABLE IF NOT EXISTS path ( \
        path_id INTEGER PRIMARY KEY, \
        path TEXT NOT NULL, \
        parent INTEGER)";

    std::string resource = "CREATE TABLE IF NOT EXISTS resource ( \
        id INTEGER PRIMARY KEY, \
        path_id INTEGER REFERENCES path(path_id), \
        filename TEXT, \
        size INTEGER, \
        type INTEGER, \
        hash TEXT, \
        modified INTEGER)";

    std::string type = "CREATE TABLE IF NOT EXISTS type ( \
        id INTEGER PRIMARY KEY, \
        extension TEXT NOT NULL, \
        category INTEGER)";

    std::string category = "CREATE TABLE IF NOT EXISTS category ( \
        id INTEGER PRIMARY KEY, \
        name TEXT)";
    
    std::string audioTrack = "CREATE TABLE IF NOT EXISTS audio_track ( \
        id INTEGER PRIMARY KEY REFERENCES resource(id), \
        title TEXT, \
        artist INTEGER, \
        album INTEGER, \
        tracknumber TEXT, \
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
        id INTEGER PRIMARY KEY REFERENCES resource(id), \
        title TEXT)";

    std::string pathTrigger = "CREATE TRIGGER Path_CascadeDelete \
        AFTER DELETE ON path \
        FOR EACH ROW \
        BEGIN \
            DELETE FROM resource \
            WHERE path_id = OLD.path_id; \
        END;";

    std::string resAudioTrigger = "CREATE TRIGGER Audio_CascadeDelete \
        AFTER DELETE ON resource \
        FOR EACH ROW \
        BEGIN \
            DELETE FROM audio_track \
            WHERE id = OLD.id; \
        END;";

    std::string resImageTrigger = "CREATE TRIGGER Image_CascadeDelete \
        AFTER DELETE ON resource \
        FOR EACH ROW \
        BEGIN \
            DELETE FROM image \
            WHERE id = OLD.id; \
        END;";

    insert(path);
    insert(resource);
    insert(type);
    insert(category);
    insert(audioTrack);
    insert(audioAlbum);
    insert(artist);
    insert(image);

    insert(pathTrigger);
    insert(resAudioTrigger);
    insert(resImageTrigger);

    return true;
}

