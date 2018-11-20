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

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <n2t/n2t.h>
#include <n2t/n2s.h>
using namespace Net2Tr;

int main()
{
    int fd;
    if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strcpy(ifr.ifr_name, "tun0");
    if (ioctl(fd, TUNSETIFF, (void *) &ifr) < 0) {
        perror("ioctl");
        close(fd);
        exit(EXIT_FAILURE);
    }
    N2T n2t("10.114.51.5", "255.255.255.254", "fd00:114:514::1");
    N2S n2s(fd, n2t, "", 0);
    n2s.start();
    return 0;
}
