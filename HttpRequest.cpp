#include "HttpRequest.h"
#include "Resource.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <iostream>
#include <pthread.h>

namespace fs = boost::filesystem;

bool HttpRequest::init()
{
    MHD_get_connection_values(connection, MHD_HEADER_KIND, HttpRequest::headerIterator, &headers);

    parseURL();

    return true;
}

void HttpRequest::parseURL()
{
    if(url[url.size() - 1] != '/')
        url += '/';
    
    int index = 0;
    int pos = url.find_first_of('/');
    int end = url.find_last_of('/');
    while(pos != end) {
        int next = url.find_first_of('/', pos + 1);

        std::string substr = url.substr(pos + 1, next - pos - 1);
        if(index == 0) {
            if(substr == "resource")
                module = RESOURCE;
            else if(substr == "data")
                module = DATA;
        }
        else if(index == 1) {
            object = substr;
        }
        else {
            parameters.push_back(substr);
        }
        
        ++index;
        pos = next;
    }
}

int HttpRequest::headerIterator(void *map, enum MHD_ValueKind kind, const char *key, const char *value)
{
    static_cast<std::map<std::string, std::string> *> (map)->insert(std::make_pair(key, value));

    return MHD_YES;
}

void HttpRequest::renderResource(Resource *res)
{
    response = MHD_create_response_from_callback(-1, 32*1024, Resource::staticReader, res, NULL);
    MHD_add_response_header(response, "content-type", (res->getMimetype() + "; charset=utf-8").c_str());
    int ret = MHD_queue_response(connection, 200, response);
    MHD_destroy_response(response);
}

/*
int HttpRequest::Process()
{
	int result, statusCode;
    std::string text, fragment = url, mimeType = "text/html";
    fs::path base = fs::current_path();
	fs::path path = base / "template" / fragment;

	struct MHD_Response *response;

    // Determine response
    if(fragment.find("/resource/") == 0) {
        // File resource
        
        Resource *res = new Audio();
        res->init(fs::path("air.flac"));
        res->load(SLING_VORBIS);

        statusCode = 200;
        mimeType = "audio/ogg";

        response = MHD_create_response_from_callback(-1, 32*1024, callback, res, NULL);
    }
    else if (fs::exists(base / "template" / fragment)) {
        // HTTP data from JS templates
    }
    else if (fragment.find("/data/") == 0) {
        // JSON data
		HttpRequest::RunScript(fragment, &statusCode, text);
		response = MHD_create_response_from_data(text->length(), (void*) text->c_str(), false, false);
    }
    else {
		statusCode = 404;
		text = "<!DOCTYPE html><html><head><title>404</title></head><body><h2>404 - File not found.</h2></body></html>";
		response = MHD_create_response_from_data(text.size(), (void*) text.data(), MHD_NO, MHD_NO);
    }

	MHD_add_response_header(response, "content-type", mimeType.append("; charset=utf-8").c_str());
	int ret = MHD_queue_response(connection, statusCode, response);
	MHD_destroy_response(response);

    std::cout << "URL: " << url << std::endl;

    return ret;
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
