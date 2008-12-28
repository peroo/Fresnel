#ifndef SQLITE_H
#define SQLITE_H

#include <string>

struct sqlite3;

class SQLite() {
	public:
		SQLite() { }
		~SQLite();
		
		bool Initialize(const char *filename
		
		std::string[][] Select(std::string &query);
		bool Insert(std::string &query);
		bool Update(std::string &query);
	private:
		sqlite3 *db;
};

#endif