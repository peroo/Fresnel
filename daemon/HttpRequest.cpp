#include "HttpRequest.h"
#include "Resource.h"

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include <iostream>
#include <sstream>
#include <cstring>

namespace fs = boost::filesystem;

HttpRequest::HttpRequest(MHD_Connection *_connection, std::string _url, std::string _method)
    : url(_url), method(_method), module(INDEX), connection(_connection)
{
    MHD_get_connection_values(_connection, MHD_HEADER_KIND, HttpRequest::headerIterator, &headers);

    parseURL();
}


HttpRequest::~HttpRequest()
{
}

void HttpRequest::parseURL()
{
    // TODO lowercase url first
    int index = 0;
    int pos = url.find_first_of('/');
    int end = url.size() - 1;
    if(url[url.size() - 1] != '/')
        end += 1; 

    while(pos != end) {
        unsigned int next = url.find_first_of('/', pos + 1);
        if(next == std::string::npos)
            next = end;

        std::string substr = url.substr(pos + 1, next - pos - 1);
        if(index == 0) {
            if(substr == "resource")
                module = RESOURCE;
            else if(substr == "data")
                module = DATA;
            else {
                module = STATIC_FILE;
                object = url.substr(1, end);
                return;
            }
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
    (void) kind;
    static_cast<std::map<std::string, std::string> *> (map)->insert(std::make_pair(key, value));

    return MHD_YES;
}

const char* itos(int number)
{
    std::stringstream out;
    out << number;
    return out.str().c_str();
}

void HttpRequest::render(Resource *res)
{
    response = MHD_create_response_from_callback(-1, 32*1024, Resource::staticReader, res, NULL);
    MHD_add_response_header(response, "Content-Type", res->getMimetype().c_str());
    //MHD_add_response_header(response, "Content-Length", itos(res->getSize()));
    MHD_add_response_header(response, "Accept-Ranges", "None");
    MHD_queue_response(connection, 200, response);
    MHD_destroy_response(response);
}

void HttpRequest::render(std::string text, std::string mimetype)
{
    response = MHD_create_response_from_data(text.length(), (void*)text.c_str(), MHD_NO, MHD_YES);
    MHD_add_response_header(response, "Content-Type", (mimetype + "; charset=utf-8;").c_str());
    MHD_add_response_header(response, "Content-Length", itos(strlen(text.c_str())));
    MHD_queue_response(connection, 200, response);
    MHD_destroy_response(response);
}

static int file_reader (void *cls, uint64_t pos, char *buf, int max)
{
  FILE *file = (FILE *)cls;

  fseek (file, pos, SEEK_SET);
  return fread (buf, 1, max, file);
}
static void file_close(void *file)
{
    fclose(static_cast<FILE*>(file));
}
static int getSize(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    return ftell(fp);
}
void HttpRequest::render(fs::path path)
{
    std::string mimetype;
    std::string ext = fs::extension(path);
    if(ext == ".js") {
        mimetype = "text/javascript";
    }
    else if(ext == ".htm" || ext == ".html") {
        mimetype = "text/html";
    }
    else if(ext == ".css") {
        mimetype = "text/css";
    }
    else if(ext == ".ico") {
        mimetype = "image/vnd.microsoft.icon";
    }
    else {
        mimetype = "text/plain";
    }

    FILE *fp;
    fp = fopen(path.string().c_str(), "rb");

    int size = getSize(fp);
    response = MHD_create_response_from_callback(size, 32*1024, file_reader, fp, file_close);
    MHD_add_response_header(response, "content-length", itos(size));
    MHD_add_response_header(response, "content-type", (mimetype + "; charset=utf-8").c_str());
    MHD_queue_response(connection, 200, response);
    MHD_destroy_response(response);
}

void HttpRequest::fail(int status_code)
{
    std::string text = "<!DOCTYPE><html><head><title>Phail</title></head><body><div>Phail</div></body></html>";
    response = MHD_create_response_from_data(text.length(), (void*)text.c_str(), MHD_NO, MHD_YES);
    MHD_add_response_header(response, "content-type", "text/html; charset=utf-8");
    MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);
}
