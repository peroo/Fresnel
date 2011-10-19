#ifndef INDEXER_H
#define INDEXER_H

#include "Database.h"

#include <string>
#include <vector>
#include <list>


class Indexer {
    public:
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
        int      inotify_fd;

        // TODO: Could be turned into forward_list or potential savings
        std::list<Indexer::Directory> newdir_queue;
        std::list<Indexer::Directory> olddir_queue;
        std::list<ResFile> update_queue;
        std::list<ResFile> add_queue;
        std::map<std::string, ResFile> existing_files;

        void scanFolder(const Indexer::Directory &dir);
        void updateFolder(const Indexer::Directory &dir);

        bool initInotify();
        bool openDir(const std::string &dir, DIR *dirstream);
};

#endif
