#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <cstdint>
#include <string>
#include <functional>

namespace Net2Tr {
    typedef std::function<void(const std::string &packet)> RecvHandler;
    typedef std::function<void(uint16_t len)> SentHandler;
    typedef std::function<void(int8_t err)> ErrHandler;

    class Socket {
    public:
        Socket(void *pcb);
        ~Socket();
        void send(const std::string &packet);
        void set_recv_handler(const RecvHandler &handler);
        void set_sent_handler(const SentHandler &handler);
        void set_err_handler(const ErrHandler &handler);
    private:
        class SocketInternal;
        SocketInternal *internal;
        RecvHandler recv;
        SentHandler sent;
        ErrHandler err;
    };
}

#endif // _SOCKET_H_
