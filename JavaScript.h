#ifndef JAVASCRIPT_H
#define JAVASCRIPT_H

#include <v8.h>
#include <string>
#include <map>

using namespace std;
using namespace v8;

class JSDatabase;
class HttpRequest;

/**
 * An http request processor that is scriptable using JavaScript.
 */
class JsHttpRequestProcessor {
 public:

  // Creates a new processor that processes requests by invoking the
  // Process function of the JavaScript script given as an argument.
  explicit JsHttpRequestProcessor(Handle<String> script) : script_(script) { }
  ~JsHttpRequestProcessor();

  bool Initialize(string* output);
  bool Process(HttpRequest* req);

  static void Log(const char* event);

 private:

  // Execute the script associated with this processor and extract the
  // Process function.  Returns true if this succeeded, otherwise false.
  bool ExecuteScript(Handle<String> script);

  // Constructs the template that describes the JavaScript wrapper
  // type for requests.
  static Handle<ObjectTemplate> MakeRequestTemplate();

  // Callbacks that access the individual fields of request objects.
  static Handle<Value> GetPath(Local<String> name, const AccessorInfo& info);
  static Handle<Value> GetReferrer(Local<String> name,
                                   const AccessorInfo& info);
  static Handle<Value> GetHost(Local<String> name, const AccessorInfo& info);
  static Handle<Value> GetUserAgent(Local<String> name,
                                    const AccessorInfo& info);


  // Utility methods for wrapping C++ objects as JavaScript objects,
  // and going back again.
  static Handle<Object> WrapRequest(HttpRequest* obj);
  static HttpRequest* UnwrapRequest(Handle<Object> obj);

  Handle<String> script_;
  Persistent<Context> context_;
  Persistent<Function> process_;
  static Persistent<ObjectTemplate> request_template_;
};
#endif
