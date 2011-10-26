#include "Indexer.h"
#include "Audio/Audio.h"
#include "Database.h"
#include "ResFile.h"
#include "Fresnel.h"

#include <list>
#include <map>
#include <utility>
#include <sys/inotify.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <dirent.h>
#include <errno.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
#define NO_TIMEOUT -1
#define SHORT_TIMEOUT 100

void Indexer::addFolder(const std::string &directory)
{
    std::string dir_path;
    // Strip potential trailing slash
    if(directory.at(directory.size() - 1) == '/') {
        dir_path = directory.substr(0, directory.size() - 1);
    }
    else {
        dir_path = directory;
    }

    if(inotify_fd == 0) {
        if(!initInotify()) {
            return;
        }
    }

    // Check that dir exists and is openable
    DIR *dirstream = NULL;
    if(!openDir(dir_path, &dirstream)) {
        return;
    }
    closedir(dirstream);

    db.startTransaction();

    struct timeval start, end;
    long mtime, seconds, useconds;
    gettimeofday(&start, NULL);

    added = removed = updated = 0;

    // Fetch all indexed file instances from database
    db.getFiles(existing_files);

    int32_t path_id = db.getPathID(dir_path);
    root_index[path_id] = true;
    if(path_id != -1) {
        // Indexed dir, scan tree for updated and new files
        std::pair<int32_t, std::string> dir(path_id, dir_path);
        olddir_queue.push_back(dir);
        while(olddir_queue.empty() == false) {
            dir = olddir_queue.front();
            olddir_queue.pop_front();
            updateFolder(dir);
        }
    }
    else {
        // New dir, schedule full scan of tree
        newdir_queue.push_back(std::make_pair(0, dir_path));
    }

    // Scan all new dirs found
    while(newdir_queue.empty() == false) {
        scanFolder(newdir_queue.front());
        newdir_queue.pop_front();
    }

    // Remove all files no longer found
    for(auto iter = existing_files.begin(); iter != existing_files.end(); ++iter) {
        iter->second.remove(&db);
        ++removed;
    }

    db.commitTransaction();

    gettimeofday(&end, NULL);

    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
    
    Fresnel::Debug(3) << "Finished scanning \"" << directory << "\"." << std::endl <<
                            "------------" << std::endl <<
                            "Added: " << added << std::endl <<
                            "Updated: " << updated << std::endl <<
                            "Removed: " << removed << std::endl <<
                            "Time elapsed: " << mtime << "ms." << std::endl;
}

void Indexer::updateFolder(const std::pair<int32_t, std::string> &dir)
{
    int32_t dir_id = dir.first;
    std::string dir_path = dir.second;

    DIR *dirstream = NULL;
    if(!openDir(dir_path, &dirstream)) {
        return;
    }

    watchDir(dir_path, dir_id);
    std::map<std::string, int32_t> children = db.getPathChildren(dir_id);

    struct dirent *entity;
    while((entity = readdir(dirstream)) != NULL) {
        std::string path = dir_path + '/' + entity->d_name;
        struct stat filestat;
        if(!statFile(path, &filestat)) {
            // Stat failed, skip file
            continue;
        }

        if(S_ISREG(filestat.st_mode)) {
            std::stringstream stream;
            stream << dir_id << entity->d_name;

            auto result = existing_files.find(stream.str());
            if(result != existing_files.end()) {
                // Old file, check for modifications
                if(filestat.st_mtime > result->second.modified()) {
                    Fresnel::Debug(3) << "Updating " << path << std::endl;
                    result->second.updateInfo(filestat);
                    result->second.update(&db);
                }
                existing_files.erase(result);
            }
            else {
                insertFile(dir_id, std::string(entity->d_name));
            }
        }
        else if(S_ISDIR(filestat.st_mode)) {
            if(entity->d_name[0] != '.' || (entity->d_name[1] != '\0' &&
                    (entity->d_name[1] != '.' || entity->d_name[2] != '\0'))) {
                auto result = children.find(path);
                if(result != children.end()) {
                    // Old dir, update contents
                    olddir_queue.push_back(
                        std::make_pair(result->second, path)
                    );
                    children.erase(result);
                }
                else {
                    // New dir, do full scan
                    newdir_queue.push_back(std::make_pair(dir_id, path));
                }
            }
        }
    }
    closedir(dirstream);

    // Remove all old dirs not found in scan
    for(auto iter = children.begin(); iter != children.end(); ++iter) {
        removed += db.dirFileCount(iter->first);
        db.removeDir(iter->second);
        Fresnel::Debug(3) << "Removing " << iter->first << std::endl;
    }
}

