#include "socket.h"

namespace Net2Tr {
    Socket::Socket(tcp_pcb *pcb) : pcb(pcb) {}

    Socket::~Socket()
    {
        tcp_close(pcb);
    }
}
