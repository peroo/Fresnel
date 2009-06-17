#ifndef DATABASE_H
#define DATABASE_H

#include "SQLite.h"

#include <boost/filesystem/path.hpp>
#include <string>

class Database : SQLite {
    public:
        Database() {}
        ~Database();

        bool createTables();
        int insertDir(const boost::filesystem::path &path, int parent, int type);
        int insertAudio(const boost::filesystem::path &file, int path);
        int insertImage(const boost::filesystem::path &file, int path);
        int getResourceType(int id);
        std::string getResourcePath(int id);

    private:
        int insertFile(const boost::filesystem::path &file, int path, int type);
};

#endif
