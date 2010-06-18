#include "ResFile.h"
#include "Resource.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

namespace fs = boost::filesystem;

ResFile::ResFile(const fs::path &path, int pathIndex, Database *_db)
    : db(_db)
{
    init(path, pathIndex);
}

void ResFile::init(const fs::path &path, int pathIndex)
{
    _id = -1;
    _pathIndex = pathIndex;
    _pathName = path.branch_path().string();
    _name = path.leaf();
    _size = fs::file_size(path);
    _modified = fs::last_write_time(path);

    std::string ext = fs::extension(path);
    if(ext == ".flac" || ext == ".ogg" /*|| ext == ".mp3"*/) {
        _type = AUDIO;
    }
    else if(ext == ".jpeg" || ext == ".jpg" || ext == ".png") {
        _type = IMAGE;
    }
    else if(ext == ".avi" || ext == ".mp4" || ext == ".mkv" || ext == ".wmv" || ext == ".mpeg" || ext == ".mpg") {
        _type = VIDEO;
    }
}

void ResFile::update()
{
    int id = _id;
    init(fs::path(_pathName) / _name, _pathIndex);
    _id = id;

    switch(_type) {
        case AUDIO:
            db->updateAudio(this);
            break;
        case IMAGE:
            db->updateImage(this);
            break;
    }
}

void ResFile::insert()
{
    switch(_type) {
        case AUDIO:
            _id = db->insertAudio(this);
            break;
        case IMAGE:
            _id = db->insertImage(this);
            break;
        default:
            _id = -1;
    }
}

void ResFile::remove()
{
    db->removeFile(_id);
}

bool ResFile::move(int path)
{
    return false;
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
