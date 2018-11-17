#include "socket.h"
#include <lwip/tcp.h>
#include "utils.h"
using namespace std;

namespace Net2Tr {
    class Socket::SocketInternal {
    public:
        tcp_pcb *pcb;
    };

    Socket::Socket(void *pcb)
    {
        internal = new SocketInternal();
        internal->pcb = (tcp_pcb *) pcb;
        tcp_arg(internal->pcb, this);
        tcp_recv(internal->pcb, [](void *arg, tcp_pcb *, pbuf *p, err_t err) -> err_t
        {
            Socket *s = (Socket *) arg;
            if (err == ERR_OK) {
                if (s->recv)
                    s->recv(Utils::pbuf_to_str(p));
            }
            return ERR_OK;
        });
        tcp_sent(internal->pcb, [](void *arg, tcp_pcb *, u16_t len) -> err_t
        {
            Socket *s = (Socket *) arg;
            if (s->sent)
                s->sent(len);
            return ERR_OK;
        });
        tcp_err(internal->pcb, [](void *arg, err_t err)
        {
            Socket *s = (Socket *) arg;
            if (s->err)
                s->err(err);
        });
    }

    Socket::~Socket()
    {
        tcp_close(internal->pcb);
        delete internal;
    }

    void Socket::send(const string &packet)
    {
        tcp_write(internal->pcb, packet.c_str(), packet.size(), TCP_WRITE_FLAG_COPY);
    }

    void Socket::set_recv_handler(const RecvHandler &handler)
    {
        recv = handler;
    }

    void Socket::set_sent_handler(const SentHandler &handler)
    {
        sent = handler;
    }

    void Socket::set_err_handler(const ErrHandler &handler)
    {
        err = handler;
    }
}
