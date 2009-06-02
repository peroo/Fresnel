#ifndef INDEXER_H
#define INDEXER_H

#include "Database.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

class Indexer {
    public:
        Indexer();
        int addFolder(const std::string &filename);
    private:
        Database *db;
        int scanFolder(const boost::filesystem::path &folder, int parent);
        bool scanFile(const boost::filesystem::path &file, int parent);
};

#endif
