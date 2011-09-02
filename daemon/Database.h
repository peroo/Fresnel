#ifndef DATABASE_H
#define DATABASE_H

#include "SQLite.h"
#include "ResFile.h"

#include <string>
#include <map>


class Database : SQLite {
    public:
        Database() {}
        ~Database() {};

        bool createTables();
        int insertDir(const std::string &path, int parent);
        bool removeDir(int id);
        int getResourceType(int id);
        int getPathID(const std::string &path);
        int dirFileCount(const std::string &path);
        //ResFile getFile(int id);

        int insertAudio(ResFile *file);
        void updateAudio(ResFile *file);
        int insertImage(ResFile *file);
        void updateImage(ResFile *file);
        void removeFile(int id);

        void startTransaction();
        void commitTransaction();

        void getFiles(std::map<std::string, ResFile> &files);
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
