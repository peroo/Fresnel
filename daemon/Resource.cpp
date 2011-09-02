#include "Resource.h"
#include "Database.h"

#include <iostream>
#include <vector>
#include <algorithm>

std::vector<Resource*> Resource::resources;

Resource::~Resource() {
    std::vector<Resource*>::iterator iter;
    for(iter = resources.begin(); iter < resources.end(); ++iter) {
        if(this == *iter) {
            resources.erase(iter);
        }
    }
}

void Resource::readExtension() {
    size_t index = path.find_last_of('.');
    if(index != std::string::npos) {
        extension = path.substr(index + 1);
        std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
    }
    else {
        extension = "";
    }
}

Resource* Resource::init(int index)
{
    // Caches last ten resources in case of new requests
    for(auto iter = resources.begin(); iter != resources.end(); ++iter) {
        if(index == (*iter)->file_id) {
            Resource* r = *iter;
            resources.erase(iter);
            resources.insert(resources.begin(), r);

            std::cout << "Hooking resource: " << index << std::endl;
            delete this;
            return r;
        }
    }

    if(resources.size() >= 5) {
        std::cout << "Dropping resource" << std::endl;
        delete resources.back();
    }
    resources.insert(resources.begin(), this);


    file_id = index;
    indexed = true;

    Database db = Database();
    path = db.getResourcePath(index);
    readExtension();

    std::cout << "Opening resource: " << path << std::endl;

    return this;
}

bool Resource::init(std::string _path)
{
    file_id = -1;
    indexed = false;
    path = _path;
    readExtension();

    return true;
}

ssize_t Resource::staticReader(void *res, uint64_t pos, char *buffer, size_t max)
{
    return static_cast<Resource*>(res)->read(pos, (unsigned int)max, buffer);
}
