#ifndef RESFILE_H
#define RESFILE_H

#include "Database.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <ctime>
#include <string>

class ResFile {
    public:
        ResFile() {}
        ~ResFile() {};

        bool init(int index);
        bool init(const boost::filesystem::path &path, int pathIndex);
        bool init(int index, int pathIndex, std::string pathName, std::string name, int size, std::time_t modified, int type);
        void update();
        void insert();
        void remove();

        int id();
        int type();
        int pathId();
        int size();
        std::string name();
        std::string pathName();
        std::time_t modified();

    private:
        bool move(int path);
        
        Database db;
        int _type;
        int _id;
        int _pathIndex;
        std::string _pathName;
        std::string _name;
        int _size;
        std::time_t _modified;
};

#endif
