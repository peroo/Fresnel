#ifndef RESOURCE_H
#define RESOURCE_H

#include <boost/filesystem/path.hpp>
#include <string>

class Resource {
    public:
        bool init(int index);
        bool init(boost::filesystem::path path);

        virtual bool load(int index) = 0;
        virtual int read(int pos, int max, char *buffer) = 0;
    protected:
        int fileIndex;
        bool indexed;
        std::string extension;
        boost::filesystem::path path;

};

#endif
