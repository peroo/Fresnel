#ifndef RESFILE_H
#define RESFILE_H

#include <ctime>
#include <string>
#include <dirent.h>
#include <sys/stat.h>

class Database;

class ResFile {
    public:
        ResFile() {}
        ResFile(const struct stat *fileinfo, const std::string &name, int32_t path_id, const std::string &path);
        ResFile(int32_t id, time_t modified, int type, std::string path)
            : _id(id), _modified(modified), _type(type), _path(path)
            {}
        ~ResFile() {};

        bool supported();
        void update(Database* db);
        void updateInfo(const struct stat *fileinfo);
        void insert(Database* db);
        void remove(Database* db);

        int32_t id();
        int type();
        int pathId();
        off_t size();
        std::string name();
        std::string path();
        time_t modified();

    private:
        bool move(int path);
        void initInfo(const struct stat *fileinfo);
        std::string readExtension();
        
        int32_t _id;
        off_t _size;
        std::string _name;
        int32_t _path_id;
        time_t _modified;
        int _type;
        std::string _path;
};

#endif
