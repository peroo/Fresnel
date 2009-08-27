#ifndef REQUEST_H 
#define REQUEST_H

#include <sys/socket.h>
#include <stdint.h>
#include <stdarg.h>
#include <microhttpd.h>
#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>
#include <map>

class Resource;
class JavaScript;

enum req_module {
    RESOURCE,
    DATA,
    STATIC_FILE 
};

class HttpRequest {
    public:
        HttpRequest(MHD_Connection *_connection, std::string _url, std::string _method) : resource(NULL), connection(_connection), url(_url), method(_method) {}
        ~HttpRequest();
        bool init();
        void render(Resource *res);
        void render(JavaScript *script);
        void render(boost::filesystem::path path);

        void fail(int status);

        std::string       url;
        std::string       referrer;
        std::string       userAgent;
        std::string       host;
        std::string       method;
        std::map<std::string, std::string> headers;

        req_module                  module;
        std::string                 object;
        std::vector<std::string>    parameters;

    private:
        MHD_Connection  *connection;
        MHD_Response    *response;
        Resource                   *resource;

        void parseURL();
        static int headerIterator(void *, MHD_ValueKind, const char *, const char *);
};

#endif
