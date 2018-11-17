#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <lwip/tcp.h>

namespace Net2Tr {
    class Socket {
    public:
        Socket(tcp_pcb *pcb);
        ~Socket();
    private:
        tcp_pcb *pcb;
    };
}

#endif // _SOCKET_H_
