#ifndef DBQUERY_H
#define DBQUERY_H

#include "SQLite.h"
#include <vector>

class HttpRequest;

class DBQuery : SQLite {
    public:
        DBQuery(HttpRequest *_req) : req(_req) {}
        ~DBQuery() {}
        void parse();
    private:
        HttpRequest *req;
        std::vector<int> columnTypes;
        std::vector<std::string> columnNames;

        std::string processQuery();
        void fetchAlbums();
        void fetchPaths();
        void fetchM3UAlbum(std::string album);
        void fetchM3UArtist(std::string artist);
        void fetchM3URandom();
        std::string processM3U();
        void fetchAlbum(std::string album);
        void fetchImages(std::string pathId);
        void scanColumns();

};

#endif
