#ifndef _N2T_H_
#define _N2T_H_

#include <cstdint>
#include <string>
#include <functional>
#include <lwip/netif.h>
#include <lwip/tcp.h>
#include "socket.h"

namespace Net2Tr {
    typedef std::function<void(const std::string &packet)> OutputHandler;
    typedef std::function<void(Socket &s)> NewConnectionHandler;

    class N2T {
    public:
        N2T(std::string ip_addr, std::string netmask, uint16_t mtu = 1500);
        ~N2T();
        void output_handler(const OutputHandler &handler);
        void input(const std::string &packet);
        static void process_events();
        void new_connection_handler(const NewConnectionHandler &handler);
    private:
        netif ni;
        tcp_pcb *listen_pcb;
        OutputHandler output;
        NewConnectionHandler new_connection;
        static void init();
        static err_t output_cb(netif *ni, pbuf *p);
    };
}

#endif // _N2T_H_
