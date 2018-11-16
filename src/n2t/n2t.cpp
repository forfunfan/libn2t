#include "n2t.h"
#include <lwip/init.h>
#include <lwip/ip.h>
using namespace std;

namespace Net2Tr {
    N2T::N2T(string ip_addr, string netmask, uint16_t mtu)
    {
        init();
        ip4_addr_t addr;
        ip4_addr_t mask;
        ip4_addr_t gw;
        ip4addr_aton(ip_addr.c_str(), &addr);
        ip4addr_aton(netmask.c_str(), &mask);
        ip4_addr_set_any(&gw);
        netif_add(&ni, &addr, &mask, &gw, this, [](netif *ni) -> signed char {
            return ERR_OK;
        }, &ip_input);
    }

    void N2T::init()
    {
        static bool inited = false;
        if (!inited)
            lwip_init();
        inited = true;
    }
}
