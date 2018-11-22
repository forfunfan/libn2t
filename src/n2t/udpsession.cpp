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

#include "udpsession.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/write.hpp>
#include "udppacket.h"
#include "utils.h"
using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

namespace Net2Tr{
    class UDPSession::UDPSessionInternal {
    public:
        enum Status {
            HANDSHAKE,
            REQUEST,
            FORWARD,
            DESTROY
        } status;
        UDPSession &session;
        string socks5_addr;
        uint16_t socks5_port;
        udp::socket out_sock;
        tcp::socket tcp_sock;
        char recv_buf[8192];
        steady_timer gc_timer;
        UDPPacket initial_packet;
        WriteUDP in_write;

        UDPSessionInternal(UDPSession &session, io_service &service, const string &socks5_addr, uint16_t socks5_port, const UDPPacket &initial_packet, WriteUDP write_udp) : status(HANDSHAKE), session(session), socks5_addr(socks5_addr), socks5_port(socks5_port), out_sock(service), tcp_sock(service), gc_timer(service), initial_packet(initial_packet), in_write(write_udp) {}

        void tcp_async_read()
        {

        }

        void tcp_async_write(const string &data)
        {

        }

        void out_async_read()
        {

        }

        void in_recv(const UDPPacket &packet)
        {

        }

        void tcp_recv(const string &data)
        {

        }

        void tcp_sent()
        {

        }

        void out_recv(const string &data)
        {

        }

        void destroy()
        {

        }
    };

    UDPSession::UDPSession(void *service, const string &socks5_addr, uint16_t socks5_port, const UDPPacket &initial_packet, WriteUDP write_udp)
    {
        internal = new UDPSessionInternal(*this, *(io_service *) service, socks5_addr, socks5_port, initial_packet, write_udp);
    }

    UDPSession::~UDPSession()
    {
        delete internal;
    }

    void UDPSession::start()
    {
        auto self = shared_from_this();
        internal->tcp_sock.async_connect(tcp::endpoint(address::from_string(internal->socks5_addr), internal->socks5_port), [this, self](const boost::system::error_code &error)
        {
            if (error) {
                internal->destroy();
                return;
            }
            internal->tcp_async_write(string("\x05\x01\x00", 3));
        });
    }

    bool UDPSession::process(const UDPPacket &packet)
    {
        if (packet.src_addr != internal->initial_packet.src_addr || packet.src_port != internal->initial_packet.src_port)
            return false;
        internal->in_recv(packet);
        return true;
    }
}
