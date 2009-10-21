#include "Slingshot.h"
#include "SQLite.h"
#include "Database.h"
#include "JSDatabase.h"
#include "JavaScript.h"
#include "HttpRequest.h"
#include "Audio/Audio.h"
#include "Image/Image.h"
#include "Indexer.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <sqlite3.h>
#include <microhttpd.h>

#include <cstdio>
#include <stdint.h>
#include <unistd.h>
#include <iostream>

namespace fs = boost::filesystem;


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
    req->init();

    Database db = Database();

    if(req->module == RESOURCE) {
        Resource *res;
        int type = db.getResourceType(atoi(req->object.c_str()));
        if(type == AUDIO) {
            res = new Audio();
        }
        else if(type == IMAGE) {
            res = new Image();
        }

        res = res->init(atoi(req->object.c_str()));
        res->load();
        req->render(res);
    }
    else if(req->module == DATA) {
        JavaScript script = JavaScript();
        script.run(req);
        req->render(&script);
    }
    else if(req->module == STATIC_FILE) {
        fs::path path = fs::path("public_html/") / req->object;
        if(fs::exists(path)) {
            std::cout << method << ": " << path.string() << " - 200" << std::endl;
            req->render(path);
        }
        else {
            std::cout << method << ": " << path.string() << " - 404" << std::endl;
            req->fail(404);
        }
    }

    *ptr = req;

    return 1;
}

void connectionClosed(void *cls, struct MHD_Connection *connection, void **con_cls, MHD_RequestTerminationCode toe)
{
    (void) cls;
    (void) connection;

    std::cout << "Connection closed: ";
    if(toe == MHD_REQUEST_TERMINATED_COMPLETED_OK)
        std::cout << "Completed OK";
    else if(toe == MHD_REQUEST_TERMINATED_WITH_ERROR)
        std::cout << "Error";
    else if(toe == MHD_REQUEST_TERMINATED_TIMEOUT_REACHED)
        std::cout << "Timeout";
    std::cout << std::endl;

    delete (HttpRequest*)*con_cls;
}

bool Slingshot::init(bool test) {
	// Setup working dir
    base = fs::path(getenv("HOME")) / ".slingshot";
	if(!fs::exists(base)) {
		if(!create_directory(Slingshot::base))
            std::cout << "Unable to create directory: " << base.string() << std::endl;
			return false;
	}
	chdir(Slingshot::base.directory_string().c_str());

    std::string dbname = test ? "test.sqlite" : "db.sqlite"; 
    std::cout << dbname << std::endl;

	// Init SQLite
    if(!SQLite::selectDB(dbname.c_str())) {
        std::cout << "SQL initialization failed." << std::endl;
        return false;
	}
    Database db = Database();
    db.createTables();

	// Init MHD
	server = MHD_start_daemon(
        MHD_USE_DEBUG|MHD_USE_THREAD_PER_CONNECTION, 
        PORT, 
        false, 
        false, 
        requestCurrier, 
        false,
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

void Slingshot::StopServer()
{
    MHD_stop_daemon(server);
}

int main(int argc, char *argv[])
{
    (void) argv;

    bool test = argc > 1 ? true : false;
    Slingshot slingshot = Slingshot();
    slingshot.init(test);
    
    /*Image test = Image();
    test.open("002.jpg");
    std::cout << "Barcode detected: " << test.scanBarcode() << std::endl;
    test.resize(850, 442, BICUBIC);
    test.write("test.jpg", JPEG);*/

    Indexer index = Indexer();
    index.addFolder("/home/peroo/raid/flac_inc/");
    index.addFolder("/home/peroo/raid/inc/unsorted_music/");
    index.addFolder("/home/peroo/raid/inc/Flac/");
    index.addFolder("/home/peroo/raid/inc/incoming/");

    while(1) {
        sleep(600);
    }

    slingshot.StopServer();

    return 0;
}
