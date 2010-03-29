#include "Indexer.h"
#include "Audio/Audio.h"
#include "Database.h"
#include "ResFile.h"
#include "Slingshot.h"

#include "inotify-cxx/inotify-cxx.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#include <vector>
#include <map>

namespace fs = boost::filesystem;

void Indexer::addFolder(const std::string &directory)
{
    fs::path dir(directory);

    if(!fs::exists(dir)) {
        Slingshot::Debug(1) << "Path \"" << dir.file_string() << "\" doesn't exist." << std::endl;
        return;
    } else if(!fs::is_directory(dir)) {
        Slingshot::Debug(1) << "Path \"" << dir.file_string() << "\" isn't a directory." << std::endl;
        return;
    } else if(fs::is_symlink(dir)) {
        Slingshot::Debug(1) << "Path \"" << dir.file_string() << "\" is a symlink. Skipping." << std::endl;
        return;
    } 

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
    
    Slingshot::Debug(3) << "Finished scanning \"" << directory << "\"." << std::endl <<
                            "------------" << std::endl <<
                            "Added: " << added << std::endl <<
                            "Updated: " << updated << std::endl <<
                            "Removed: " << removed << std::endl;
}

void Indexer::updateFolder(const fs::path &dir, int pathIndex)
{
    Slingshot::Debug(3) << "Updating " << dir.string() << std::endl;
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
        removed += db.dirFileCount(iter->first);
        db.removeDir(iter->second);
        Slingshot::Debug(3) << "Removing " << iter->first << std::endl;
    }
}

void Indexer::scanFolder(const fs::path &dir, int parent)
{
    Slingshot::Debug(3) << "Adding " << dir.string() << std::endl;
    int pathIndex = db.insertDir(dir, parent);
    std::vector<fs::path> files;

    fs::directory_iterator endIter;
    for(fs::directory_iterator iter(dir); iter!= endIter; ++iter) {
        if(fs::is_directory(iter->status())) {
            scanFolder(*iter, pathIndex);
        }
        else {
            files.push_back(*iter);
        }
    }

    for(auto iter = files.begin(); iter != files.end(); ++iter) {
        ResFile file(*iter, pathIndex);
        file.insert();
        ++added;
    }
}

void Indexer::updateFiles(int path, const std::vector<fs::path> &files) {
    std::map<std::string, ResFile> dbFiles = db.getFiles(path);

    for(auto iter = files.begin(); iter != files.end(); ++iter) {
        auto result = dbFiles.find(iter->leaf());
        if(result != dbFiles.end()) {
            if(fs::last_write_time(*iter) > result->second.modified()) {
                result->second.update();
                ++updated;
            }
            dbFiles.erase(result);
        }
        else {
            // TODO: catch exceptions
            ResFile file = ResFile(*iter, path);
            file.insert();
            ++added;
        }
    }

    for(auto iter = dbFiles.begin(); iter != dbFiles.end(); ++iter) {
        iter->second.remove();
        ++removed;
    }

}

