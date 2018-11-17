#include "socket.h"
#include <lwip/tcp.h>

namespace Net2Tr {
    class Socket::SocketInternal {
    public:
        tcp_pcb *pcb;
    };

    Socket::Socket(void *pcb)
    {
        internal = new SocketInternal();
        internal->pcb = (tcp_pcb *) pcb;
    }

    Socket::~Socket()
    {
        tcp_close(internal->pcb);
        delete internal;
    }
}
