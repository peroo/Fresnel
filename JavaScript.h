#ifndef JAVASCRIPT_H
#define JAVASCRIPT_H

#include <v8.h>
#include <string>
#include <map>

class HttpRequest;

class JavaScript {
    public:
        bool run(HttpRequest *req);
        std::string getResult();
        std::string getMimetype();
        static void Log(const char* message);
    private:

        std::string result;
        std::string mimetype;

        static v8::Persistent<v8::ObjectTemplate> request_template;

        std::string ParseHandle(v8::Handle<v8::Value> value);

  static bool ExecuteScript(v8::Handle<v8::String> script);
  static v8::Handle<v8::ObjectTemplate> MakeRequestTemplate();
  static v8::Handle<v8::ObjectTemplate> MakeSQLTemplate();
  static v8::Handle<v8::Object> WrapRequest(HttpRequest* obj);
  static HttpRequest* UnwrapRequest(v8::Handle<v8::Object> obj);

  static v8::Handle<v8::Array> embedParams(HttpRequest *req);

  // Callbacks that access the individual fields of request objects.
  static v8::Handle<v8::Value> GetPath(v8::Local<v8::String> name, const v8::AccessorInfo& info);
  static v8::Handle<v8::Value> GetHost(v8::Local<v8::String> name, const v8::AccessorInfo& info);
  static v8::Handle<v8::Value> GetReferrer(v8::Local<v8::String> name, const v8::AccessorInfo& info);
  static v8::Handle<v8::Value> GetUserAgent(v8::Local<v8::String> name, const v8::AccessorInfo& info);

};
#endif
