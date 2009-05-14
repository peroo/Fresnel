#include "Slingshot.h"
#include "Database.h"
#include "JSDatabase.h"
#include "JavaScript.h"
#include "HttpRequest.h"
#include "Image.h"
#include "Indexer.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <sqlite3.h>
#include <microhttpd.h>

#include <cstdio>
#include <stdint.h>
#include <iostream>

namespace fs = boost::filesystem;

int requestCurrier(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, unsigned int *upload_data_size, void **con_cls)
{
    HttpRequest *req = new HttpRequest(connection, url, method);
    return req->Process();
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
    Database *db = new Database();
    if(!db->init("db.sqlite")) {
        std::cout << "SQL initialization failed." << std::endl;
        return false;
	}
    db->createTables();

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
    Slingshot *slingshot = new Slingshot();
    slingshot->init();
    
    /*OggEncode *oggenc = new OggEncode();
    oggenc->init();
    oggenc->addStream();

    bool ok = true;
    FILE* file = fopen("out.wav", "wb");
    FLACDecoder decoder(file, mongis);

    (void)decoder.set_md5_checking(true);

    FLAC__StreamDecoderInitStatus init_status = decoder.init("air.flac");
    if(init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        fprintf(stderr, "ERROR: initializing decoder: %s\n", FLAC__StreamDecoderInitStatusString[init_status]);
        ok = false;
    }

    if(ok) {
        ok = decoder.process_until_end_of_stream();
        fprintf(stderr, "decoding: %s\n", ok ? "succeeded" : "FAILED");
        fprintf(stderr, "   state: %s\n", decoder.get_state().resolved_as_cstring(decoder));
    }

    fclose(file);

    oggenc->closeStream();*/

    /*Image test = Image();
    test.open("002.jpg");
    std::cout << "Barcode detected: " << test.scanBarcode() << std::endl;
    test.resize(850, 442, BICUBIC);
    test.write("test.jpg", JPEG);*/

    /*Indexer test = Indexer();
    std::string path = string("/home/peroo/ampex");
    test.addFolder(path);*/

    sleep(600);

    slingshot->StopServer();

    return 0;
}
