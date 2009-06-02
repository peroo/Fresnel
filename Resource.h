#ifndef RESOURCE_H
#define RESOURCE_H

#include <boost/filesystem/path.hpp>
#include <string>

enum res_type {
    AUDIO,
    IMAGE,
    VIDEO
};

class Resource {
    public:
        bool init(int index);
        bool init(boost::filesystem::path path);
        int static staticReader(void *res, uint64_t pos, char *buffer, int max);

        virtual bool load(int index) = 0;
        virtual bool load() = 0;
        virtual int read(int pos, int max, char *buffer) = 0;
        virtual std::string getMimetype() = 0;
    protected:
        int fileIndex;
        bool indexed;
        std::string extension;
        boost::filesystem::path path;

};

#endif
