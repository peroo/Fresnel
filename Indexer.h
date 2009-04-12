#ifndef INDEXER_H
#define INDEXER_H



#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

class Indexer {
    public:
        Indexer();
        int addFolder(const std::string &filename);
    private:
        int scanFolder(const boost::filesystem::path &folder, int parent);
        bool scanFile(const boost::filesystem::path &file);
};

#endif