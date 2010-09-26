#ifndef INDEXER_H
#define INDEXER_H

#include "Database.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>
#include <list>

class Indexer {
    public:
        Indexer() : added(0), removed(0), updated(0) {};
        void addFolder(const std::string &filename);

    private:
        Database db;
        int added;
        int removed;
        int updated;

        std::vector<ResFile> updateQueue;
        std::vector<ResFile> addQueue;

        void scanFolder(const boost::filesystem::path &folder, int parent);
        void updateFolder(const boost::filesystem::path &folder, int index);
        void updateFiles(int index, const std::list<boost::filesystem::path> &files);
};

#endif
