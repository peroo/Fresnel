#include "ResFile.h"
#include "Database.h"
#include "Resource.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

namespace fs = boost::filesystem;

bool ResFile::init(int index)
{
    _id = index;
    return db.getFile(this);
}
bool ResFile::init(const fs::path &path, int pathIndex)
{
    _id = -1;
    _pathIndex = pathIndex;
    _pathName = path.branch_path().string();
    _name = path.leaf();
    _size = fs::file_size(path);
    _modified = fs::last_write_time(path);

    std::string ext = fs::extension(path);
    if(ext == ".flac" || ext == ".ogg" /*|| ext == ".mp3"*/) {
        _type = 0;
    }
    else if(ext == ".jpeg" || ext == ".jpg" || ext == ".png") {
        _type = 1;
    }
    else {
        return false;
    }

    return true;
}
bool ResFile::init(int id, int pathIndex, std::string pathName, std::string name, int size, std::time_t modified, int type)
{
    _id = id;
    _pathIndex = pathIndex;
    _pathName = pathName;
    _name = name;
    _size = _size;
    _modified = modified;
    _type = type;
    return true;
}

void ResFile::update()
{
    db.updateAudio(this);
}

void ResFile::insert()
{
    switch(_type) {
        case AUDIO:
            _id = db.insertAudio(this);
            break;
        case IMAGE:
            _id = db.insertImage(this);
            break;
        default:
            _id = -1;
    }
}

void ResFile::remove()
{
    db.removeFile(_id);
}

bool ResFile::move(int path)
{
}

std::time_t ResFile::modified()
{
    return _modified;
}
int ResFile::id()
{
    return _id;
}
int ResFile::pathId()
{
    return _pathIndex;
}
std::string ResFile::pathName()
{
    return _pathName;
}
int ResFile::size()
{
    return _size;
}
std::string ResFile::name()
{
    return _name;
}
int ResFile::type()
{
    return _type;
}
