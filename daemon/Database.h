#ifndef DATABASE_H
#define DATABASE_H

#include "SQLite.h"
#include "ResFile.h"

#include <boost/filesystem/path.hpp>
#include <string>
#include <map>


class Database : SQLite {
    public:
        Database() {}
        ~Database() {};

        bool createTables();
        int insertDir(const boost::filesystem::path &path, int parent);
        bool removeDir(int id);
        int getResourceType(int id);
        int getPath(int id);
        int getPath(std::string path);
        int dirFileCount(std::string path);
        ResFile getFile(int id);

        int insertAudio(ResFile *file);
        void updateAudio(ResFile *file);
        int insertImage(ResFile *file);
        void updateImage(ResFile *file);
        void removeFile(int id);

        void startTransaction();
        void commitTransaction();

        std::map<std::string, ResFile> getFiles(int pathId);
        std::string getResourcePath(int id);
        std::map<std::string, int> getPathChildren(int index);

    private:
        int insertFile(ResFile *file);
        void updateFile(ResFile *file);
        int getArtistId(std::string artist, std::string sortname);
        int getAlbumId(std::string title, std::string date, int artist);

        static std::map<std::string, int> artistCache;
        static std::map<std::string, int> albumCache;
};

#endif
