#ifndef _N2T_H_
#define _N2T_H_

#include <cstdint>
#include <string>
#include <functional>
#include "socket.h"

namespace Net2Tr {
    typedef std::function<void(const std::string &packet)> OutputHandler;
    typedef std::function<void(Socket &s)> NewConnectionHandler;

    class N2T {
    public:
        N2T(std::string ip_addr, std::string netmask, uint16_t mtu = 1500);
        ~N2T();
        void input(const std::string &packet);
        void set_output_handler(const OutputHandler &handler);
        void set_new_connection_handler(const NewConnectionHandler &handler);
        static void process_events();
    private:
        class N2TInternal;
        N2TInternal *internal;
        OutputHandler output;
        NewConnectionHandler new_connection;
        static void init();
    };
}

#endif // _N2T_H_
