
#include "JavaScript.h"
#include "HttpRequest.h"

#include <v8.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <map>

using namespace v8;

Persistent<ObjectTemplate> JavaScript::request_template;

static Handle<Value> LogCallback(const Arguments& args) {
  if (args.Length() < 1) return v8::Undefined();
  HandleScope scope;
  Handle<Value> arg = args[0];
  String::Utf8Value value(arg);
  JavaScript::Log(*value);
  return v8::Undefined();
}


// Reads a file into a v8 string.
v8::Handle<v8::String> ReadFile(const std::string& name) {
  FILE* file = fopen(name.c_str(), "rb");
  if (file == NULL) return v8::Handle<v8::String>();

  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);

  char* chars = new char[size + 1];
  chars[size] = '\0';
  for (int i = 0; i < size;) {
    int read = fread(&chars[i], 1, size - i, file);
    i += read;
  }
  fclose(file);
  v8::Handle<v8::String> result = v8::String::New(chars, size);
  delete[] chars;
  return result;
}

std::string JavaScript::getResult()
{
    return result;
}

bool JavaScript::run(HttpRequest *req)
{
  // Create a handle scope to hold the temporary references.
  HandleScope handle_scope;

  v8::Handle<v8::String> source = ReadFile("scripts/" + req->object + ".js");

  // Create a template for the global object where we set the
  // built-in global functions.
  Handle<ObjectTemplate> global = ObjectTemplate::New();
  global->Set(String::New("log"), FunctionTemplate::New(LogCallback));
/*
  Handle<ObjectTemplate> sql = ObjectTemplate::New();
  sql->Set(String::New("insert"), FunctionTemplate::New(JSDatabase::Insert));
  sql->Set(String::New("select"), FunctionTemplate::New(JSDatabase::Select));
  sql->Set(String::New("update"), FunctionTemplate::New(JSDatabase::Update));
  sql->Set(String::New("delete"), FunctionTemplate::New(JSDatabase::Delete));

  global->Set(String::New("SQL"), sql);*/

  Handle<Context> context = Context::New(NULL, global);
  Context::Scope context_scope(context);

  context->Global()->Set(String::New("Request"), WrapRequest(req));

  // Compile and run the script
  if (!ExecuteScript(source))
    return false;

  Handle<String> output_name = String::New("Output");
  Handle<Value> output_val = context->Global()->Get(output_name);

  if (!output_val->IsObject()) {
	Log("No output.");
	return false;
  }

  Handle<Object> output = Handle<Object>::Cast(output_val);

  result = "bah";

  // All done; all went well
  return true;
}

bool JavaScript::ExecuteScript(Handle<String> script) {
  HandleScope handle_scope;

  // We're just about to compile the script; set up an error handler to
  // catch any exceptions the script might throw.
  TryCatch try_catch;

  // Compile the script and check for errors.
  Handle<Script> compiled_script = Script::Compile(script);
  if (compiled_script.IsEmpty()) {
    String::Utf8Value error(try_catch.Exception());
    Log(*error);
    // The script failed to compile; bail out.
    return false;
  }

  // Run the script!
  Handle<Value> result = compiled_script->Run();
  if (result.IsEmpty()) {
    // The TryCatch above is still in effect and will have caught the error.
    String::Utf8Value error(try_catch.Exception());
    Log(*error);
    // Running the script failed; bail out.
    return false;
  }
  return true;
}

Handle<Object> JavaScript::WrapRequest(HttpRequest* request) {
  // Handle scope for temporary handles.
  HandleScope handle_scope;

  // Fetch the template for creating JavaScript http request wrappers.
  // It only has to be created once, which we do on demand.
  /*if (request_template.IsEmpty()) {
    Handle<ObjectTemplate> raw_template = MakeRequestTemplate();
    request_template = Persistent<ObjectTemplate>::New(raw_template);
  }

  Handle<ObjectTemplate> templ = request_template;*/
  Handle<ObjectTemplate> templ = MakeRequestTemplate();

  // Create an empty http request wrapper.
  Handle<Object> result = templ->NewInstance();

  // Wrap the raw C++ pointer in an External so it can be referenced
  // from within JavaScript.
  Handle<External> request_ptr = External::New(request);

  // Store the request pointer in the JavaScript wrapper.
  result->SetInternalField(0, request_ptr);

  // Return the result through the current handle scope.  Since each
  // of these handles will go away when the handle scope is deleted
  // we need to call Close to let one, the result, escape into the
  // outer handle scope.
  return handle_scope.Close(result);

}

HttpRequest* JavaScript::UnwrapRequest(Handle<Object> obj) {
  Handle<External> field = Handle<External>::Cast(obj->GetInternalField(0));
  void* ptr = field->Value();
  return static_cast<HttpRequest*>(ptr);
}

Handle<Value> JavaScript::GetPath(Local<String> name,
                                              const AccessorInfo& info) {
  HttpRequest* request = UnwrapRequest(info.Holder());
  return String::New(request->url.c_str(), request->url.length());
}

Handle<Value> JavaScript::GetReferrer(Local<String> name,
                                                  const AccessorInfo& info) {
  HttpRequest* request = UnwrapRequest(info.Holder());
  return String::New(request->referrer.c_str(), request->referrer.length());
}

Handle<Value> JavaScript::GetHost(Local<String> name,
                                              const AccessorInfo& info) {
  HttpRequest* request = UnwrapRequest(info.Holder());
  return String::New(request->host.c_str(), request->host.length());
}

Handle<Value> JavaScript::GetUserAgent(Local<String> name,
                                                   const AccessorInfo& info) {
  HttpRequest* request = UnwrapRequest(info.Holder());
  return String::New(request->userAgent.c_str(), request->userAgent.length());
}


Handle<ObjectTemplate> JavaScript::MakeRequestTemplate() {
  HandleScope handle_scope;

  Handle<ObjectTemplate> result = ObjectTemplate::New();
  result->SetInternalFieldCount(1);

  // Add accessors for each of the fields of the request.
  result->SetAccessor(String::NewSymbol("path"), GetPath);
  result->SetAccessor(String::NewSymbol("referrer"), GetReferrer);
  result->SetAccessor(String::NewSymbol("host"), GetHost);
  result->SetAccessor(String::NewSymbol("userAgent"), GetUserAgent);

  // Again, return the result through the current handle scope.
  return handle_scope.Close(result);
}


void JavaScript::Log(const char* message) {
  std::cout << "Logged: " << message << std::endl;
}
