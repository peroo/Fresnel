#include "Indexer.h"
#include "Audio/Audio.h"
#include "Database.h"
#include "ResFile.h"
#include "Slingshot.h"

#include <list>
#include <map>
#include <sys/time.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>


void Indexer::addFolder(const std::string &directory)
{
    std::string dir_path;
    // Strip potential trailing slash
    if(directory.at(directory.size() - 1) == '/') {
        dir_path = directory.substr(0, directory.size() - 1);
    }
    else {
        dir_path = directory;;
    }

    DIR *dirent = opendir(dir_path.c_str());
    if(dirent == NULL) {
        int error = errno;
        if(error == ENOTDIR) {
            Slingshot::Debug(1) << "Path \"" << dir_path << "\" isn't a directory." << std::endl;
        }
        else if(error == ENOENT) {
            Slingshot::Debug(1) << "Path \"" << dir_path << "\" doesn't exist." << std::endl;
        }
        else if(error == EACCES) {
            Slingshot::Debug(1) << "Access denied to path: \"" << dir_path << "\"" << std::endl;
        }
        else {
            Slingshot::Debug(1) << "Unknown error accessing path: \"" << dir_path << "\"" << std::endl;
        }
        return;
    }

    db.startTransaction();

    struct timeval start, end;
    long mtime, seconds, useconds;
    gettimeofday(&start, NULL);

    int32_t path_id = db.getPathID(dir_path);
    if(path_id != -1) {
        Directory dir = {dir_path, path_id};
        olddir_queue.push_front(dir);
        while(olddir_queue.empty() == false) {
            dir = olddir_queue.front();
            olddir_queue.pop_front();
            updateFolder(dir);
        }
    }
    else {
        path_id = db.insertDir(dir_path, 0);
        Directory dir = {dir_path, path_id}; 
        newdir_queue.push_front(dir);
    }

    while(newdir_queue.empty() == false) {
        Directory dir = newdir_queue.front();
        newdir_queue.pop_front();
        scanFolder(dir);
    }

    for(auto iter = update_queue.begin(); iter != update_queue.end(); ++iter) {
        iter->update(&db);
        ++updated;
    }

    for(auto iter = add_queue.begin(); iter != add_queue.end(); ++iter) {
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

void Indexer::updateFolder(const Directory &dir)
{
    DIR *dirstream = opendir(dir.path.c_str());
    if(dirstream == NULL) {
        int error = errno;
        Slingshot::Debug(1) << "Unknown error code " << error << 
            "accessing path: \"" << dir.path << "\"" << std::endl;
        return;
    }

    std::map<std::string, int32_t> children = db.getPathChildren(dir.id);
    std::map<std::string, ResFile> files = db.getFiles(dir.id);

    struct dirent *entity;
    while((entity = readdir(dirstream)) != NULL) {
        std::string path = dir.path + '/' + entity->d_name;
        struct stat filestat;
        lstat(path.c_str(), &filestat);

        if(S_ISREG(filestat.st_mode)) {
            std::string name(entity->d_name);

            auto result = files.find(name);
            if(result != files.end()) {
                if(filestat.st_mtime > result->second.modified()) {
                    Slingshot::Debug(3) << "Updating " << path << std::endl;
                    result->second.updateInfo(filestat);
                    update_queue.push_front(result->second);
                }
                files.erase(result);
            }
            else {
                ResFile file(filestat, name, dir.id, dir.path);
                add_queue.push_front(file);
            }
        }
        else if(S_ISDIR(filestat.st_mode)) {
            if(entity->d_name[0] != '.' || (entity->d_name[1] != '\0' &&
                    (entity->d_name[1] != '.' || entity->d_name[2] != '\0'))) {
                auto result = children.find(path);
                if(result != children.end()) {
                    Directory olddir = {path, result->second};
                    olddir_queue.push_front(olddir);
                    children.erase(result);
                }
                else {
                    int32_t id = db.insertDir(path, dir.id);
                    Directory newdir = {path, id};
                    newdir_queue.push_front(newdir);
                }
            }
        }
    }

    for(auto iter = files.begin(); iter != files.end(); ++iter) {
        iter->second.remove(&db);
        ++removed;
    }

    for(auto iter = children.begin(); iter != children.end(); ++iter) {
        removed += db.dirFileCount(iter->first);
        db.removeDir(iter->second);
        Slingshot::Debug(3) << "Removing " << iter->first << std::endl;
    }
}

void Indexer::scanFolder(const Directory &dir)
{
    DIR *dirstream = opendir(dir.path.c_str());
    if(dirstream == NULL) {
        int error = errno;
        Slingshot::Debug(1) << "Unknown error code " << error << 
            "accessing path: \"" << dir.path << "\"" << std::endl;
        return;
    }

    struct dirent *entity;
    while((entity = readdir(dirstream)) != NULL) {
        std::string path = dir.path + '/' + entity->d_name;
        struct stat filestat;
        int result = lstat(path.c_str(), &filestat);
        if(result == -1) {
            int error = errno;
            Slingshot::Debug(1) << "Error #" << error << " in path: " << path << std::endl;
            return;
        }

        if(S_ISREG(filestat.st_mode)) {
            ResFile file(filestat, std::string(entity->d_name), dir.id, dir.path);
            add_queue.push_front(file);
        }
        else if(S_ISDIR(filestat.st_mode)) {
            if(entity->d_name[0] != '.' || (entity->d_name[1] != '\0' &&
                    (entity->d_name[1] != '.' || entity->d_name[2] != '\0'))) {
                int32_t id = db.insertDir(path, dir.id);
                Directory newdir = {path, id};
                newdir_queue.push_front(newdir);
            }
        }
    }
}
