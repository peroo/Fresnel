#ifndef DATABASE_H
#define DATABASE_H

#include "SQLite.h"
#include "ResFile.h"

#include <string>
#include <map>
#include <list>


class Database : SQLite {
    public:
        Database() {}
        ~Database() {};

        bool createTables();
        int insertDir(const std::string &path, int parent);
        bool removeDir(int id);
        int getResourceType(int id);
        int getPathID(const std::string &path);
        std::string getPathByID(int id);
        int dirFileCount(const std::string &path);
        int getFileIDByName(int path_id, const std::string &filename);
        int32_t getPathIDByName(int32_t path_id, const std::string &name);

        int insertAudio(ResFile *file);
        void updateAudio(ResFile *file);
        int insertImage(ResFile *file);
        void updateImage(ResFile *file);
        void removeFile(int id);
        void movePath(int32_t from_id, int32_t to_id, const std::string &from_name, const std::string &to_name);
        void moveFile(int32_t file_id, int32_t path_id, const std::string &name);

        void startTransaction();
        void commitTransaction();

        ResFile getFileByID(int id);
        void getFiles(std::map<std::string, ResFile> &files);
        std::string getResourcePath(int id);
        std::map<std::string, int> getPathChildren(int index);
        std::list<int> getSubPaths(int id);

    private:
        int insertFile(ResFile *file);
        void updateFile(ResFile *file);
        int getArtistId(const std::string &artist,
                        const std::string &sortname);
        int getAlbumId(const std::string &title, 
                        const std::string &date, int artist);

        static std::map<std::string, int> artistCache;
        static std::map<std::string, int> albumCache;
};

#endif
