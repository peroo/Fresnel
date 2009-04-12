#include "Indexer.h"
#include "AudioFile.h"

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
    int count = 0;
    
    //Database::insertPath();
    parent = 1;
    std::cout << "Folder: \"" << dir.leaf() << "\"" << std::endl;

    fs::directory_iterator endIter;
    for(fs::directory_iterator iter(dir); iter != endIter; ++iter) {
        if(fs::is_directory(*iter)) {
            if(!fs::is_symlink(*iter)) {
                count += scanFolder(*iter, parent);
            }
        } else {
            if(scanFile(*iter)) count++;
        }
    }

    return count;
}

bool Indexer::scanFile(const fs::path &file)
{
    std::cout << "File: \"" << file.leaf() << "\" - ";

    AudioFile meta = AudioFile(&file);
    if(meta.readMeta()) {
        std::cout << "track #" << meta.tracknumber << " by artist \"" << meta.artist << "\"" << std::endl;
    }

    return true;
}
