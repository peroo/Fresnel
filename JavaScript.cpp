
#include "JavaScript.h"
#include "JSDatabase.h"
#include "HttpRequest.h"

#include <v8.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>

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

std::string JavaScript::getMimetype()
{
    if(!mimetype.empty())
        return mimetype;
    else
        return "text/plain";
}

bool JavaScript::run(HttpRequest *req)
{
  // Create a handle scope to hold the temporary references.
  HandleScope handle_scope;

  v8::Handle<v8::String> source = ReadFile("scripts/" + req->object + ".js");

  Handle<ObjectTemplate> global = ObjectTemplate::New();

  Handle<Context> context = Context::New(NULL, global);
  Context::Scope context_scope(context);

  Handle<ObjectTemplate> sling = ObjectTemplate::New();
  sling->Set(String::New("Log"), FunctionTemplate::New(LogCallback));
  sling->Set(String::New("Query"), FunctionTemplate::New(JSDatabase::Query));

  Handle<Object> obj = sling->NewInstance();
  obj->Set(String::New("Request"), WrapRequest(req));
  obj->Set(String::New("params"), embedParams(req));

  context->Global()->Set(String::New("Slingshot"), obj);

  

  // Compile and run the script
  if (!ExecuteScript(source))
    return false;

  Handle<Value> output_val = context->Global()->Get(String::New("Output"));

  if(output_val->IsObject()) {
    Handle<Object> output = Handle<Object>::Cast(output_val);
    result = ParseHandle(output);
    Log("JSON output.");
  }
  else {
      Handle<Value> val = context->Global()->Get(String::New("Plain"));
      if(val->IsString()) {
          Handle<String> str = Handle<String>::Cast(val);
          String::Utf8Value bah(str);
          result = *bah;
          Log("String output.");
      }
      else {
          Log("No output.");
          return false;
      }
  }

  Handle<Value> mime = context->Global()->Get(String::New("Mimetype"));
  if(mime->IsString()) {
      Handle<String> mimestr = Handle<String>::Cast(mime);
      String::Utf8Value mimeval(mimestr);
      mimetype = *mimeval;
  }

  return true;
}

Handle<Array> JavaScript::embedParams(HttpRequest *req)
{
    HandleScope scope;

    Local<Array> arr = Array::New(req->parameters.size());

    std::vector<std::string>::iterator iter;
    int i = 0;
    for(iter = req->parameters.begin(); iter != req->parameters.end(); iter++) {
        arr->Set(Integer::New(i++), String::New(iter->c_str(), iter->length()));
    }
    return scope.Close(arr);
}

std::string JavaScript::ParseHandle(Handle<Value> value)
{
    HandleScope handle_scope;

    if(value->IsString()) {
        String::Utf8Value val(value);
        std::string str = *val;
        int i=0;
        int pos = str.find('"');
        while(pos != std::string::npos) {
            i = pos;
            str.insert(i, 1, '\\');
            pos = str.find('"', i+2);
        }

        return std::string("\"") + str + "\"";
    }
    else if(value->IsNumber()) {
        std::ostringstream o;
        o << value->ToNumber()->Value();
        return o.str();
    }
    else if(value->IsBoolean()) {
        return value->ToBoolean()->Value() ? "true" : "false";
    }
    else if(value->IsArray()) {
        std::vector<std::string> out;
        Local<Array> val = Array::Cast(*value);
        for(int i=0; i < val->Length(); ++i) {
            Handle<Value> asd = val->Get(Number::New(i));
            out.push_back(ParseHandle(asd));
        }
        
        std::string result = "[";
        std::vector<std::string>::iterator iter;
        for(iter = out.begin(); iter != out.end(); iter++) {
            result += *iter + ",";
        }
        if(out.size() > 0)
            result[result.length() -1] = ']';
        else
            result += "]";
        
        return result;
    }
    else if(value->IsNull()) {
        return "null";
    }
    else if(value->IsFunction()) {
        return "\"function\"";
    }
    else if(value->IsObject()) {
        std::vector<std::string> out;
        Local<Object> val = value->ToObject();
        Local<Array> arr = val->GetPropertyNames();
        for(int i=0; i < arr->Length(); ++i) {
            Local<Value> key = arr->Get(Number::New(i));

            String::Utf8Value str(key->ToString());

            Handle<Value> asd = val->Get(key);
            out.push_back(std::string(*str) + ": " + ParseHandle(asd));
        }
        
        std::string result = "{";
        std::vector<std::string>::iterator iter;
        for(iter = out.begin(); iter != out.end(); iter++) {
            result += *iter + ",";
        }
        if(out.size() > 0)
            result[result.length() -1] = '}';
        else
            result += "}";
        
        return result;
    }
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
  std::cout << "Javascript: " << message << std::endl;
}
