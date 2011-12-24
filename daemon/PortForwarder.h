#ifndef PORTFORWARDER_H
#define PORTFORWARDER_H

#include <string>

class PortForwarder {
    public:
        bool init(unsigned int timeout);
        bool add( unsigned short port );
        static std::string getExtIp();
    protected:
        struct UPNPUrls* urls;
        struct IGDdatas* data;
        void get_status();

        static std::string m_externalip;
        std::string m_lanip;
        unsigned int m_upbps, m_downbps;
};

#endif
