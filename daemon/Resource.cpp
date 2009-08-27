#include "Resource.h"
#include "Database.h"

#include <boost/filesystem/convenience.hpp>
#include <iostream>
#include <vector>

namespace fs = boost::filesystem;

std::vector<Resource*> Resource::resources;

bool Resource::load(int index) {}
bool Resource::load() {}
bool Resource::done() {}
std::string Resource::getMimetype() {}
int Resource::getSize() {}
int Resource::read(int pos, int max, char *buffer) {}

Resource::~Resource() {
    std::vector<Resource*>::iterator iter;
    for(iter = resources.begin(); iter < resources.end(); ++iter) {
        if(this == *iter) {
            resources.erase(iter);
        }
    }
}

Resource* Resource::init(int index)
{
    // Caches last ten resources in case of new requests
    std::vector<Resource*>::iterator iter;
    for(iter = resources.begin(); iter < resources.end(); ++iter) {
        if(index == (*iter)->fileIndex) {
            Resource* r = *iter;
            resources.erase(iter);
            resources.insert(resources.begin(), r);

            std::cout << "Hooking resource: " << index << std::endl;
            delete this;
            return r;
        }
    }

    if(resources.size() > 5) {
        std::cout << "Dropping resource: " << (*(resources.end()-1))->fileIndex << std::endl;
        delete *(resources.end()-1);
        resources.pop_back();
    }
    resources.insert(resources.begin(), this);


    fileIndex = index;
    indexed = true;

    Database db = Database();
    path = db.getResourcePath(index);
    extension = fs::extension(path);

    std::cout << "Opening resource: " << path << std::endl;

    return this;
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

int Resource::staticReader(void *res, uint64_t pos, char *buffer, int max)
{
    return static_cast<Resource*>(res)->read(pos, max, buffer);
}
