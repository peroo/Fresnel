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
        ResFile(const struct stat &fileinfo, const std::string &name, int32_t path_id, const std::string &path);
        ResFile(int32_t id, time_t modified, int32_t path_id, std::string path, std::string name, int type)
            : _name(name), _id(id), _modified(modified), _path_id(path_id), _type(type), _path(path)
            {}
        ~ResFile() {};

        bool supported();
        void update(Database* db);
        void updateInfo(const struct stat &fileinfo);
        void updatePath(Database* db, int path_id, const std::string &name);
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
        void initInfo(const struct stat &fileinfo);
        std::string readExtension();
        
        std::string _name;
        int32_t _id;
        time_t _modified;
        int32_t _path_id;
        int _type;
        std::string _path;
        off_t _size;
};

#endif
