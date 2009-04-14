#ifndef DATABASE_H
#define DATABASE_H

#include <boost/filesystem/path.hpp>
#include <string>

struct sqlite3;
class AudioFile;

class Database {
    public:
        bool init(const char *filename);
        static bool createTables();
        static int insertDir(const boost::filesystem::path &path, int parent, int type);
        static bool insertAudio(const boost::filesystem::path &file, const AudioFile *meta, int path);
    private:
        static sqlite3 *db;
        static int insertFile(const boost::filesystem::path &file, int path);
        static bool insert(std::string query);
};

#endif
