#include "JSDatabase.h"

#include <v8.h>

using namespace v8;

v8::Handle<Value> JSDatabase::Select(const Arguments& args) {
    Handle<Array> test = Array::New(2);
    test->Set(Number::New(0), String::New("Mango"));
    test->Set(Number::New(1), String::New("Pango"));
    return test;
}

v8::Handle<Value> JSDatabase::Insert(const Arguments& args) {
    return v8::String::New("mongo");
}

v8::Handle<Value> JSDatabase::Update(const Arguments& args) {
    return v8::Boolean::New(true);
}

v8::Handle<Value> JSDatabase::Delete(const Arguments& args) {
    return v8::Boolean::New(false);
}
