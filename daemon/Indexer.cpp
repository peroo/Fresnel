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

#include <list>
#include <map>
#include <sys/time.h>

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

    db.startTransaction();

    struct timeval start, end;
    long mtime, seconds, useconds;
    gettimeofday(&start, NULL);

    int path = db.getPath(dir.string());
    if(path < 0) {
        scanFolder(dir, 0);
    }
    else {
        updateFolder(dir, path);
    }

    for(auto iter = updateQueue.begin(); iter != updateQueue.end(); ++iter) {
        iter->update(&db);
        ++updated;
    }

    for(auto iter = addQueue.begin(); iter != addQueue.end(); ++iter) {
        if(iter->supported()) {
            iter->insert(&db);
            ++added;
        }
    }

    db.commitTransaction();

    gettimeofday(&end, NULL);

    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
    
    Slingshot::Debug(3) << "Finished scanning \"" << directory << "\"." << std::endl <<
                            "------------" << std::endl <<
                            "Added: " << added << std::endl <<
                            "Updated: " << updated << std::endl <<
                            "Removed: " << removed << std::endl <<
                            "Time elapsed: " << mtime << "ms." << std::endl;
}

void Indexer::updateFolder(const fs::path &dir, int pathIndex)
{
    std::map<std::string, int> children = db.getPathChildren(pathIndex);
    std::list<fs::path> files;

    fs::directory_iterator endIter;
    for(fs::directory_iterator iter(dir); iter!= endIter; ++iter) {
        if(fs::is_directory(*iter)) {
            auto result = children.find(iter->string());
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
    //Slingshot::Debug(3) << "Adding " << dir.string() << std::endl;
    int pathIndex = db.insertDir(dir, parent);

    fs::directory_iterator endIter;
    for(fs::directory_iterator iter(dir); iter!= endIter; ++iter) {
        if(fs::is_directory(iter->status())) {
            scanFolder(*iter, pathIndex);
        }
        else {
            ResFile file(*iter, pathIndex);
            addQueue.push_back(file);
        }
    }
}

void Indexer::updateFiles(int path, const std::list<fs::path> &files) {
    std::map<std::string, ResFile> dbFiles = db.getFiles(path);

    for(auto iter = files.begin(); iter != files.end(); ++iter) {
        auto result = dbFiles.find(iter->leaf());
        if(result != dbFiles.end()) {
            if(fs::last_write_time(*iter) > result->second.modified()) {
                Slingshot::Debug(3) << "Updating " << iter->string() << std::endl;
                updateQueue.push_back(result->second);
            }
            dbFiles.erase(result);
        }
        else {
            ResFile file = ResFile(*iter, path);
            addQueue.push_back(file);
        }
    }

    for(auto iter = dbFiles.begin(); iter != dbFiles.end(); ++iter) {
        iter->second.remove(&db);
        ++removed;
    }
}

