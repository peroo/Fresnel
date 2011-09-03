#include "DBQuery.h"
#include "sqlite3.h"
#include "HttpRequest.h"
#include <xspf/Xspf.h>
#include <iostream>
#include <sstream>

void DBQuery::parse()
{
    if(req->object == "album") {
        if(req->parameters.size() == 0) {
            fetchAlbums();
        }
        else {
            fetchAlbum(req->parameters[0]);
        }
    }
    else if(req->object == "images") {
        fetchImages(req->parameters[0]);
    }
    else if(req->object == "path") {
        fetchPaths();
    }
    else if(req->object == "m3u") {
        if(req->parameters[0] == "album") {
            fetchM3UAlbum(req->parameters[1]);
        }
        else if(req->parameters[0] == "artist"){
            fetchM3UArtist(req->parameters[1]);
        }
        else if(req->parameters[0] == "random") {
            fetchM3URandom();
        }
    }
    else if(req->object == "xspf") {
       if(req->parameters[0] == "album") {
           fetchXSPFAlbum(req->parameters[1]);
       }
       else if(req->parameters[0] == "artist"){
           fetchXSPFArtist(req->parameters[1]);
       }
       else if(req->parameters[0] == "random") {
           fetchXSPFRandom();
       }
       else if(req->parameters[0] == "date"){
           fetchXSPFDate(req->parameters[1]);
       }
    }
    else {
        req->fail(MHD_HTTP_NOT_FOUND);
    }
}

void DBQuery::fetchPaths()
{
    query(
        "SELECT * \
        FROM path");

    return req->render(processQuery(), "application/json");
}

void DBQuery::fetchImages(std::string pathId)
{
    int id = atoi(pathId.c_str());
    query(
        "SELECT id, filename \
         FROM resource \
         WHERE type=1 \
         AND path_id=?");

    bindInt(id);

    return req->render(processQuery(), "application/json");
}

void DBQuery::fetchAlbums()
{
    // TODO: Using REPLACE as hack for proper escaping
    query(
        "SELECT A.id, REPLACE(title, '\\', '-') AS title, date, name, sortname \
         FROM audio_album A \
         JOIN artist T ON A.artist=T.id \
         ORDER BY sortname, title, date");

    req->render(processQuery(), "application/json");
}

void DBQuery::fetchAlbum(std::string album)
{
    int id = atoi(album.c_str());
    if(id == 0) {
        req->fail(404);
    }
    else {
        query(
            "SELECT R.id,title,tracknumber, length, bitrate, path_id, name \
             FROM resource R \
             JOIN audio_track A USING (id) \
             JOIN artist T ON artist=T.id \
             WHERE type=0 \
             AND A.album = ? \
             ORDER BY tracknumber ASC");

        bindInt(id);
        req->render(processQuery(), "application/json");
    }
}

void DBQuery::fetchM3UAlbum(std::string album)
{
    query(
        "SELECT R.id,T.title,I.name AS artist,A.title AS album,tracknumber, length \
         FROM resource R \
         JOIN audio_track T USING (id) \
         JOIN audio_album A ON album=A.id \
         JOIN artist I ON T.artist=I.id \
         WHERE type=0 \
         AND A.title LIKE ? \
         ORDER BY A.title, R.path_id, tracknumber ASC");

    bindString("%" + album + "%");

    req->render(processM3U(), "audio/x-mpegurl");
}

void DBQuery::fetchXSPFAlbum(std::string album)
{
    query(
        "SELECT R.id,T.title,I.name AS artist,A.title AS album,tracknumber, length \
         FROM resource R \
         JOIN audio_track T USING (id) \
         JOIN audio_album A ON album=A.id \
         JOIN artist I ON T.artist=I.id \
         WHERE type=0 \
         AND A.title LIKE ? \
         ORDER BY A.title, R.path_id, tracknumber ASC");

    bindString("%" + album + "%");

    req->render(processXSPF(), "application/xspf+xml");
}

void DBQuery::fetchM3UArtist(std::string artist)
{
    query(
        "SELECT R.id,T.title,I.name AS artist,A.title AS album,tracknumber, length \
         FROM resource R \
         JOIN audio_track T USING (id) \
         JOIN audio_album A ON album=A.id \
         JOIN artist I ON T.artist=I.id \
         WHERE type=0 \
         AND I.name LIKE ? \
         ORDER BY A.date ASC, A.title, R.path_id, tracknumber ASC");

    bindString("%" + artist + "%");

    req->render(processM3U(), "audio/x-mpegurl");
}

void DBQuery::fetchXSPFArtist(std::string artist)
{
    query(
        "SELECT R.id,T.title,I.name AS artist,A.title AS album,tracknumber, length \
         FROM resource R \
         JOIN audio_track T USING (id) \
         JOIN audio_album A ON album=A.id \
         JOIN artist I ON T.artist=I.id \
         WHERE type=0 \
         AND I.name LIKE ? \
         ORDER BY A.date ASC, A.title, R.path_id, tracknumber ASC");

    bindString("%" + artist + "%");

    req->render(processXSPF(), "application/xspf+xml");
}

