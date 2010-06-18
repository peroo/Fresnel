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
        void fetchAlbum(std::string album);
        void scanColumns();

};

#endif