#ifndef _N2T_H_
#define _N2T_H_

#include <cstdint>
#include <string>
#include <lwip/netif.h>

namespace Net2Tr {
    class N2T {
    public:
        N2T(std::string ip_addr, std::string netmask, uint16_t mtu = 1500);
    private:
        static void init();
        netif ni;
    };
}

#endif // _N2T_H_
