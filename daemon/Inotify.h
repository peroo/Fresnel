#ifndef INOTIFY_H
#define INOTIFY_H

#include <string>
#include <map>

class Inotify {
    public:
        void watchDir(const std::string &path, int32_t path_id);
        void readInotify();
        bool init();
        void* startWatcher(void *pointer);
    private:
        int inotify_fd;
        std::map<int, int32_t> watch_index;

};

#endif
