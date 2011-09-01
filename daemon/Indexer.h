#ifndef INDEXER_H
#define INDEXER_H

#include "Database.h"

#include <string>
#include <vector>
#include <list>


class Indexer {
    public:
        Indexer() : added(0), removed(0), updated(0) {};
        void addFolder(const std::string &filename);

        struct Directory {
            std::string path;
            int32_t id;
        };

    private:
        Database db;
        uint32_t added;
        uint32_t removed;
        uint32_t updated;

        // TODO: Could be turned into forward_list or potential savings
        std::list<Indexer::Directory> newdir_queue;
        std::list<Indexer::Directory> olddir_queue;
        std::list<ResFile> update_queue;
        std::list<ResFile> add_queue;

        void scanFolder(const Indexer::Directory &dir);
        void updateFolder(const Indexer::Directory &dir);
};

#endif
