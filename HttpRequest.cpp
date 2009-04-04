#include "HttpRequest.h"

#include <sys/socket.h>
#include <cstdio>
#include <stdint.h>
#include <microhttpd.h>
#include <string>
#include <iostream>


using namespace std;

HttpRequest::HttpRequest(struct MHD_Connection *connection, const char *url, const char *method)
    : connection(connection), url(url), method(method) {}



int HttpRequest::headerIterator(void *map, enum MHD_ValueKind kind, const char *key, const char *value)
{
    static_cast<std::map<const char*, const char*>*>(map)->insert(make_pair(key, value));

    return MHD_YES;
}

int HttpRequest::Process()
{
    cout << "URL: " << url << endl;
    MHD_get_connection_values(connection, MHD_HEADER_KIND, HttpRequest::headerIterator, &headers);


    map<const char* ,const char*>::iterator iter;
    for( iter = headers.begin(); iter != headers.end(); ++iter ) {
      cout << iter->first << ": " << iter->second << endl;
    }

    return MHD_YES;
}

static int file_reader (void *cls, size_t pos, char *buf, int max)
{
  FILE *file = (FILE *)cls;

  fseek (file, pos, SEEK_SET);
  return fread (buf, 1, max, file);
}

/*
int Main::Response(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, unsigned int *upload_data_size, void **con_cls)
{
	string* text = new string();
	string fragment = url;
	fs::path path = base / "template" / fragment;
	int ret, statusCode;
	struct MHD_Response *response;

	if(fs::exists(path) ) {
		if(fs::is_directory(path)) {
			statusCode = 403;
			*text = "<!DOCTYPE html><html><head><title>403</title></head><body><h2>403 - Forbidden.</h2></body></html>";
			response = MHD_create_response_from_data(text->length(), (void*) text->data(), false, false);
		} else {
			statusCode = 200;
			FILE *file = fopen(path.native_file_string().c_str(), "r");
			response = MHD_create_response_from_callback(fs::file_size(path), 32 * 1024, &file_reader, file, NULL);
		}
	} else if(fragment.find("/data/") == 0) {
		Main::RunScript(fragment, &statusCode, text);
		response = MHD_create_response_from_data(text->length(), (void*) text->data(), false, false);
	} else {
		statusCode = 404;
		*text = "<!DOCTYPE html><html><head><title>404</title></head><body><h2>404 - File not found.</h2></body></html>";
		response = MHD_create_response_from_data(text->length(), (void*) text->data(), false, false);
	}

	MHD_add_response_header(response, "content-type", "text/html; charset=utf-8");
	ret = MHD_queue_response(connection, statusCode, response);
	MHD_destroy_response(response);

	return ret;
}

bool Main::RunScript(const string &fragment, int *status, string *text) {
	int index = fragment.find_first_of("/", 6);
	string module = fragment.substr(6, index - 6);

	string file = fs::path(Main::base / "modules" / (module + ".js")).directory_string();
	cout << "Opening Script: " << file << endl;

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
