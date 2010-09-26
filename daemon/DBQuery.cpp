#include "DBQuery.h"
#include "sqlite3.h"
#include "HttpRequest.h"
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
    query(
        "SELECT A.id, title, date, name, sortname \
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

std::string escapeStr(std::string str)
{
    int pos = str.find_first_of('"', 0);
    while(pos != std::string::npos) {
        str.insert(pos, "\\");
        pos = str.find_first_of('"', pos+2);
    }
    return str;
}

std::string DBQuery::processQuery()
{
    bool res = step();

    if(res) {
        scanColumns();
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

