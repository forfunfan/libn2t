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

#include "utils.h"
#include <lwip/pbuf.h>
#include <boost/asio/ip/address.hpp>
using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

namespace Net2Tr {
    pbuf *Utils::str_to_pbuf(const string &str)
    {
        pbuf *p = pbuf_alloc(PBUF_RAW, str.size(), PBUF_POOL);
        if (p == NULL)
            return NULL;
        pbuf_take(p, str.c_str(), str.size());
        return p;
    }

    string Utils::pbuf_to_str(pbuf *p)
    {
        if (p == NULL)
            return string();
        string str;
        str.reserve(p->tot_len);
        for (pbuf *pp = p; pp != NULL; pp = pp->next)
            str += string((const char *) pp->payload, pp->len);
        return str;
    }

    string Utils::addrport_to_socks5(const string &addr, uint16_t port)
    {
        string ret;
        address a = address::from_string(addr);
        if (a.is_v4()) {
            ret += '\x01';
            auto ip = a.to_v4().to_bytes();
            for (int i = 0; i < 4; ++i)
                ret += char(ip[i]);
        } else {
            ret += '\x04';
            auto ip = a.to_v6().to_bytes();
            for (int i = 0; i < 16; ++i)
                ret += char(ip[i]);
        }
        ret += char(uint8_t(port >> 8));
        ret += char(uint8_t(port & 0xFF));
        return ret;
    }
}
