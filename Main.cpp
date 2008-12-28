#include "Main.h"
#include "JavaScript.h"
#include "HttpRequest.h"

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <iostream>	
#include <fstream>
#include <v8.h>
#include <sqlite3.h>
#include <microhttpd.h>

namespace fs = boost::filesystem;
using namespace std;

fs::path Main::base = "";

static int file_reader (void *cls, size_t pos, char *buf, int max)
{
  FILE *file = (FILE *)cls;

  fseek (file, pos, SEEK_SET);
  return fread (buf, 1, max, file);
}

// Reads a file into a v8 string.
v8::Handle<v8::String> ReadFile(const string& name) {
  FILE* file = fopen(name.c_str(), "rb");
  if (file == NULL) return v8::Handle<v8::String>();

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (int i = 0; i < size;) {
    int read = fread(&chars[i], 1, size - i, file);
    i += read;
  }
  fclose(file);
  v8::Handle<v8::String> result = v8::String::New(chars, size);
  delete[] chars;
  return result;
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
	  return 1;
	}
	/*
	HttpRequest *req = new HttpRequest("test", "test2", "test3", "test4");
  
	if (!processor->Process(req))
	  return 1;
	
	delete processor;
	delete req;*/
	
	*text = output;
	*status = 200;
	
	return true;
}

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


static int callback(void *NotUsed, int argc, char **argv, char **azColName){ 
  for(int i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}

string Main::ExecuteQuery(string &query)
{
  int rc;
  char *zErrMsg = 0;
  rc = sqlite3_exec(db, "CREATE TABLE test(`id` SMALLINT UNSIGNED NOT NULL, PRIMARY KEY ( `id` ))", callback, 0, &zErrMsg);
  if(rc != SQLITE_OK){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return NULL;
  }
}

bool Main::Initialize() {
	// Setup working dir
	string home = getenv("HOME");
	Main::base = home + "/.tessst";
	if(!fs::exists(Main::base)) {
		if(!create_directory(Main::base)) {
			return false;
		}
	}
	chdir(Main::base.directory_string().c_str());
	
	// Init SQLite
	const char *dbName = "database.db";
  
	if(sqlite3_open(dbName, &db)) {
		sqlite3_close(db);
		return false;
	}
	
	// Init MHD
	server = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, PORT, false, false, Main::Response, false, MHD_OPTION_END);
	if (!server)
		return false;

	cout << "Init successful." << endl;
}

void Main::StopServer()
{
  MHD_stop_daemon(server);
}

int main(void)
{
	Main *instance = new Main();
	
	instance->Initialize();

	sleep (600);
	
	instance->StopServer();

	return 0;
}
