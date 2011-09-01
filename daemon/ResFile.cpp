#include "ResFile.h"
#include "Resource.h"
#include "Database.h"

#include <algorithm>

ResFile::ResFile(const struct stat *fileinfo, const std::string &name, 
        int32_t path_id, const std::string &path)
        : _name(name), _path_id(path_id), _path(path)
{
    initInfo(fileinfo);
}

void ResFile::initInfo(const struct stat *fileinfo)
{
    std::string ext = readExtension();
    if(ext == "flac" || ext == "ogg" /*|| ext == ".mp3"*/) {
        _type = AUDIO;
    }
    else if(ext == "jpeg" || ext == "jpg" || ext == "png") {
        _type = IMAGE;
    }
    else if(ext == "avi" || ext == "mp4" || ext == "mkv" || ext == "wmv" || 
            ext == "mpeg" || ext == "mpg") {
        _type = VIDEO;
    }
    else {
        // Unsupported file, return
        _type = UNKNOWN;
        return;
    }

    _id = -1;
    _size = fileinfo->st_size;
    _modified = fileinfo->st_mtime;
}

std::string ResFile::readExtension() {
    size_t index = _name.find_last_of('.');
    if(index != std::string::npos) {
        std::string ext = _name.substr(index + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }

    std::string ext;
    return ext;
}

bool ResFile::supported()
{
    if(_type == UNKNOWN)
        return false;
    else
        return true;
}

void ResFile::updateInfo(const struct stat *fileinfo) {
    _size = fileinfo->st_size;
    _modified = fileinfo->st_mtime;
}

void ResFile::update(Database* db)
{
    switch(_type) {
        case AUDIO:
            db->updateAudio(this);
            break;
        case IMAGE:
            db->updateImage(this);
            break;
    }
}

void ResFile::insert(Database* db)
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

void ResFile::remove(Database* db)
{
    db->removeFile(_id);
}

bool ResFile::move(int path)
{
    (void) path;
    return false;
}

time_t ResFile::modified()
{
    return _modified;
}
int ResFile::id()
{
    return _id;
}
int32_t ResFile::pathId()
{
    return _path_id;
}
off_t ResFile::size()
{
    return _size;
}
std::string ResFile::name()
{
    return _name;
}
std::string ResFile::path()
{
    return _path;
}
int ResFile::type()
{
    return _type;
}
