#include "SQLite.h"
#include <sqlite3.h>

SQLite::~SQLite()
{
	sqlite3_close(db);
}

SQLite::Initialize(const char *filename)
{
	if(sqlite3_open(filename, &db) != SQLITE_OK) {
		return false;
	} else {
		return true;
	}
}