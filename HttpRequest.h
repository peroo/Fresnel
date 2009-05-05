#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "HttpRequest.h"
#include "FLAC.h"
#include "OggEncode.h"

#include <microhttpd.h>
#include <boost/filesystem/path.hpp>

#include <sys/socket.h>
#include <cstdio>
#include <stdint.h>
#include <string>
#include <map>
#include <pthread.h>


/**
 * A simplified http request.
 */
class HttpRequest {
    public:
        HttpRequest(struct MHD_Connection *_connection, const char *_url, const char *_method, pthread_t *thread) : connection(_connection), url(_url), method(_method), processor(thread) {}

        int Process();
    private:
        MHD_Connection* connection;

        std::string url;
        std::string method;
        std::string referrer;
        std::string host;
        std::string user_agent;
        std::map<std::string, std::string> headers;

        pthread_t *processor;

        static int headerIterator(void *, enum MHD_ValueKind, const char *, const char *);
};

#endif
