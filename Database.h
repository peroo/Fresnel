#ifndef DATABASE_H
#define DATABASE_H

#include <boost/filesystem/path.hpp>
#include <string>

struct sqlite3;
class Audio;

class Database {
    public:
        Database() : paramIndex(0) {}
        ~Database();

        bool createTables();
        int insertDir(const boost::filesystem::path &path, int parent, int type);
        int insertAudio(const boost::filesystem::path &file, int path);
        int getResourceType(int id);
        std::string getResourcePath(int id);

        static bool selectDB(std::string filename);
    private:
        static sqlite3 *db;

        sqlite3_stmt *statement;
        int paramIndex;
        int stepIndex;

        void insert(std::string);

        void            query(std::string query);
        bool            step();
        void            bindInt(int value);
        void            bindString(std::string value);
        int             getInt();
        std::string     getString();

        int insertFile(const boost::filesystem::path &file, int path);
};

#endif
