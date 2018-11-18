#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <cstdint>
#include <string>
#include <functional>

namespace Net2Tr {
    typedef std::function<void(const std::string &packet)> RecvHandler;
    typedef std::function<void()> SentHandler;
    typedef std::function<void(int8_t err)> ErrHandler;

    class Socket {
    public:
        Socket();
        ~Socket();
        void set_pcb(void *pcb);
        void async_recv(const RecvHandler &handler);
        void async_send(const std::string &packet, const SentHandler &handler);
        void async_err(const ErrHandler &handler);
        void cancel();
        std::string src_addr();
        uint16_t src_port();
        std::string dst_addr();
        uint16_t dst_port();
    private:
        class SocketInternal;
        SocketInternal *internal;
    };
}

#endif // _SOCKET_H_
