#ifndef INDEXER_H
#define INDEXER_H

#include "Database.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>

class Indexer {
    public:
        Indexer() {};
        void addFolder(const std::string &filename);
    private:
        Database db;
        int added;
        int removed;
        int updated;

        void scanFolder(const boost::filesystem::path &folder, int parent);
        void updateFolder(const boost::filesystem::path &folder, int index);
        void updateFiles(int index, const std::vector<boost::filesystem::path> &files);
};

#endif
