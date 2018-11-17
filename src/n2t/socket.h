#ifndef _SOCKET_H_
#define _SOCKET_H_

namespace Net2Tr {
    class Socket {
    public:
        Socket(void *pcb);
        ~Socket();
    private:
        class SocketInternal;
        SocketInternal *internal;
    };
}

#endif // _SOCKET_H_
