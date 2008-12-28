#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <string>


/**
 * A simplified http request.
 */
class HttpRequest {
 public:
  HttpRequest(const std::string& path,
              const std::string& referrer,
              const std::string& host,
              const std::string& user_agent);
  ~HttpRequest() { }
  virtual const std::string& Path() { return path_; }
  virtual const std::string& Referrer() { return referrer_; }
  virtual const std::string& Host() { return host_; }
  virtual const std::string& UserAgent() { return user_agent_; }
 private:
  std::string path_;
  std::string referrer_;
  std::string host_;
  std::string user_agent_;
};

#endif