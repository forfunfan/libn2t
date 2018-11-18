#ifndef _UDPPACKET_H_
#define _UDPPACKET_H_

#include <cstdint>
#include <string>

namespace Net2Tr {
    class UDPPacket {
    public:
        std::string src_addr;
        uint16_t src_port;
        std::string dst_addr;
        uint16_t dst_port;
        std::string data;
    };
}

#endif // _UDPPACKET_H_
