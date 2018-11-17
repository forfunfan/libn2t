#include "socket.h"
#include <queue>
#include <lwip/tcp.h>
#include "utils.h"
using namespace std;

namespace Net2Tr {
    class Socket::SocketInternal {
    public:
        tcp_pcb *pcb;
        RecvHandler recv;
        SentHandler sent;
        ErrHandler err;
        unsigned pending_len;
        std::string recv_buf;
        queue<err_t> err_que;
        bool end;

        SocketInternal() : pcb(NULL), pending_len(0), end(false) {}
    };

    Socket::Socket()
    {
        internal = new SocketInternal();
    }

    Socket::~Socket()
    {
        if (internal->pcb != NULL)
            tcp_close(internal->pcb);
        delete internal;
    }

    void Socket::set_pcb(void *pcb)
    {
        internal->pcb = (tcp_pcb *) pcb;
        tcp_arg(internal->pcb, internal);
        tcp_recv(internal->pcb, [](void *arg, tcp_pcb *, pbuf *p, err_t err) -> err_t
        {
            if (err == ERR_OK) {
                SocketInternal *internal = (SocketInternal *) arg;
                string packet = Utils::pbuf_to_str(p);
                if (packet.size() == 0)
                    internal->end = true;
                if (internal->recv) {
                    RecvHandler tmp = internal->recv;
                    internal->recv = RecvHandler();
                    tmp(packet);
                } else {
                    internal->recv_buf += packet;
                }
            }
            return ERR_OK;
        });
        tcp_sent(internal->pcb, [](void *arg, tcp_pcb *, u16_t len) -> err_t
        {
            SocketInternal *internal = (SocketInternal *) arg;
            internal->pending_len -= len;
            if (internal->pending_len == 0) {
                SentHandler tmp = internal->sent;
                internal->sent = SentHandler();
                tmp();
            }
            return ERR_OK;
        });
        tcp_err(internal->pcb, [](void *arg, err_t err)
        {
            SocketInternal *internal = (SocketInternal *) arg;
            if (internal->err) {
                ErrHandler tmp = internal->err;
                internal->err = ErrHandler();
                tmp(err);
            } else {
                internal->err_que.push(err);
            }
        });
    }

    void Socket::async_recv(const RecvHandler &handler)
    {
        if (internal->recv_buf.size() == 0 && !internal->end) {
            internal->recv = handler;
        } else {
            handler(internal->recv_buf);
            internal->recv_buf.clear();
        }
    }

    void Socket::async_send(const string &packet, const SentHandler &handler)
    {
        internal->pending_len += packet.size();
        internal->sent = handler;
        tcp_write(internal->pcb, packet.c_str(), packet.size(), TCP_WRITE_FLAG_COPY);
    }

    void Socket::async_err(const ErrHandler &handler)
    {
        if (internal->err_que.empty()) {
            internal->err = handler;
        } else {
            err_t err = internal->err_que.front();
            internal->err_que.pop();
            handler(err);
        }
    }

    void Socket::cancel()
    {
        internal->recv = RecvHandler();
        internal->sent = SentHandler();
        internal->err = ErrHandler();
    }
}
