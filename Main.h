#ifndef MAIN_H
#define MAIN_H

#include <boost/filesystem/path.hpp>
#include <string>

#define PORT 9999


struct sqlite3;
struct MHD_Daemon;

class Main {
	public:
		Main(){};
		~Main(){};
		
		bool Initialize();
		void StopServer();
		std::string ExecuteQuery(std::string &query);
		static int Response(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, unsigned int *upload_data_size, void **con_cls);
	
	private:
		static bool RunScript(const std::string &fragment, int *status, std::string *text);
		
		static boost::filesystem::path base;
		sqlite3 *db;
		MHD_Daemon *server;
};
#endif