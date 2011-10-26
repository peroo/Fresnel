#ifndef INDEXER_H
#define INDEXER_H

#include "Database.h"
#include "Inotify.h"

#include <string>
#include <vector>
#include <list>


class Indexer {
    public:
        void addFolder(const std::string &filename);
        void watchFolders();
        void readInotify();

        struct Directory {
            std::string path;
            int32_t id;
        };

    private:
        Database db;
        uint32_t added;
        uint32_t removed;
        uint32_t updated;
        pthread_t inotify_reader;
        int inotify_fd;

        class Move {
            public:
                Move() : from_id(0), to_id(0), is_dir(false) {}
                int32_t from_id;
                int32_t to_id;
                std::string from_name;
                std::string to_name;
                bool is_dir;
        };

        // TODO: Could be turned into forward_list or potential savings
        std::list<Indexer::Directory> newdir_queue;
        std::list<Indexer::Directory> olddir_queue;
        std::map<std::string, ResFile> existing_files;
        std::map<int, int32_t> watch_index;
        std::map<int32_t, int> reverse_watch_index;
        std::map<int32_t, bool> root_index;
        std::map<uint32_t, Move> move_cache; 

        void scanFolder(const Indexer::Directory &dir);
        void updateFolder(const Indexer::Directory &dir);
        bool statFile(const std::string &path, struct stat *filestat);
        bool insertFile(int32_t path_id, const std::string &name);
        bool insertDir(int32_t parent_id, const std::string &name);
        void removeTree(int32_t root_id);
        bool updateFile(int32_t file_id);
        bool moveFile(int32_t file_id, int path_id);

        bool initInotify();
        static void* startWatcher(void *pointer);
        void watchDir(const std::string &path, int32_t path_id);
        void processIEvent(struct inotify_event *event);
        void processOrphanMoves();
        void processMove(struct inotify_event *event);
        void movePath(uint32_t cookie);
        void moveFile(uint32_t cookie);

        bool openDir(const std::string &dir, DIR **dirstream);
};

#endif