void Indexer::scanFolder(const std::pair<int32_t, std::string> &dir)
{
    int32_t parent_id = dir.first;
    std::string dir_path = dir.second;

    DIR *dirstream = NULL;
    if(!openDir(dir_path, &dirstream)) {
        Fresnel::Debug(3) << "Failed to open path \"" << 
            dir_path << "\"" << std::endl;
        return;
    }

    int32_t dir_id = db.insertDir(dir_path, parent_id);
    watchDir(dir_path, dir_id);

    struct dirent *entity;
    while((entity = readdir(dirstream)) != NULL) {
        std::string path = dir_path + '/' + entity->d_name;
        struct stat filestat;
        if(!statFile(path, &filestat)) {
            // Stat failed, skip file
            continue;
        }

        if(S_ISREG(filestat.st_mode)) {
            insertFile(dir_id, std::string(entity->d_name));
        }
        else if(S_ISDIR(filestat.st_mode)) {
            if(entity->d_name[0] != '.' || (entity->d_name[1] != '\0' &&
                    (entity->d_name[1] != '.' || entity->d_name[2] != '\0'))) {
                newdir_queue.push_back(std::make_pair(dir_id, path));
            }
        }
    }
    closedir(dirstream);
}

bool Indexer::openDir(const std::string &dir_path, DIR **dirstream)
{
    *dirstream = opendir(dir_path.c_str());
    if(*dirstream == NULL) {
        Fresnel::Debug(1) << "Path \"" << dir_path << "\"";
        switch(errno) {
            case ENOTDIR:
                Fresnel::Debug(1) <<  " isn't a directory.";
                break;
            case ENOENT:
                Fresnel::Debug(1) << " doesn't exist.";
                break;
            case EACCES:
                Fresnel::Debug(1) << ": Access denied";
                break;
            default:
                Fresnel::Debug(1) << ": Unknown error.";
        }
        Fresnel::Debug(1) << std::endl;
        return false;
    }
    return true;
}

bool Indexer::statFile(const std::string &path, struct stat *filestat)
{
    int result = lstat(path.c_str(), filestat);
    if(result == -1) {
        switch(errno) {
            case ENOENT:
                Fresnel::Debug(1) << "File not found: ";
                break;
            case EIO:
                Fresnel::Debug(1) << "Error reading file: ";
                break;
            case EACCES:
            case ELOOP:
            case ENAMETOOLONG:
            case EOVERFLOW:
            case ENOTDIR:
                Fresnel::Debug(1) << "Unknown error reading file: ";
        } 
        Fresnel::Debug(1) << path << std::endl;
        return false;
    }
    return true;
}

void Indexer::watchFolders()
{
    int status = pthread_create(&inotify_reader, NULL, &Indexer::startWatcher, this);
    if(status != 0) {
        Fresnel::Debug(1) << "A critical error occurred when attempting to initialize inotify thread." << std::endl;
    }
}

bool Indexer::initInotify()
{
    inotify_fd = inotify_init();
    if(inotify_fd == -1) {
        Fresnel::Debug(1) << "Inotify initialization failed: ";
        switch(errno) {
            case EMFILE:
                Fresnel::Debug(1) << "The user limit on the total number of inotify instances has been reached.";
                break;
            case ENFILE:
                Fresnel::Debug(1) << "The system limit on the total number of file descriptors has been reached.";
                break;
            case ENOMEM:
                Fresnel::Debug(1) <<  "Insufficient kernel memory is available.";
        }
        Fresnel::Debug(1) << std::endl;
        return false;
    }
    return true;
}


void Indexer::readInotify()
{
    char buffer[BUF_LEN];
    int length, i;
    int timeout = NO_TIMEOUT;

    int efd = epoll_create1(0);
    struct epoll_event event;
    struct epoll_event events;
    event.data.fd = inotify_fd;
    event.events = EPOLLIN;
    epoll_ctl(efd, EPOLL_CTL_ADD, inotify_fd, &event);

    for(;;) {
        int count = epoll_wait(efd, &events, 1, timeout);
        i = 0;

        if(count > 0) {
            length = read(inotify_fd, buffer, sizeof(buffer));
            while(i < length) {
                struct inotify_event *event = 
                    (struct inotify_event *) &buffer[i];
                processIEvent(event);
                i += EVENT_SIZE + event->len;
            }
            timeout = SHORT_TIMEOUT;
        }
        else {
            // 100ms without events, so clean up orphan events and reset timeout
            processOrphanMoves();
            timeout = NO_TIMEOUT;
        }
    }
}

