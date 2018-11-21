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

#include "n2t.h"
#include <queue>
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/ip.h>
#include <lwip/tcp.h>
#include <lwip/udp.h>
#include <lwip/timeouts.h>
#include "socket.h"
#include "udppacket.h"
#include "utils.h"
using namespace std;

namespace Net2Tr {
    class N2T::N2TInternal {
    public:
        netif ni;
        tcp_pcb *listen_pcb;
        udp_pcb *upcb;
        OutputHandler output;
        NewConnectionHandler new_connection;
        UDPRecvHandler udp_recv;
        std::queue<string> output_que;
        std::queue<tcp_pcb *> connection_que;
        std::queue<UDPPacket> udp_que;
        Socket *pending_socket;

        N2TInternal()
        {
            init();
        }

        static void init()
        {
            static bool inited = false;
            if (!inited)
                lwip_init();
            inited = true;
        }

        static err_t output_cb(netif *ni, pbuf *p)
        {
            N2TInternal *internal = (N2TInternal *) (ni->state);
            string packet = Utils::pbuf_to_str(p);
            if (internal->output) {
                OutputHandler tmp = internal->output;
                internal->output = OutputHandler();
                tmp(packet);
            } else {
                internal->output_que.push(packet);
            }
            return ERR_OK;
        }
    };

    N2T::N2T(const string &ip_addr, const string &netmask, const string &ip6_addr, uint16_t mtu)
    {
        internal = new N2TInternal();
        ip4_addr_t addr;
        ip4_addr_t mask;
        ip4_addr_t gw;
        ip4addr_aton(ip_addr.c_str(), &addr);
        ip4addr_aton(netmask.c_str(), &mask);
        ip4_addr_set_any(&gw);
        netif_add(&internal->ni, &addr, &mask, &gw, internal, [](netif *ni) -> err_t
        {
            ni->name[0] = 't';
            ni->name[1] = '0';
            ni->output = [](netif *ni, pbuf *p, const ip4_addr_t *) -> err_t
            {
                return N2TInternal::output_cb(ni, p);
            };
            return ERR_OK;
        }, &ip_input);
        internal->ni.mtu = mtu;
        if (ip6_addr.size() > 0) {
            ip6_addr_t ip6addr;
            ip6addr_aton(ip6_addr.c_str(), &ip6addr);
            netif_ip6_addr_set(&internal->ni, 0, &ip6addr);
            netif_ip6_addr_set_state(&internal->ni, 0, IP6_ADDR_VALID);
            internal->ni.output_ip6 = [](netif *ni, pbuf *p, const ip6_addr_t *) -> err_t
            {
                return N2TInternal::output_cb(ni, p);
            };
            internal->ni.mtu6 = mtu;
        }
        netif_set_up(&internal->ni);
        netif_set_link_up(&internal->ni);
        netif_set_default(&internal->ni);
        internal->listen_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
        tcp_bind_netif(internal->listen_pcb, &internal->ni);
        tcp_bind(internal->listen_pcb, IP_ANY_TYPE, 14514);
        internal->listen_pcb = tcp_listen(internal->listen_pcb);
        tcp_arg(internal->listen_pcb, internal);
        tcp_accept(internal->listen_pcb, [](void *arg, tcp_pcb *newpcb, err_t err) -> err_t
        {
            if (err == ERR_OK) {
                N2TInternal *internal = (N2TInternal *) arg;
                if (internal->new_connection) {
                    internal->pending_socket->set_pcb(newpcb);
                    NewConnectionHandler tmp = internal->new_connection;
                    internal->new_connection = NewConnectionHandler();
                    tmp();
                } else {
                    internal->connection_que.push(newpcb);
                }
            }
            return ERR_OK;
        });
        internal->upcb = udp_new_ip_type(IPADDR_TYPE_ANY);
        udp_bind_netif(internal->upcb, &internal->ni);
        udp_bind(internal->upcb, IP_ANY_TYPE, 14514);
        udp_recv(internal->upcb, [](void *arg, udp_pcb *pcb, pbuf *p, const ip_addr_t *addr, u16_t port)
        {
            N2TInternal *internal = (N2TInternal *) arg;
            UDPPacket packet;
            packet.src_addr = ipaddr_ntoa(addr);
            packet.src_port = port;
            packet.dst_addr = ipaddr_ntoa(ip_current_dest_addr());
            packet.dst_port = pcb->local_port;
            packet.data = Utils::pbuf_to_str(p);
            pbuf_free(p);
            if (internal->udp_recv) {
                UDPRecvHandler tmp = internal->udp_recv;
                internal->udp_recv = UDPRecvHandler();
                tmp(packet);
            } else {
                internal->udp_que.push(packet);
            }
        }, internal);
    }

    N2T::~N2T()
    {
        udp_remove(internal->upcb);
        tcp_close(internal->listen_pcb);
        netif_remove(&internal->ni);
        delete internal;
    }

    void N2T::input(const string &packet)
    {
        pbuf *p = Utils::str_to_pbuf(packet);
        if (p == NULL)
            return;
        if (internal->ni.input(p, &internal->ni) != ERR_OK)
            pbuf_free(p);
    }

    void N2T::async_output(const OutputHandler &handler)
    {
        if (internal->output_que.empty()) {
            internal->output = handler;
        } else {
            string packet = internal->output_que.front();
            internal->output_que.pop();
            handler(packet);
        }
    }

    void N2T::async_accept(Socket *s, const NewConnectionHandler &handler)
    {
        if (internal->connection_que.empty()) {
            internal->new_connection = handler;
            internal->pending_socket = s;
        } else {
            tcp_pcb *pcb = internal->connection_que.front();
            internal->connection_que.pop();
            s->set_pcb(pcb);
            handler();
        }
    }

    void N2T::async_udp_recv(const UDPRecvHandler &handler)
    {
        if (internal->udp_que.empty()) {
            internal->udp_recv = handler;
        } else {
            UDPPacket packet = internal->udp_que.front();
            internal->udp_que.pop();
            handler(packet);
        }
    }

    void N2T::udp_send(const UDPPacket &packet)
    {
        pbuf *p = Utils::str_to_pbuf(packet.data);
        if (p == NULL)
            return;
        ip_addr_t src_addr;
        ipaddr_aton(packet.src_addr.c_str(), &src_addr);
        internal->upcb->local_port = packet.src_port;
        ip_addr_t dst_addr;
        ipaddr_aton(packet.dst_addr.c_str(), &dst_addr);
        udp_sendto_if_src(internal->upcb, p, &dst_addr, packet.dst_port, &internal->ni, &src_addr);
        pbuf_free(p);
    }

    void N2T::cancel()
    {
        internal->output = OutputHandler();
        internal->new_connection = NewConnectionHandler();
        internal->udp_recv = UDPRecvHandler();
    }

    void N2T::process_events()
    {
        sys_check_timeouts();
    }
}
