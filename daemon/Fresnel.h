#ifndef FRESNEL_H 
#define FRESNEL_H

#include <ostream>

#define PORT 9996

struct sqlite3;
struct MHD_Daemon;

class Fresnel {
	public:
		bool init();
		void StopServer();
        static std::ostream& Debug(int level);

	private:
        std::string base;
		sqlite3* db;
		MHD_Daemon *server;
};
#endif
