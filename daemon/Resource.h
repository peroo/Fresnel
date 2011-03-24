#ifndef RESOURCE_H
#define RESOURCE_H

//#include <boost/stdint.hpp>
#include <boost/filesystem/path.hpp>
#include <string>
#include <vector>

enum res_type {
    AUDIO,
    IMAGE,
    VIDEO,
    UNKNOWN
};

class Resource {
    public:
        Resource() : loaded(false) {}
        virtual ~Resource();
        Resource* init(int index);
        bool init(boost::filesystem::path path);
        ssize_t static staticReader(void *res, uint64_t pos, char *buffer, size_t max);

        virtual bool load(int index) = 0;
        virtual bool load() = 0;
        virtual int read(unsigned int pos, unsigned int max, char *buffer) = 0;
        virtual std::string getMimetype() = 0;
        virtual int getSize() = 0;
        virtual bool done() = 0;

        int fileIndex;
        static std::vector<Resource*> resources;
    protected:
        bool indexed;
        bool loaded;
        std::string extension;
        boost::filesystem::path path;
    private:

};

#endif
