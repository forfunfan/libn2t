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
#include <list>
#include <boost/asio/io_context.hpp>
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
        list<UDPPacket> send_que;
        WriteUDP in_write;

        UDPSessionInternal(UDPSession &session, io_context &context, const string &socks5_addr, uint16_t socks5_port, const UDPPacket &initial_packet, WriteUDP write_udp)
        : status(HANDSHAKE), session(session), socks5_addr(socks5_addr), socks5_port(socks5_port),
        out_sock(context), tcp_sock(context), gc_timer(context), initial_packet(initial_packet), in_write(write_udp) {}
        ~UDPSessionInternal() {
            destroy();
        }

        void async_wait_timer()
        {
            gc_timer.expires_after(boost::asio::chrono::minutes(1));
            auto self = session.shared_from_this();
            gc_timer.async_wait([this, self](const boost::system::error_code &error)
            {
                if (!error)
                    destroy();
            });
        }

        void tcp_async_read()
        {
            auto self = session.shared_from_this();
            tcp_sock.async_read_some(buffer(recv_buf, sizeof(recv_buf)), [this, self](const boost::system::error_code &error, size_t length)
            {
                if (error) {
                    N2T_LOG(error);
                    destroy();
                    return;
                }
                tcp_recv(string(recv_buf, length));
            });
        }

        void tcp_async_write(const string &data)
        {
            auto self = session.shared_from_this();
            auto data_copy = make_shared<string>(data);
            async_write(tcp_sock, buffer(*data_copy), [this, self, data_copy](const boost::system::error_code &error, size_t)
            {
                if (error) {
                    N2T_LOG(error);
                    destroy();
                    return;
                }
                tcp_sent();
            });
        }

        void out_async_read()
        {
            auto self = session.shared_from_this();
            out_sock.async_receive(buffer(recv_buf, sizeof(recv_buf)), [this, self](const boost::system::error_code &error, size_t length)
            {
                if (error) {
                    N2T_LOG(error);
                    destroy();
                    return;
                }
                out_recv(string(recv_buf, length));
            });
        }

        void in_recv(const UDPPacket &packet)
        {
            gc_timer.cancel();
            async_wait_timer();
            if (status == FORWARD) {
                string data("\x00\x00\x00", 3);
                data += Utils::addrport_to_socks5(packet.dst_addr, packet.dst_port);
                data += packet.data;
                out_sock.send(buffer(data));
            } else {
                send_que.push_back(packet);
            }
        }

        void tcp_recv(const string &data)
        {
            boost::system::error_code error;
            switch (status) {
                case HANDSHAKE:
                    if (data != string("\x05\x00", 2)) {
                        destroy();
                        return;
                    }
                    status = REQUEST;
                    tcp_async_write(string("\x05\x03\x00\x01\x00\x00\x00\x00\x00\x00", 10));
                    break;
                case REQUEST: {
                    if (data.size() <= 3 || data.substr(0, 3) != string("\x05\x00\x00", 3)) {
                        destroy();
                        return;
                    }
                    string addr;
                    uint16_t port;
                    if (Utils::socks5_to_addrport(data.substr(3), addr, port) == -1) {
                        destroy();
                        return;
                    }
                    out_sock.connect(udp::endpoint(address::from_string(addr), port), error);
                    if (error) {
                        N2T_LOG(error);
                        destroy();
                        return;
                    }
                    status = FORWARD;
                    tcp_async_read();
                    out_async_read();
                    in_recv(initial_packet);
                    for (auto it = send_que.begin(); it != send_que.end(); ++it)
                        in_recv(*it);
                    break;
                }
                case FORWARD:
                    destroy();
                    break;
                default: break;
            }
        }

        void tcp_sent()
        {
            switch (status) {
                case HANDSHAKE:
                case REQUEST:
                    tcp_async_read();
                    break;
                default: break;
            }
        }

        void out_recv(const string &data)
        {
            gc_timer.cancel();
            async_wait_timer();
            if (status == FORWARD) {
                if (data.size() <= 3 && data.substr(0, 3) != string("\x00\x00\x00", 3)) {
                    destroy();
                    return;
                }
                string addr;
                uint16_t port;
                int res = Utils::socks5_to_addrport(data.substr(3), addr, port);
                if (res == -1) {
                    destroy();
                    return;
                }
                UDPPacket packet;
                packet.src_addr = addr;
                packet.src_port = port;
                packet.dst_addr = initial_packet.src_addr;
                packet.dst_port = initial_packet.src_port;
                packet.data = data.substr(res + 3);
                in_write(packet);
                out_async_read();
            }
        }

        void destroy()
        {
            if (status == DESTROY)
                return;
            status = DESTROY;
            boost::system::error_code ec;
            if (tcp_sock.is_open()) {
                tcp_sock.cancel(ec);
                tcp_sock.shutdown(tcp::socket::shutdown_both, ec);
                tcp_sock.close(ec);
            }
            if (out_sock.is_open()) {
                out_sock.cancel(ec);
                out_sock.close(ec);
            }
            gc_timer.cancel();
        }
    };

    UDPSession::UDPSession(void *context, const string &socks5_addr, uint16_t socks5_port, const UDPPacket &initial_packet, WriteUDP write_udp)
    {
        internal = new UDPSessionInternal(*this, *(io_context *) context, socks5_addr, socks5_port, initial_packet, write_udp);
    }

    UDPSession::~UDPSession()
    {
        if (internal != NULL) {
            delete internal;
            internal = NULL;
        }
    }

    void UDPSession::start()
    {
        internal->async_wait_timer();
        auto self = shared_from_this();
        internal->tcp_sock.async_connect(tcp::endpoint(address::from_string(internal->socks5_addr), internal->socks5_port), [this, self](const boost::system::error_code &error)
        {
            if (error) {
                N2T_LOG(error);
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
