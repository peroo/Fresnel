#include "HttpRequest.h"
#include "OggEncode.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <sys/socket.h>
#include <cstdio>
#include <stdint.h>
#include <microhttpd.h>
#include <string>
#include <iostream>
#include <pthread.h>

namespace fs = boost::filesystem;

int HttpRequest::headerIterator(void *map, enum MHD_ValueKind kind, const char *key, const char *value)
{
    static_cast<std::map<std::string, std::string> *> (map)->insert(std::make_pair(key, value));

    return MHD_YES;
}

int callback(void *cls, uint64_t pos, char *buf, int max)
{
    return static_cast<OggEncode *>(cls)->read(pos, max, buf);
}

void mongis(const FLAC__int32 * const buffer[], int num, OggEncode *ogg) {
    ogg->feed(buffer, num);
    return;
}

void * encode(void *flacdec)
{
    FLACDecoder *flac = static_cast<FLACDecoder *>(flacdec);
    std::cout << "encoding..." << std::endl;
    int ok = flac->process_until_end_of_stream();
    std::cout << "decoding: " << (ok ? "succeeded" : "FAILED") << std::endl;

    flac->close();
    delete flac;
}

int HttpRequest::Process()
{
	int result, statusCode;
    std::string text, fragment = url, mimeType = "text/html";
    fs::path base = fs::current_path();
	fs::path path = base / "template" / fragment;

	struct MHD_Response *response;
    MHD_get_connection_values(connection, MHD_HEADER_KIND, HttpRequest::headerIterator, &headers);

    OggEncode *oggenc = new OggEncode();
    FLACDecoder *flacdec = new FLACDecoder(mongis, oggenc);

    // Determine response
    if(fragment.find("/resource/") == 0) {
        // File resource
        
        oggenc->init();
        
        statusCode = 200;
        mimeType = "audio/ogg";

        FLAC__StreamDecoderInitStatus init_status = flacdec->init("air.flac");
        flacdec->set_md5_checking(true);
        
        pthread_create(processor, NULL, encode, flacdec);

        response = MHD_create_response_from_callback(-1, 32*1024, callback, oggenc, NULL);
    } else if (fs::exists(base / "template" / fragment)) {
        // HTTP data from JS templates
    

    } else if (fragment.find("/data/") == 0) {
        // JSON data
		//HttpRequest::RunScript(fragment, &statusCode, text);
		//response = MHD_create_response_from_data(text->length(), (void*) text->c_str(), false, false);
    } else {
		statusCode = 404;
		text = "<!DOCTYPE html><html><head><title>404</title></head><body><h2>404 - File not found.</h2></body></html>";
		response = MHD_create_response_from_data(text.size(), (void*) text.data(), false, false);
    }

	MHD_add_response_header(response, "content-type", mimeType.append("; charset=utf-8").c_str());
	MHD_queue_response(connection, statusCode, response);
	MHD_destroy_response(response);

    std::cout << "URL: " << url << std::endl;

    //pthread_join(processor, NULL);

    return MHD_YES;
}

/*static int file_reader (void *cls, size_t pos, char *buf, int max)
{
  FILE *file = (FILE *)cls;

  fseek (file, pos, SEEK_SET);
  return fread (buf, 1, max, file);
}

bool Main::RunScript(const string &fragment, int *status, string *text) {
	int index = fragment.find_first_of("/", 6);
	string module = fragment.substr(6, index - 6);

	string file = fs::path(Main::base / "modules" / (module + ".js")).directory_string();
	std::cout << "Opening Script: " << file << std::endl;

	v8::HandleScope scope;
	v8::Handle<v8::String> source = ReadFile(file);
	if (source.IsEmpty()) {
	  fprintf(stderr, "Error reading '%s'.\n", file.c_str());
	  return 1;
	}

	JsHttpRequestProcessor *processor =  new JsHttpRequestProcessor(source);

	string output;
	if (!processor->Initialize(&output)) {
	  fprintf(stderr, "Error initializing processor.\n");
	  delete processor;
	  return 1;
	}

	delete processor;

	*text = output;
	*status = 200;

	return true;
}*/
