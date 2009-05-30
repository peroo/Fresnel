#ifndef REQUEST_H 
#define REQUEST_H

#include <sys/socket.h>
#include <stdint.h>
#include <stdarg.h>
#include <microhttpd.h>

#include <string>
#include <map>

class Resource;

enum req_module {
    Resource,
    Data
}

/**
 * A simplified http request.
 */
class HttpRequest {
    public:
        HttpRequest(struct MHD_Connection *connection, const char *url, const char *method) : connection(connection), url(url), method(method) {}
        bool init();

        void renderResource(Resource *res);

        const std::string       url;
        const std::string       method;
        std::map<std::string, std::string> headers;

        req_module                  module;
        int                         object;
        std::vector<std::string>    parameters;
        Resource                   *resource;

    private:
        const MHD_Connection*   connection;
        struct MHD_Response *response;
        void parseURL();
        static int headerIterator(void *, enum MHD_ValueKind, const char *, const char *);
};

#endif
