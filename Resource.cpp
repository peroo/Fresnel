#include "Resource.h"

#include <boost/filesystem/convenience.hpp>

namespace fs = boost::filesystem;

bool Resource::load(int index) {}
bool Resource::load() {}
std::String Resource::getMimetype() {}
int Resource::read(int pos, int max, char *buffer) {}

bool Resource::init(int index)
{
    fileIndex = index;
    indexed = true;

    Database db = Database();
    path = db.getResourcePath(index);
    extension = fs::extension(path);

    return true;
}

bool Resource::init(boost::filesystem::path _path)
{
    fileIndex = -1;
    indexed = false;
    path = _path;

    extension = fs::extension(path);
    for(int i = 0; i < extension.size(); ++i) {
        extension[i] = tolower(extension[i]);
    }

    return true;
}

int Resource::staticReader(void *res, int pos, char *buffer, int max)
{
    return static_cast<Resource*>(res)->read(pos, max, buffer);
}
