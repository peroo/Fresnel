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

int requestCurrier(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, unsigned int *upload_data_size, void **con_cls)
{
    HttpRequest *req = new HttpRequest(connection, url, method);
    req->init();

    Database *db = new Database();

    if(req->module == RESOURCE) {
        Resource *res;
        int type = db->getResourceType(atoi(req->object.c_str()));
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
        std::cout << req->object << std::endl;
        std::cout << path.string() << std::endl;
        if(fs::exists(path)) {
            std::cout << path.string() << std::endl;
            req->render(path);
        }
        else {
            std::cout << path.string() << " - 404" << std::endl;
            req->fail(404);
        }
    }

    return 1;
}

bool Slingshot::init() {
	// Setup working dir
    base = fs::path(getenv("HOME")) / ".slingshot";
	if(!fs::exists(base)) {
		if(!create_directory(Slingshot::base))
            std::cout << "Unable to create directory: " << base.string() << std::endl;
			return false;
	}
	chdir(Slingshot::base.directory_string().c_str());

	// Init SQLite
    if(!SQLite::selectDB("db.sqlite")) {
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
        MHD_OPTION_END
    );
	if(!server)
        return false;

	std::cout << "Init successful." << std::endl;
}

void Slingshot::StopServer()
{
    MHD_stop_daemon(server);
}

int main(void)
{
    Slingshot slingshot = Slingshot();
    slingshot.init();
    
    /*Image test = Image();
    test.open("002.jpg");
    std::cout << "Barcode detected: " << test.scanBarcode() << std::endl;
    test.resize(850, 442, BICUBIC);
    test.write("test.jpg", JPEG);*/

    /*Indexer index = Indexer();
    std::string path = "/home/peroo/raid/inc/Flac/";
    index.addFolder(path);
    path = "/home/peroo/raid/inc/incoming/";
    index.addFolder(path);*/

    while(1) {
        sleep(600);
    }

    slingshot.StopServer();

    return 0;
}
