#ifndef RESFILE_H
#define RESFILE_H

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <ctime>
#include <string>

class Database;

class ResFile {
    public:
        ResFile() {}
        ResFile(const boost::filesystem::path &path, int pathIndex);
        ResFile(int id, int pathIndex, std::string pathName, 
                std::string name, int size, std::time_t modified, int type)
            : _id(id), _type(type), _pathIndex(pathIndex), _size(size), 
              _pathName(pathName), _name(name), _modified(modified)
            {}
        ~ResFile() {};

        bool supported();
        void update(Database* db);
        void insert(Database* db);
        void remove(Database* db);

        int id();
        int type();
        int pathId();
        int size();
        std::string name();
        std::string pathName();
        std::time_t modified();

    private:
        bool move(int path);
        void init(const boost::filesystem::path &path, int pathIndex);
        
        int _id;
        int _type;
        int _pathIndex;
        int _size;
        std::string _pathName;
        std::string _name;
        std::time_t _modified;
};

#endif
