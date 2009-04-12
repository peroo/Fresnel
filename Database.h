#ifndef DATABASE_H
#define DATABASE_H

#include <boost/filesystem/path.hpp>
#include <string>

struct sqlite3;

class Database {
    public:
        bool init(const char *filename);
        static bool createTables();
        static bool insertDir(const boost::filesystem::path &path, int parent, int type);
        static bool insertFile(const std::string filename, int path);
    private:
        static sqlite3 *db;
        static bool insert(std::string query);
};

#endif
