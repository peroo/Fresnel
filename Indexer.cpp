#include "Indexer.h"
#include "Audio/Audio.h"
#include "Database.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>

namespace fs = boost::filesystem;

Indexer::Indexer()
{
}

int Indexer::addFolder(const std::string &directory)
{
    fs::path dir = fs::path(directory);

    if(!fs::exists(dir)) {
        std::cout << "Path \"" << dir.file_string() << "\" doesn't exist." << std::endl;
        return 0;
    } else if(!fs::is_directory(dir)) {
        std::cout << "Path \"" << dir.file_string() << "\" isn't a directory." << std::endl;
        return 0;
    } else if(fs::is_symlink(dir)) {
        std::cout << "Path \"" << dir.file_string() << "\" is a symlink." << std::endl;
        return 0;
    } 
    
    return scanFolder(dir, 0);

    return 0;
}

int Indexer::scanFolder(const fs::path &dir, int parent)
{
    int count = 0, dirID;
    
    dirID = Database::insertDir(dir, parent, NULL);
    std::cout << "Folder: \"" << dir.leaf() << "\"" << std::endl;

    fs::directory_iterator endIter;
    for(fs::directory_iterator iter(dir); iter != endIter; ++iter) {
        if(fs::is_directory(*iter)) {
            if(!fs::is_symlink(*iter)) {
                count += scanFolder(*iter, dirID);
            }
        } else {
            if(scanFile(*iter, dirID))
                count++;
        }
    }

    return count;
}

bool Indexer::scanFile(const fs::path &file, int path)
{
    std::cout << "File: \"" << file.leaf() << "\" - " << std::endl;

    Database::insertAudio(file, path);

    return true;
}
