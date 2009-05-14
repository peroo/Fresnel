#include "Resource.h"

#include <boost/filesystem/convenience.hpp>

namespace fs = boost::filesystem;

bool Resource::init(int index)
{
    fileIndex = index;
    indexed = true;
    path = NULL;
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
