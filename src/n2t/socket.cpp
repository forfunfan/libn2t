/*
 * This file is part of the libn2t project.
 * Libn2t is a C++ library transforming network layer into transport layer.
 * Copyright (C) 2018  GreaterFire
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

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
        if (internal->pcb != NULL) {
            tcp_recv(internal->pcb, NULL);
            tcp_sent(internal->pcb, NULL);
            tcp_err(internal->pcb, NULL);
            tcp_close(internal->pcb);
        }
        delete internal;
    }

    void Socket::set_pcb(tcp_pcb *pcb)
    {
        internal->pcb = pcb;
        tcp_nagle_disable(internal->pcb);
        tcp_arg(internal->pcb, internal);
        tcp_recv(internal->pcb, [](void *arg, tcp_pcb *, pbuf *p, err_t err) -> err_t
        {
            if (err == ERR_OK) {
                SocketInternal *internal = (SocketInternal *) arg;
                string packet = Utils::pbuf_to_str(p);
                if (packet.size() == 0) {
                    internal->end = true;
                } else {
                    tcp_recved(internal->pcb, packet.size());
                    pbuf_free(p);
                }
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
            string tmp = internal->recv_buf;
            internal->recv_buf.clear();
            handler(tmp);
        }
    }

    void Socket::async_send(const string &packet, const SentHandler &handler)
    {
        internal->pending_len += packet.size();
        internal->sent = handler;
        tcp_write(internal->pcb, packet.c_str(), packet.size(), TCP_WRITE_FLAG_COPY);
        tcp_output(internal->pcb);
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

    std::string Socket::src_addr()
    {
        return ipaddr_ntoa(&internal->pcb->remote_ip);
    }

    uint16_t Socket::src_port()
    {
        return internal->pcb->remote_port;
    }

    std::string Socket::dst_addr()
    {
        return ipaddr_ntoa(&internal->pcb->local_ip);
    }

    uint16_t Socket::dst_port()
    {
        return internal->pcb->local_port;
    }
}
