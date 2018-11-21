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

#ifndef _N2T_UDPSESSION_H_
#define _N2T_UDPSESSION_H_

#include <cstdint>
#include <string>
#include <memory>
#include <functional>

namespace Net2Tr{
    class UDPPacket;

    typedef std::function<void()> GarbageCollector;
    typedef std::function<void()> AsyncReadUDP;
    typedef std::function<void(const UDPPacket &packet)> WriteUDP;

    class UDPSession : public std::enable_shared_from_this<UDPSession> {
    public:
        UDPSession(void *service, const std::string &socks5_addr, uint16_t socks5_port, const UDPPacket &initial_packet, GarbageCollector gc, AsyncReadUDP async_read_udp, WriteUDP write_udp);
        ~UDPSession();
        void start();
        bool process(const UDPPacket &packet);
    private:
        class UDPSessionInternal;
        UDPSessionInternal *internal;
    };
}

#endif // _N2T_UDPSESSION_H_
