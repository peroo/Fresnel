#ifndef REQUEST_H 
#define REQUEST_H

#include <sys/socket.h>
#include <stdarg.h>
#include <stdint.h>
#include <microhttpd.h>

#include <string>
#include <vector>
#include <map>

class Resource;
class JavaScript;

enum req_module {
    RESOURCE,
    DATA,
    STATIC_FILE,
    INDEX
};

class HttpRequest {
    public:
        HttpRequest(MHD_Connection *_connection, std::string _url, std::string _method); 
        ~HttpRequest() {}
        void render(Resource *res);
        void render(std::string path);
        void render(std::string text, std::string mimetype);

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

        void parseURL();
        static int headerIterator(void *, MHD_ValueKind, const char *, const char *);
};

#endif
