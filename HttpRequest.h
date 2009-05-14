#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <sys/socket.h>
#include <stdint.h>
#include <stdarg.h>
#include <microhttpd.h>

#include <string>
#include <map>


/**
 * A simplified http request.
 */
class HttpRequest {
    public:
        HttpRequest(struct MHD_Connection *_connection, const char *_url, const char *_method) : connection(_connection), url(_url), method(_method) {}

        int Process();
    private:
        MHD_Connection* connection;

        std::string url;
        std::string method;
        std::string referrer;
        std::string host;
        std::string user_agent;
        std::map<std::string, std::string> headers;

        static int headerIterator(void *, enum MHD_ValueKind, const char *, const char *);
};

#endif
