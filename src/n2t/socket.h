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

#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <cstdint>
#include <string>
#include <functional>

namespace Net2Tr {
    typedef std::function<void(const std::string &packet)> RecvHandler;
    typedef std::function<void()> SentHandler;
    typedef std::function<void(int8_t err)> ErrHandler;

    class Socket {
    public:
        Socket();
        ~Socket();
        void set_pcb(void *pcb);
        void async_recv(const RecvHandler &handler);
        void async_send(const std::string &packet, const SentHandler &handler);
        void async_err(const ErrHandler &handler);
        void cancel();
        std::string src_addr();
        uint16_t src_port();
        std::string dst_addr();
        uint16_t dst_port();
    private:
        class SocketInternal;
        SocketInternal *internal;
    };
}

#endif // _SOCKET_H_
