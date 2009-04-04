#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <sys/socket.h>
#include <cstdio>
#include <stdint.h>
#include <microhttpd.h>
#include <string>
#include <map>


/**
 * A simplified http request.
 */
class HttpRequest {
    public:
        HttpRequest(struct MHD_Connection*, const char *, const char *);
        ~HttpRequest(){}

        int Process();
    private:
        MHD_Connection* connection;

        const char *url;
        const char *method;
        std::string referrer;
        std::string host;
        std::string user_agent;
        std::map<const char *, const char *> headers;

        static int headerIterator(void *, enum MHD_ValueKind, const char *, const char *);
};

#endif