void DBQuery::fetchM3URandom()
{
    query(
        "SELECT R.id,T.title,I.name AS artist,A.title AS album,tracknumber, length \
         FROM resource R \
         JOIN audio_track T USING (id) \
         JOIN audio_album A ON album=A.id \
         JOIN artist I ON T.artist=I.id \
         WHERE type=0 \
         ORDER BY RANDOM() \
         LIMIT 50");

    req->render(processM3U(), "audio/x-mpegurl");
}

void DBQuery::fetchXSPFRandom()
{
    query(
        "SELECT R.id,T.title,I.name AS artist,A.title AS album,tracknumber, length \
         FROM resource R \
         JOIN audio_track T USING (id) \
         JOIN audio_album A ON album=A.id \
         JOIN artist I ON T.artist=I.id \
         WHERE type=0 \
         ORDER BY RANDOM() \
         LIMIT 50");

    req->render(processXSPF(), "application/xspf+xml");
}

void DBQuery::fetchXSPFDate(std::string date)
{
    query(
        "SELECT R.id,T.title,I.name AS artist,A.title AS album,tracknumber, length \
         FROM resource R \
         JOIN audio_track T USING (id) \
         JOIN audio_album A ON album=A.id \
         JOIN artist I ON T.artist=I.id \
         WHERE type=0 \
         AND A.date LIKE ? \
         ORDER BY A.date ASC, A.title, tracknumber ASC");

    bindString("%" + date + "%");

    req->render(processXSPF(), "application/xspf+xml");
}

std::string escapeStr(std::string str)
{
    size_t pos = str.find_first_of('"', 0);
    while(pos != std::string::npos) {
        str.insert(pos, "\\");
        pos = str.find_first_of('"', pos+2);
    }
    return str;
}

std::string DBQuery::processM3U()
{
    bool res = step();

    if(!res) {
        return "error";
    }

    std::ostringstream output;
    output << "#EXTM3U" << std::endl << std::endl;
    do {
        int id = getInt();
        std::string title = getString();
        std::string artist = getString();
        std::string album = getString();
        std::string tracknumber = getString();
        int length = getInt();

        output << "#EXTINF:" << length << "," << artist << " - " << title << std::endl;
        output << "http://129.241.122.115:9996/resource/" << id << "/" << tracknumber << ".ogg" <<  std::endl << std::endl;

    } while(step());
    return output.str();

}

std::string DBQuery::processXSPF()
{
    bool res = step();

    if(!res) {
        return "error";
    }

    Xspf::XspfIndentFormatter formatter = Xspf::XspfIndentFormatter();
    Xspf::XspfWriter *writer = Xspf::XspfWriter::makeWriter(formatter, NULL);

    do {
        int id = getInt();
        std::string title = getString();
        std::string artist = getString();
        std::string album = getString();
        std::string tracknumber = getString();
        int length = getInt();

        std::stringstream loc;
        loc << id << ".oga";

        Xspf::XspfTrack track = Xspf::XspfTrack();
        track.giveAlbum(album.c_str(), true);
        track.giveCreator(artist.c_str(), true);
        track.giveTitle(title.c_str(), true);
        // TODO: setTrackNum expects integer, while we have string
        //track.setTrackNum(tracknumber);
        track.setDuration(length * 1000);
        track.giveAppendLocation((std::string("http://129.241.138.184:9996/resource/") + loc.str()).c_str(), true);
        writer->addTrack(track);
    } while(step());

    int bytes = 500000;
    char *output = new char[bytes];
    writer->writeMemory(output, bytes);
    std::string out(output);
    delete[] output;
    delete writer;
    return out;
}
std::string DBQuery::processQuery()
{
    // TODO: Bad queries merely return string error instead of 504
    bool res = step();

    if(res) {
        scanColumns();
    }
    else {
        return "error";
    }

    std::ostringstream output;
    output << "[";
    bool asd = true;
    int count = ColCount();
    do {
        if(asd) {
            output << "{";
            asd = false;
        }
        else {
            output << ",{";
        }
        for(int i=0; i < count; ++i) {
            output << (i==0 ? "\"" : ",\"") << columnNames[i] << "\":";
            switch(columnTypes[i]) {
                case SQLITE_TEXT:
                    output << "\"" << escapeStr(getString()) << "\"";
                    break;
                case SQLITE_INTEGER:
                    output << getString();
                    break;
                case SQLITE_FLOAT:
                    output << getFloat();
                    break;
                case SQLITE_NULL:
                    getVoid();
                    break;
            }
        }
        output << "}" << std::endl;
    } while(step());
    output << "]";
    return output.str();
}

void DBQuery::scanColumns()
{
    int count = ColCount();
    for(int i=0; i < count; ++i) {
        columnNames.push_back(ColName(i));
        columnTypes.push_back(ColType(i));
    }
}