void Indexer::processIEvent(struct inotify_event *event)
{
    bool is_dir = event->mask & IN_ISDIR;
    if(event->mask & IN_CLOSE_WRITE) {
        if(!is_dir) {
            Fresnel::Debug(1) << "Inotify: " << "CLOSE_WRITE" << std::endl;
            int32_t path_id = watch_index[event->wd];
            std::string name = std::string(event->name);
            int file_id = db.getFileIDByName(path_id, name);

            if(file_id > 0) {
                // Existing file, check if modified
                if(updateFile(file_id)) {
                    Fresnel::Debug(1) << "Inotify: " << 
                        name << " updated." << std::endl;
                }
            }
            else {
                // New file, insert in db
                if(insertFile(path_id, name)) {
                    Fresnel::Debug(1) << "Inotify: " << 
                        name << " inserted." << std::endl;
                }
            }
        }
    }
    else if(event->mask & IN_CREATE) {
        if(is_dir) {
            Fresnel::Debug(1) << "Inotify: " << "CREATE" << std::endl;
            // New dir, insert in db and watch
            int32_t parent_id = watch_index[event->wd];
            std::string name = std::string(event->name);
            insertTree(parent_id, name);
            Fresnel::Debug(1) << "Inotify: " << 
                name << " registered." << std::endl;
        }
    }
    else if (event->mask & IN_MOVE) {
        processMove(event);
    }
    else if(event->mask & IN_MOVE_SELF)
    {
        int32_t id = watch_index[event->wd];
        // Only process if path is a root node
        if(root_index.find(id) != root_index.end()) {
            removeTree(watch_index[event->wd]);
            root_index.erase(id);
            Fresnel::Debug(1) << "Inotify: Root path #" << 
                id << " removed." << std::endl;
        }
    }
    else if(event->mask & IN_DELETE_SELF) {
        Fresnel::Debug(1) << "Inotify: " << "DELETE_SELF" << std::endl;
        int32_t id = watch_index[event->wd];
        removeTree(id);
        if(root_index.find(id) != root_index.end()) {
            root_index.erase(id);
        }
        Fresnel::Debug(1) << "Inotify: Path #" << 
            id << " deleted." << std::endl;
    }
    else if(event->mask & IN_DELETE) {
        if(!is_dir) {
            Fresnel::Debug(1) << "Inotify: " << "DELETE" << std::endl;
            int32_t path_id = watch_index[event->wd];
            std::string name = std::string(event->name);
            int file_id = db.getFileIDByName(path_id, name);
            ResFile file = db.getFileByID(file_id);
            file.remove(&db);
            Fresnel::Debug(1) << "Inotify: File " << 
                name << " deleted." << std::endl;
        }
    }
    else if(event->mask & IN_UNMOUNT) {
        // LOG
        Fresnel::Debug(1) << "IN_UNMOUNT" << event->name << std::endl;
    }
}

void Indexer::processMove(struct inotify_event *event)
{
    Move move;
    if(move_cache.find(event->cookie) != move_cache.end()) {
        move = move_cache[event->cookie];
    }

    if(event->mask & IN_MOVED_FROM) {
        move.from_id = watch_index[event->wd];
        move.from_name = std::string(event->name);
    }
    else {
        move.to_id = watch_index[event->wd];
        move.to_name = std::string(event->name);
    }

    if(move.to_id != 0 && move.from_id != 0) {
        if(move.is_dir)
            movePath(event->cookie);
        else
            moveFile(event->cookie);
    }
    else {
        move.is_dir = (event->mask & IN_ISDIR);
        move_cache[event->cookie] = move;

    }
}

void Indexer::movePath(uint32_t cookie)
{
    Move move = move_cache[cookie];
    db.movePath(move.from_id, move.to_id, move.from_name, move.to_name);
    move_cache.erase(cookie);
    Fresnel::Debug(1) << "Inotify: Path \"" << move.from_name << 
        "\" moved to \"" << move.to_name << "\"." << std::endl;
}

void Indexer::moveFile(uint32_t cookie)
{
    Move move = move_cache[cookie];

    int file_id = db.getFileIDByName(move.from_id, move.from_name);
    ResFile file = db.getFileByID(file_id);
    file.updatePath(&db, move.to_id, move.to_name);

    move_cache.erase(cookie);
    Fresnel::Debug(1) << "Inotify: File \"" << move.from_name << 
        "\" moved to \"" << move.to_name << "\"." << std::endl;
}

