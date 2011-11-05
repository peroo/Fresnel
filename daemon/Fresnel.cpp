#include "Fresnel.h"
#include "SQLite.h"
#include "Database.h"
#include "HttpRequest.h"
#include "DBQuery.h"
#include "Audio/Audio.h"
#include "Image/Image.h"
#include "Indexer.h"

#include <sqlite3.h>
#include <microhttpd.h>

#include <cstdio>
#include <unistd.h>
#include <iostream>
#include <fstream>

//#include "video.h"

int requestCurrier(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr)
{
    (void) cls;
    (void) version;
    (void) upload_data;
    (void) upload_data_size;

    static int dummy;

    //TODO: Investigate parsing headers early and returning MHD_NO in some cases.
    if(&dummy != *ptr) {
        *ptr = &dummy;
        return MHD_YES;
    }

    HttpRequest *req = new HttpRequest(connection, url, method);

    if(req->module == RESOURCE) {
        Database db = Database();
        Resource *res = NULL;

        int id = atoi(req->object.c_str());
        int type = db.getResourceType(id);

        if(type == -1) {
            req->fail(MHD_HTTP_NOT_FOUND);
        }
        else {
            if(type == AUDIO) {
                res = new Audio();
            }
            else if(type == IMAGE) {
                res = new Image();
            }

            res = res->init(id);
            res->load();
            req->render(res);
        }
    }
    else if(req->module == DATA) {
        DBQuery query = DBQuery(req);
        query.parse();
    }
    else if(req->module == STATIC_FILE) {
        std::string path = std::string("public_html/") + '/' + req->object;
        FILE *file;
        if((file = fopen(path.c_str(), "r")) != NULL) {
            fclose(file);
            std::cout << method << ": " << path << " - 200" << std::endl;
            req->render(path);
        }
        else {
            std::cout << method << ": " << path << " - 404" << std::endl;
            req->fail(MHD_HTTP_NOT_FOUND);
        }
    }
    else if(req->module == INDEX) {
        req->render(std::string("public_html/index.html"));
    }

    *ptr = req;

    return 1;
}

void connectionClosed(void *cls, struct MHD_Connection *connection, void **con_cls, MHD_RequestTerminationCode toe)
{
    (void) cls;
    (void) connection;

    std::cout << "Connection closed -- ";
    if(toe == MHD_REQUEST_TERMINATED_COMPLETED_OK)
        std::cout << "Completed OK";
    else if(toe == MHD_REQUEST_TERMINATED_WITH_ERROR)
        std::cout << "Error";
    else if(toe == MHD_REQUEST_TERMINATED_TIMEOUT_REACHED)
        std::cout << "Timeout";
    std::cout << std::endl;

    delete (HttpRequest*)*con_cls;
}

bool Fresnel::init() {
	// Setup working dir
    base = std::string(getenv("HOME")) + '/' +  ".fresnel";
    DIR *dir;
	if((dir = opendir(base.c_str())) == NULL) {
        chdir(getenv("HOME"));
		if(!mkdir(".fresnel", S_IRWXU)) {
            std::cout << "Unable to create directory: " << base << std::endl;
			return false;
        }
	}
    else {
        closedir(dir);
    }
	chdir(base.c_str());

    Database db = Database();
    db.createTables();

	// Init MHD
	server = MHD_start_daemon(
        MHD_USE_DEBUG|MHD_USE_THREAD_PER_CONNECTION,
        PORT,
        NULL,
        NULL,
        requestCurrier,
        NULL,
        MHD_OPTION_NOTIFY_COMPLETED,
        connectionClosed,
        NULL,
        MHD_OPTION_END
    );
	if(!server)
        return false;

	std::cout << "Init successful." << std::endl;
    return true;
}

void Fresnel::StopServer()
{
    MHD_stop_daemon(server);
}

int main(int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    Fresnel fresnel = Fresnel();
    fresnel.init();

    Indexer index = Indexer();
    //index.addFolder("/home/peroo/raid/Music/inc/Flac/");
    index.addFolder("/home/peroo/raid/Music/lol/");
    index.watchFolders();

    while(1) {
        sleep(600);
    }

    fresnel.StopServer();

    return 0;
}

std::ostream& Fresnel::Debug(int level)
{
    std::clog.clear(level <= 5 ? std::ios_base::goodbit : std::ios_base::badbit);
    return std::clog;
}
