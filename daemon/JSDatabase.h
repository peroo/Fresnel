#ifndef JSDATABASE_H
#define JSDATABASE_H

#include "SQLite.h"

#include <v8.h>
#include <string>
#include <vector>
#include <map>

class JSDatabase : SQLite {
	public:
        static v8::Handle<v8::Value> Query(const v8::Arguments& arg);
        static v8::Handle<v8::Value> GetRow(uint32_t index, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> GetRowsAffected(v8::Local<v8::String> str, const v8::AccessorInfo& info);
        static v8::Handle<v8::Value> GetInsertId(v8::Local<v8::String> str, const v8::AccessorInfo& info);

        v8::Handle<v8::Value> Execute(const v8::Arguments& arg);
        v8::Handle<v8::Value> ReadRow(unsigned int index);
	private:
        bool done;
        std::map<std::string, int> columns;
        std::vector< v8::Persistent<v8::Value> > rows;

        static JSDatabase* UnwrapDb(const v8::AccessorInfo& info);
        v8::Handle<v8::Object> MakeObject();
        void SaveColNames();
};
#endif
