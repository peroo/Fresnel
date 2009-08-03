#ifndef DATABASE_H
#define DATABASE_H

#include "SQLite.h"

#include <boost/filesystem/path.hpp>
#include <string>
#include <map>

class Database : SQLite {
    public:
        Database() {}
        ~Database();

        bool createTables();
        int insertDir(const boost::filesystem::path &path, int parent, int type);
        bool removeDir(const boost::filesystem::path &path);
        int insertAudio(const boost::filesystem::path &file, int path);
        int insertImage(const boost::filesystem::path &file, int path);
        int getResourceType(int id);
        std::string getResourcePath(int id);

    private:
        int insertFile(const boost::filesystem::path &file, int path, int type);
        int getArtistId(std::string artist, std::string sortname);
        int getAlbumId(std::string title, std::string date, int artist);

        std::map<std::string, int> artistCache;
        std::map<std::string, int> albumCache;
};

#endif
