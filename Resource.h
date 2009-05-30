#ifndef RESOURCE_H
#define RESOURCE_H

#include <boost/filesystem/path.hpp>
#include <string>

enum res_type {
    Audio,
    Image,
    Video
};

class Resource {
    public:
        bool init(int index);
        bool init(boost::filesystem::path path);

        virtual bool load(int index) = 0;
        virtual bool load() = 0;
        virtual int read(int pos, int max, char *buffer) = 0;
        int static staticReader(void *res, int pos, char *buffer, int max);
        virtual std::string getMimetype() = 0;
    protected:
        int fileIndex;
        bool indexed;
        std::string extension;
        boost::filesystem::path path;

};

#endif
