#include "Inotify.h"

#include "Indexer.h"
#include "Fresnel.h"

#include <sys/inotify.h>

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

bool Inotify::init()
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


void Inotify::readInotify()
{
    char buffer[BUF_LEN];
    int length, i;

    while(1) {
        length = read(inotify_fd, buffer, BUF_LEN);
        Fresnel::Debug(1) << "Inotify: ";
        i = 0;
        while(i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i];
            bool isDir = event->mask & IN_ISDIR;
            if(event->mask & IN_CLOSE_WRITE) {
                if(!isDir) {
                    int32_t path_id = watch_index[event->wd];
                    std::string name = std::string(event->name, event->len);
                    int file_id = db.getFileIDByName(path_id, name);
                    struct stat filestat;

                    if(file_id > 0) {
                        // Existing file, check if modified
                        ResFile file = db.getFileByID(file_id);
                        std::string path = file.path() + '/' + file.name();
                        if(Indexer::statFile(path, &filestat)) {
                            if(filestat.st_mtime > file.modified()) {
                                file.updateInfo(filestat);
                                Fresnel::Debug(1) << name << " updated." << std::endl;
                            }
                        }
                    }
                    else {
                        // New file, insert in db
                        std::string path = db.getPathByID(path_id);
                        if(Indexer::statFile(path, &filestat)) {
                            ResFile file = ResFile(filestat, name, path_id, path);
                            file.insert(&db);
                            Fresnel::Debug(1) << name << " inserted." << std::endl;
                        }
                    }
                }
            }
            else if(event->mask & IN_CREATE) {
                if(isDir) {
                    // New dir, insert in db and watch
                    int32_t parent_id = watch_index[event->wd];
                    std::string parent_path = db.getPathByID(parent_id);
                    std::string name = std::string(event->name, event->len);
                    std::string dir_path = parent_path + '/' + name;

                    int32_t path_id = db.insertDir(name, parent_id);
                    watchDir(dir_path, path_id);
                    Directory dir = {dir_path, path_id}; 
                    scanFolder(dir);
                    Fresnel::Debug(1) << name << " registered." << std::endl;
                }
            }
            else if (event->mask & IN_MOVE_SELF) {
                // PUSH ON STACK IF DIR
                Fresnel::Debug(1) << "IN_MOVE_SELF" << event->name << std::endl;
            }
            else if (event->mask & IN_MOVED_FROM) {
                // PUSH ON STACK IF FILE
                Fresnel::Debug(1) << "IN_MOVED_FROM:" << event->name << std::endl;
            }
            else if(event->mask & IN_MOVED_TO) {
                // CHECK STACK, MOVE OR INSERT
                Fresnel::Debug(1) << "IN_MOVED_TO" << event->name << std::endl;
            }
            else if(event->mask & IN_DELETE_SELF) {
                // NUKE IF FOLDER
                Fresnel::Debug(1) << "IN_DELETE_SELF" << event->name << std::endl;
            }
            else if(event->mask & IN_DELETE) {
                // NUKE IF FILE
                Fresnel::Debug(1) << "IN_DELETE" << event->name << std::endl;
            }
            else if(event->mask & IN_UNMOUNT) {
                // LOG
                Fresnel::Debug(1) << "IN_UNMOUNT" << event->name << std::endl;
            }
            i += EVENT_SIZE + event->len;
        }
    }
}

void Inotify::watchDir(const std::string &path, int32_t path_id)
{
    if(inotify_fd < 1) {
        if(!init())
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
            case ENOMEM:
                Fresnel::Debug(1) << "Insufficient kernel memory was available.";
                break;
            case ENOSPC:
                Fresnel::Debug(1) << "The user limit on the total number of inotify watches was reached or the kernel failed to allocate a needed resource.";
        }
        Fresnel::Debug(1) << std::endl;
    }
    else {
        watch_index[descriptor] = path_id;
    }
}

void* Inotify::startWatcher(void *pointer)
{
    Inotify *inotify = static_cast<Inotify*>(pointer);
    inotify->readInotify();
    return 1;
}

