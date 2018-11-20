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

#include "tcpsession.h"
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include "socket.h"
using namespace std;
using namespace boost::asio;

namespace Net2Tr {
    class TCPSession::TCPSessionInternal {
    public:
        TCPSession &session;
        string socks5_addr;
        uint16_t socks5_port;
        Socket in_sock;
        ip::tcp::socket out_sock;
        char recv_buf[8192];

        TCPSessionInternal(TCPSession &session, io_service &service, const string &socks5_addr, uint16_t socks5_port) : session(session), socks5_addr(socks5_addr), socks5_port(socks5_port), out_sock(service) {}
    };

    TCPSession::TCPSession(void *service, const string &socks5_addr, uint16_t socks5_port)
    {
        internal = new TCPSessionInternal(*this, *(io_service *) service, socks5_addr, socks5_port);
    }

    TCPSession::~TCPSession()
    {
        puts("disconnected");
        delete internal;
    }

    Socket *TCPSession::socket()
    {
        return &internal->in_sock;
    }

    void TCPSession::start()
    {
        puts("new connection");
    }
}
