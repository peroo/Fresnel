#include "PortForwarder.h"

#include "miniupnpc/miniwget.h"
#include "miniupnpc/miniupnpc.h"
#include "miniupnpc/upnpcommands.h"
    
#include <string.h>

std::string PortForwarder::m_externalip;

std::string
PortForwarder::getExtIp()
{
    return m_externalip;
}

bool
PortForwarder::init(unsigned int timeout)
{
   struct UPNPDev *devlist;
   struct UPNPDev *dev;
   char *descXML;
   int descXMLsize = 0;

   urls = (UPNPUrls*)malloc(sizeof(struct UPNPUrls));
   data = (IGDdatas*)malloc(sizeof(struct IGDdatas));
   memset(urls, 0, sizeof(struct UPNPUrls));
   memset(data, 0, sizeof(struct IGDdatas));
   int error;
   devlist = upnpDiscover(timeout, NULL, NULL, 0, 0, &error);
   if (devlist)
   {
       dev = devlist;
       while (dev)
       {
           if (strstr (dev->st, "InternetGatewayDevice"))
               break;
           dev = dev->pNext;
       }
       if (!dev)
           dev = devlist; /* defaulting to first device */

       printf("UPnP device :\n"
              " desc: %s\n st: %s\n",
              dev->descURL, dev->st);

       descXML = (char*)miniwget(dev->descURL, &descXMLsize);
       if (descXML)
       {
           parserootdesc (descXML, descXMLsize, data);
           free (descXML); descXML = 0;
           GetUPNPUrls (urls, data, dev->descURL);
       }
       // get lan IP:
       char lanaddr[16];
       //int i;
       UPNP_GetValidIGD(devlist, urls, data, (char*)&lanaddr, 16);
       m_lanip = std::string(lanaddr);
       
       freeUPNPDevlist(devlist);
       get_status();
       return true;
   }
   return false;
}

bool
PortForwarder::add( unsigned short port )
{
   char port_str[16];
   int r;
   printf("Portfwd::add (%s, %d)\n", m_lanip.c_str(), port);
   if(urls->controlURL[0] == '\0')
   {
       printf("Portfwd - the init was not done !\n");
       return false;
   }
   sprintf(port_str, "%d", port);
   r = UPNP_AddPortMapping(urls->controlURL, data->first.servicetype,
                           port_str, port_str, m_lanip.c_str(), NULL, "TCP", NULL, NULL);
   if(r!=0)
   {
    printf("AddPortMapping(%s, %s, %s) failed, code %d\n", port_str, port_str, m_lanip.c_str(), r);
    return false;
   }
   return true;
}

void
PortForwarder::get_status()
{
    // get connection speed
    UPNP_GetLinkLayerMaxBitRates(
        urls->controlURL_CIF, data->CIF.servicetype, &m_downbps, &m_upbps);

    // get external IP adress
    char ip[16];
    if( 0 != UPNP_GetExternalIPAddress( urls->controlURL, 
                                        data->first.servicetype, 
                                        (char*)&ip ) )
    {
        m_externalip = ""; //failed
    }else{
        m_externalip = std::string(ip);
    }
}