void Indexer::processOrphanMoves()
{
    Move move;
    for(auto iter = move_cache.begin(); iter != move_cache.end(); iter++) {
        move = iter->second;
        if(move.is_dir) {
            if(move.from_id == 0) {
                // Dir moved into tree
                insertTree(move.to_id, move.to_name);
                Fresnel::Debug(1) << "Inotify: Path " << 
                    move.to_name << " moved in." << std::endl;
            }
            else {
                // Dir moved out of tree
                int32_t id = db.getPathIDByName(move.from_id, move.from_name);
                removeTree(id);
                Fresnel::Debug(1) << "Inotify: Path " << 
                    move.from_name << " moved out." << std::endl;
            }
        }
        else {
            if(move.from_id == 0) {
                // File moved into tree
                insertFile(move.to_id, move.to_name);
                Fresnel::Debug(1) << "Inotify: File " << 
                    move.to_name << " moved in." << std::endl;
            }
            else {
                // File moved out of tree
                int32_t id = db.getFileIDByName(move.from_id, move.from_name);
                ResFile file = db.getFileByID(id);
                file.remove(&db);
                Fresnel::Debug(1) << "Inotify: file " << 
                    move.from_name << " moved out." << std::endl;
            }
        }
    }

    move_cache.clear();
}

void Indexer::watchDir(const std::string &path, int32_t path_id)
{
    if(inotify_fd < 1) {
        if(!initInotify())
            return;
    }

    int descriptor = inotify_add_watch(inotify_fd, path.c_str(), 
            IN_CREATE|IN_MOVE|IN_CLOSE_WRITE|IN_DELETE|IN_DELETE_SELF|IN_MOVE_SELF|IN_UNMOUNT);
    if(descriptor == -1) {
        Fresnel::Debug(1) << "Error adding watch: ";
        switch(errno) {
            case EACCES:
                Fresnel::Debug(1) << "Read access to the given file is not permitted.";
                break;
            case EBADF:
                Fresnel::Debug(1) << "The given inotify descriptor is not valid.";
                break;
            case EFAULT:
                Fresnel::Debug(1) << "Pathname points outside of the process's accessible address space.";
                break;
            case EINVAL:
                Fresnel::Debug(1) << "The given event mask contains no legal events; or fd is not an inotify file descriptor.";
                break;
            case ENOENT:
                Fresnel::Debug(1) << "A directory component in pathname does not exist or is a dangling symbolic link.";
                break;
            case ENOMEM:
                Fresnel::Debug(1) << "Insufficient kernel memory was available.";
                break;
            case ENOSPC:
                Fresnel::Debug(1) << "The user limit on the total number of inotify watches was reached or the kernel failed to allocate a needed resource.";
                break;
        }
        Fresnel::Debug(1) << std::endl;
    }
    else {
        watch_index[descriptor] = path_id;
        watch_index[path_id] = descriptor;
    }
}

void* Indexer::startWatcher(void *pointer)
{
    Indexer *indexer = static_cast<Indexer*>(pointer);
    indexer->readInotify();
    return 0;
}


bool Indexer::insertFile(int32_t path_id, const std::string &name)
{
    struct stat filestat;
    std::string path = db.getPathByID(path_id);
    if(statFile(path + '/' + name, &filestat)) {
        ResFile file = ResFile(filestat, name, path_id, path);
        if(file.supported()) {
            file.insert(&db);
            return true;
        }
    }
    return false;
}

void Indexer::insertTree(int32_t parent_id, const std::string &name)
{
    std::string parent_path = db.getPathByID(parent_id);
    std::string path = parent_path + '/' + name;

    newdir_queue.push_back(std::make_pair(parent_id, path));
    while(newdir_queue.empty() == false) {
        scanFolder(newdir_queue.front());
        newdir_queue.pop_front();
    }
}

void Indexer::removeTree(int32_t root_id)
{
    int path_id;

    std::list<int32_t> paths;
    paths.push_back(root_id);
    while(paths.size() > 0) {
        path_id = paths.front();
        paths.pop_front();
        std::list<int32_t> sublist = db.getSubPaths(path_id);
        paths.splice(paths.end(), sublist);
        inotify_rm_watch(inotify_fd, reverse_watch_index[path_id]);

        watch_index.erase(reverse_watch_index[path_id]);
        reverse_watch_index.erase(path_id);
    }
    // Recursive remove
    db.removeDir(root_id);
}


bool Indexer::updateFile(int32_t file_id)
{
    struct stat filestat;
    ResFile file = db.getFileByID(file_id);
    std::string path = file.path() + '/' + file.name();
    if(statFile(path, &filestat)) {
        if(filestat.st_mtime > file.modified()) {
            file.updateInfo(filestat);
            file.update(&db);
            return true;
        }
    }
    return false;
}
