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

#ifndef _N2T_H_
#define _N2T_H_

#include <cstdint>
#include <string>
#include <functional>
#include "socket.h"
#include "udppacket.h"

namespace Net2Tr {
    typedef std::function<void(const std::string &packet)> OutputHandler;
    typedef std::function<void()> NewConnectionHandler;
    typedef std::function<void(const UDPPacket &packet)> UDPRecvHandler;

    class N2T {
    public:
        N2T(std::string ip_addr, std::string netmask, std::string ip6_addr = std::string(), uint16_t mtu = 1500);
        ~N2T();
        void input(const std::string &packet);
        void async_output(const OutputHandler &handler);
        void async_accept(Socket *s, const NewConnectionHandler &handler);
        void async_udp_recv(const UDPRecvHandler &handler);
        void udp_send(const UDPPacket &packet);
        void cancel();
        static void process_events();
    private:
        class N2TInternal;
        N2TInternal *internal;
    };
}

#endif // _N2T_H_
