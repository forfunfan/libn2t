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

#ifndef _UDPPACKET_H_
#define _UDPPACKET_H_

#include <cstdint>
#include <string>

namespace Net2Tr {
    class UDPPacket {
    public:
        std::string src_addr;
        uint16_t src_port;
        std::string dst_addr;
        uint16_t dst_port;
        std::string data;
    };
}

#endif // _UDPPACKET_H_
