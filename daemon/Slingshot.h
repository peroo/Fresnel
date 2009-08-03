#ifndef SLINGSHOT_H
#define SLINGSHOT_H

#include <boost/filesystem/path.hpp>

#define PORT 9999

struct sqlite3;
struct MHD_Daemon;

class Slingshot {
	public:
		bool init(bool test);
		void StopServer();

	private:
		boost::filesystem::path base;
		sqlite3* db;
		MHD_Daemon *server;
};
#endif
