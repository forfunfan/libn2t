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
using namespace std;

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
}
