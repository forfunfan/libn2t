#ifndef _N2T_H_
#define _N2T_H_

#include <cstdint>
#include <string>
#include <functional>
#include "socket.h"
#include "udppacket.h"

namespace Net2Tr {
    typedef std::function<void(const std::string &packet)> OutputHandler;
    typedef std::function<void(Socket *s)> NewConnectionHandler;
    typedef std::function<void(const UDPPacket &packet)> UDPRecvHandler;

    class N2T {
    public:
        N2T(std::string ip_addr, std::string netmask, std::string ip6_addr = std::string(), uint16_t mtu = 1500);
        ~N2T();
        void input(const std::string &packet);
        void async_output(const OutputHandler &handler);
        void async_accept(Socket *s, const NewConnectionHandler &handler);
        void async_udp_recv(const UDPRecvHandler &handler);
        void udp_send(const UDPPacket &packet);
        void cancel();
        static void process_events();
    private:
        class N2TInternal;
        N2TInternal *internal;
    };
}

#endif // _N2T_H_
