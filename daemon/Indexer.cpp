#include "Indexer.h"
#include "Audio/Audio.h"
#include "Database.h"
#include "ResFile.h"

#include "inotify-cxx/inotify-cxx.h"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#include <iostream>
#include <vector>
#include <map>

namespace fs = boost::filesystem;

void Indexer::addFolder(const std::string &directory)
{
    fs::path dir(directory);

    if(!fs::exists(dir)) {
        std::cout << "Path \"" << dir.file_string() << "\" doesn't exist." << std::endl;
        return;
    } else if(!fs::is_directory(dir)) {
        std::cout << "Path \"" << dir.file_string() << "\" isn't a directory." << std::endl;
        return;
    } else if(fs::is_symlink(dir)) {
        std::cout << "Path \"" << dir.file_string() << "\" is a symlink. Skipping." << std::endl;
        return;
    } 

    added = removed = updated = 0;

    // TODO: Investigate if transactions make a difference
    Database db = Database();
    db.startTransaction();

    int path = db.getPath(dir.string());
    if(path < 0) {
        scanFolder(dir, 0);
    }
    else {
        updateFolder(dir, path);
    }

    db.commitTransaction();
    
    std::cout << "Finished scanning \"" << directory << "\"." << std::endl;
    std::cout << "------------\nAdded: " << added << "\nUpdated: " << updated << "\nRemoved: " << removed << "\n" << std::endl;
}

void Indexer::updateFolder(const fs::path &dir, int pathIndex)
{
    std::map<std::string, int> children = db.getPathChildren(pathIndex);
    std::vector<fs::path> files;

    fs::directory_iterator endIter;
    for(fs::directory_iterator iter(dir); iter!= endIter; ++iter) {
        if(fs::is_directory(*iter)) {
            auto result = children.find(iter->string() + "/");
            if(result != children.end()) {
                updateFolder(*iter, result->second);
                children.erase(result);
            }
            else {
                scanFolder(*iter, pathIndex);
            }
        }
        else {
            files.push_back(*iter);
        }
    }
    
    updateFiles(pathIndex, files);

    for(auto iter = children.begin(); iter != children.end(); ++iter) {
        db.removeDir(iter->second);
    }
}

// TODO: Investigate whether deferring file insertion until after dir traversal might pay off
void Indexer::scanFolder(const fs::path &dir, int parent)
{
    int pathIndex = db.insertDir(dir, parent, NULL);

    fs::directory_iterator endIter;
    for(fs::directory_iterator iter(dir); iter!= endIter; ++iter) {
        if(fs::is_directory(*iter)) {
            scanFolder(*iter, pathIndex);
        }
        else {
            ResFile file;
            if(file.init(*iter, pathIndex)) {
                file.insert();
                ++added;
            }
        }
    }
}

void Indexer::updateFiles(int path, const std::vector<fs::path> &files) {
    std::map<std::string, ResFile> dbFiles = db.getFiles(path);

    for(auto iter = files.begin(); iter != files.end(); ++iter) {
        auto result = dbFiles.find(iter->leaf());
        if(result != dbFiles.end()) {
            if(fs::last_write_time(*iter) > result->second.modified()) {
                std::cout << result->first << " - " << fs::last_write_time(*iter) << " - " << result->second.modified() << std::endl;
                result->second.update();
                ++updated;
            }
            dbFiles.erase(result);
        }
        else {
            ResFile file = ResFile();
            if(file.init(*iter, path)) {
                file.insert();
                ++added;
            }
        }
    }

    for(auto iter = dbFiles.begin(); iter != dbFiles.end(); ++iter) {
        iter->second.remove();
        ++removed;
    }

}

