#ifndef SLINGSHOT_H
#define SLINGSHOT_H

#include <ostream>
#include <boost/filesystem/path.hpp>

#define PORT 9996

struct sqlite3;
struct MHD_Daemon;

class Slingshot {
	public:
		bool init();
		void StopServer();
        static std::ostream& Debug(int level);

	private:
		boost::filesystem::path base;
		sqlite3* db;
		MHD_Daemon *server;
};
#endif
