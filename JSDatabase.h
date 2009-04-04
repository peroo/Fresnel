#ifndef JSDATABASE_H
#define JSDATABASE_H

#include <v8.h>

struct sqlite3;

class JSDatabase {
	public:
		explicit JSDatabase(){};
		~JSDatabase(){};

		static v8::Handle<v8::Value> Select(const v8::Arguments&);
		static v8::Handle<v8::Value> Insert(const v8::Arguments&);
		static v8::Handle<v8::Value> Update(const v8::Arguments&);
		static v8::Handle<v8::Value> Delete(const v8::Arguments&);

		static void setDB(sqlite3* sqlite);

	private:

		static sqlite3* db;
};
#endif
