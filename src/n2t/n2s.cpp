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

#include <memory>
#include <list>
#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

#include "n2s.h"
#include "n2t.h"
#include "tcpsession.h"
#include "udpsession.h"
#include "utils.h"

using namespace std;
using namespace boost::asio;
using namespace boost::asio::posix;

namespace Net2Tr {
    class N2S::N2SInternal {
    public:
        io_service service;
        stream_descriptor tun_stream_descriptor;
        N2T &n2t;
        string socks5_addr;
        uint16_t socks5_port;
        steady_timer lwip_timer;
        list<weak_ptr<UDPSession> > udp_sessions;
        bool stopped;

        N2SInternal(int tun_fd, N2T &n2t, const string &socks5_addr, uint16_t socks5_port)
        : tun_stream_descriptor(service, tun_fd), n2t(n2t), socks5_addr(socks5_addr),
        socks5_port(socks5_port), lwip_timer(service), stopped(false)
        {}

        void async_wait_timer()
        {
            if (stopped) return;
            lwip_timer.expires_from_now(boost::asio::chrono::milliseconds(250));
            lwip_timer.async_wait([this](const boost::system::error_code &error)
            {
                if (!error)
                    n2t.process_events();
                async_wait_timer();
            });
        }

        void async_read_fd()
        {
            if (stopped) return;
            tun_stream_descriptor.async_wait(stream_descriptor::wait_read, [this](const boost::system::error_code &error)
            {
                if (!error) {
                    char buf[2048];
                    ssize_t len = read(tun_stream_descriptor.native_handle(), buf, sizeof(buf));
                    if (len > 0) {
                        n2t.input(string(buf, size_t(len)));
                    } else {
                        // In general, tun fd remains valid
                        Utils::log(__FILE__, __func__, __LINE__, "Failed to read tun fd: " + to_string(len));
                    }
                }
                async_read_fd();
            });
        }

        void async_output()
        {
            if (stopped) return;
            n2t.async_output([this](const string &packet)
            {
                write(tun_stream_descriptor.native_handle(), packet.c_str(), packet.size());
                async_output();
            });
        }

        void async_accept()
        {
            if (stopped) return;
            auto session = make_shared<TCPSession>(&service, socks5_addr, socks5_port);
            n2t.async_accept(session->socket(), [this, session]()
            {
                session->start();
                async_accept();
            });
        }

        void async_read_udp()
        {
            if (stopped) return;
            n2t.async_udp_recv([this](const UDPPacket &packet)
            {
                for (auto it = udp_sessions.begin(); it != udp_sessions.end();) {
                    auto next = ++it;
                    --it;
                    if (it->expired()) {
                        udp_sessions.erase(it);
                    } else if (it->lock()->process(packet)) {
                        async_read_udp();
                        return;
                    }
                    it = next;
                }
                auto session = make_shared<UDPSession>(&service, socks5_addr, socks5_port, packet, bind(&N2T::udp_send, &n2t, placeholders::_1));
                udp_sessions.push_back(weak_ptr<UDPSession>(session));
                session->start();
                async_read_udp();
            });
        }
    };

    N2S::N2S(int tun_fd, N2T &n2t, const string &socks5_addr, uint16_t socks5_port)
    {
        internal = new N2SInternal(tun_fd, n2t, socks5_addr, socks5_port);
    }

    N2S::~N2S()
    {
        delete internal;
    }

    void N2S::start()
    {
        internal->stopped = false;
        internal->async_wait_timer();
        internal->async_read_fd();
        internal->async_output();
        internal->async_accept();
        internal->async_read_udp();
        internal->service.run();
    }

    void N2S::stop()
    {
        internal->stopped = true;
        internal->service.stop();
    }
}
