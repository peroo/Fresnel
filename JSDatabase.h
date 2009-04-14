#ifndef JSDATABASE_H
#define JSDATABASE_H

#include <v8.h>

class JSDatabase {
	public:
		static v8::Handle<v8::Value> Select(const v8::Arguments&);
		static v8::Handle<v8::Value> Insert(const v8::Arguments&);
		static v8::Handle<v8::Value> Update(const v8::Arguments&);
		static v8::Handle<v8::Value> Delete(const v8::Arguments&);
	private:
};
#endif
