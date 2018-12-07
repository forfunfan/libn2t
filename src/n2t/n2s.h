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

#ifndef _N2T_N2S_H_
#define _N2T_N2S_H_

#include <cstdint>
#include <string>

namespace Net2Tr {
    class N2T;

    class N2S {
    public:
        N2S(int tun_fd, N2T &n2t, const std::string &socks5_addr, uint16_t socks5_port);
        ~N2S();
        void start();
        void stop();
    private:
        class N2SInternal;
        N2SInternal *internal;
    };
}

#endif // _N2T_N2S_H_
