
#include "JavaScript.h"
#include "JSDatabase.h"
#include "HttpRequest.h"

#include <v8.h>

#include <string>
#include <map>

using namespace std;
using namespace v8;

// -------------------------
// --- P r o c e s s o r ---
// -------------------------


static Handle<Value> LogCallback(const Arguments& args) {
  if (args.Length() < 1) return v8::Undefined();
  HandleScope scope;
  Handle<Value> arg = args[0];
  String::Utf8Value value(arg);
  JsHttpRequestProcessor::Log(*value);
  return v8::Undefined();
}


// Reads a file into a v8 string.
v8::Handle<v8::String> ReadFile(const string& name) {
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

// Execute the script and fetch the Process method.
bool JsHttpRequestProcessor::Initialize(string* output) {
  // Create a handle scope to hold the temporary references.
  HandleScope handle_scope;

  // Create a template for the global object where we set the
  // built-in global functions.
  Handle<ObjectTemplate> global = ObjectTemplate::New();
  global->Set(String::New("log"), FunctionTemplate::New(LogCallback));

  Handle<ObjectTemplate> sql = ObjectTemplate::New();
  sql->Set(String::New("insert"), FunctionTemplate::New(JSDatabase::Insert));
  sql->Set(String::New("select"), FunctionTemplate::New(JSDatabase::Select));
  sql->Set(String::New("update"), FunctionTemplate::New(JSDatabase::Update));
  sql->Set(String::New("delete"), FunctionTemplate::New(JSDatabase::Delete));

  global->Set(String::New("SQL"), sql);


  // Each processor gets its own context so different processors
  // don't affect each other (ignore the first three lines).
  Handle<Context> context = Context::New(NULL, global);

  // Enter the new context so all the following operations take place
  // within it.
  Context::Scope context_scope(context);

  // Compile and run the script
  if (!ExecuteScript(script_))
    return false;

  // The script compiled and ran correctly.  Now we fetch out the
  // Process function from the global object.
  Handle<String> output_name = String::New("Output");
  Handle<Value> output_val = context->Global()->Get(output_name);

  // If there is no Process function, or if it is not a function,
  // bail out
  if (!output_val->IsString()) {
	Log("No output.");
	return false;
  }

  // It is a function; cast it to a Function
  Handle<String> output_string = Handle<String>::Cast(output_val);

  *output = *(String::Utf8Value(output_string));

  // All done; all went well
  return true;
}


bool JsHttpRequestProcessor::ExecuteScript(Handle<String> script) {
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

bool JsHttpRequestProcessor::Process(HttpRequest* request) {
  // Create a handle scope to keep the temporary object references.
  HandleScope handle_scope;

  // Enter this processor's context so all the remaining operations
  // take place there
  Context::Scope context_scope(context_);

  // Wrap the C++ request object in a JavaScript wrapper
  Handle<Object> request_obj = WrapRequest(request);

  // Set up an exception handler before calling the Process function
  TryCatch try_catch;

  // Invoke the process function, giving the global object as 'this'
  // and one argument, the request.
  const int argc = 1;
  Handle<Value> argv[argc] = { request_obj };
  Handle<Value> result = process_->Call(context_->Global(), argc, argv);
  if (result.IsEmpty()) {
    String::Utf8Value error(try_catch.Exception());
    Log(*error);
    return false;
  } else {
    return true;
  }
}


JsHttpRequestProcessor::~JsHttpRequestProcessor() {
  // Dispose the persistent handles.  When noone else has any
  // references to the objects stored in the handles they will be
  // automatically reclaimed.
  context_.Dispose();
  process_.Dispose();
}


Persistent<ObjectTemplate> JsHttpRequestProcessor::request_template_;

// -------------------------------------------
// --- A c c e s s i n g   R e q u e s t s ---
// -------------------------------------------

/**
 * Utility function that wraps a C++ http request object in a
 * JavaScript object.
 */
Handle<Object> JsHttpRequestProcessor::WrapRequest(HttpRequest* request) {
  // Handle scope for temporary handles.
  HandleScope handle_scope;

  // Fetch the template for creating JavaScript http request wrappers.
  // It only has to be created once, which we do on demand.
  if (request_template_.IsEmpty()) {
    Handle<ObjectTemplate> raw_template = MakeRequestTemplate();
    request_template_ = Persistent<ObjectTemplate>::New(raw_template);
  }
  Handle<ObjectTemplate> templ = request_template_;

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


/**
 * Utility function that extracts the C++ http request object from a
 * wrapper object.
 */
HttpRequest* JsHttpRequestProcessor::UnwrapRequest(Handle<Object> obj) {
  Handle<External> field = Handle<External>::Cast(obj->GetInternalField(0));
  void* ptr = field->Value();
  return static_cast<HttpRequest*>(ptr);
}


Handle<Value> JsHttpRequestProcessor::GetPath(Local<String> name,
                                              const AccessorInfo& info) {
  // Extract the C++ request object from the JavaScript wrapper.
  HttpRequest* request = UnwrapRequest(info.Holder());

  // Fetch the path.
  //const string& path = request->Path();
  const string& path = "test";

  // Wrap the result in a JavaScript string and return it.
  return String::New(path.c_str(), path.length());
}


Handle<Value> JsHttpRequestProcessor::GetReferrer(Local<String> name,
                                                  const AccessorInfo& info) {
  HttpRequest* request = UnwrapRequest(info.Holder());
  //const string& path = request->Referrer();
  const string& path = "test";
  return String::New(path.c_str(), path.length());
}


Handle<Value> JsHttpRequestProcessor::GetHost(Local<String> name,
                                              const AccessorInfo& info) {
  HttpRequest* request = UnwrapRequest(info.Holder());
  //const string& path = request->Host();
  const string& path = "test";
  return String::New(path.c_str(), path.length());
}


Handle<Value> JsHttpRequestProcessor::GetUserAgent(Local<String> name,
                                                   const AccessorInfo& info) {
  HttpRequest* request = UnwrapRequest(info.Holder());
  //const string& path = request->UserAgent();
  const string& path = "test";
  return String::New(path.c_str(), path.length());
}


Handle<ObjectTemplate> JsHttpRequestProcessor::MakeRequestTemplate() {
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


// --- Test ---


void JsHttpRequestProcessor::Log(const char* event) {
  printf("Logged: %s\n", event);
}
