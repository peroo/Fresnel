#include "HttpRequest.h"
#include <string>

using namespace std;

HttpRequest::HttpRequest(const string& path,
                         const string& referrer,
                         const string& host,
                         const string& user_agent)
    : path_(path),
      referrer_(referrer),
      host_(host),
      user_agent_(user_agent) { }