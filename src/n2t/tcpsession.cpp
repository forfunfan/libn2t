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
#include <boost/asio/write.hpp>
#include "socket.h"
using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

namespace Net2Tr {
    class TCPSession::TCPSessionInternal {
    public:
        enum Status {
            HANDSHAKE,
            REQUEST,
            FORWARD,
            DESTROY
        } status;
        TCPSession &session;
        string socks5_addr;
        uint16_t socks5_port;
        Socket in_sock;
        tcp::socket out_sock;
        char recv_buf[8192];

        TCPSessionInternal(TCPSession &session, io_service &service, const string &socks5_addr, uint16_t socks5_port) : status(HANDSHAKE), session(session), socks5_addr(socks5_addr), socks5_port(socks5_port), out_sock(service) {}

        void out_async_read()
        {
            auto self = session.shared_from_this();
            out_sock.async_read_some(buffer(recv_buf, sizeof(recv_buf)), [this, self](const boost::system::error_code &error, size_t length)
            {
                if (error && error != boost::asio::error::operation_aborted) {
                    destroy();
                    return;
                }
                out_recv(string(recv_buf, length));
            });
        }

        void out_async_write(const string &data)
        {
            auto self = session.shared_from_this();
            async_write(out_sock, buffer(data), [this, self](const boost::system::error_code &error, size_t)
            {
                if (error) {
                    destroy();
                    return;
                }
                out_sent();
            });
        }

        void out_recv(const string &packet)
        {
            switch (status) {
                case HANDSHAKE:
                    if (packet != string("\x05\x00", 2)) {
                        destroy();
                        return;
                    }
                    status = REQUEST;
                    break;
                case REQUEST:
                    break;
                case FORWARD:
                    break;
                default: break;
            }
        }

        void out_sent()
        {
            switch (status) {
                case HANDSHAKE:
                case REQUEST:
                    out_async_read();
                    break;
                case FORWARD:
                    break;
                default: break;
            }
        }

        void destroy()
        {
            if (status == DESTROY)
                return;
            status = DESTROY;
            in_sock.cancel();
            if (out_sock.is_open()) {
                boost::system::error_code ec;
                out_sock.cancel(ec);
                out_sock.shutdown(tcp::socket::shutdown_both, ec);
                out_sock.close(ec);
            }
        }
    };

    TCPSession::TCPSession(void *service, const string &socks5_addr, uint16_t socks5_port)
    {
        internal = new TCPSessionInternal(*this, *(io_service *) service, socks5_addr, socks5_port);
    }

    TCPSession::~TCPSession()
    {
        delete internal;
    }

    Socket *TCPSession::socket()
    {
        return &internal->in_sock;
    }

    void TCPSession::start()
    {
        auto self = shared_from_this();
        internal->out_sock.async_connect(tcp::endpoint(address::from_string(internal->socks5_addr), internal->socks5_port), [this, self](const boost::system::error_code &error)
        {
            if (error) {
                internal->destroy();
                return;
            }
            internal->out_async_write(string("\x05\x01\x00", 3));
        });
    }
}
