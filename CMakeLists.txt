cmake_minimum_required(VERSION 3.1)
project(libn2t)
set(CMAKE_CXX_STANDARD 11)
add_library(n2t
    src/n2t/n2t.cpp
    src/n2t/socket.cpp
    src/sys_arch.c
    src/lwip/src/core/tcp_out.c
    src/lwip/src/core/tcp_in.c
    src/lwip/src/core/memp.c
    src/lwip/src/core/netif.c
    src/lwip/src/core/ipv6/ip6_frag.c
    src/lwip/src/core/ipv6/ip6.c
    src/lwip/src/core/ipv6/icmp6.c
    src/lwip/src/core/ipv6/ip6_addr.c
    src/lwip/src/core/ipv6/mld6.c
    src/lwip/src/core/ipv6/nd6.c
    src/lwip/src/core/timeouts.c
    src/lwip/src/core/tcp.c
    src/lwip/src/core/ip.c
    src/lwip/src/core/udp.c
    src/lwip/src/core/mem.c
    src/lwip/src/core/def.c
    src/lwip/src/core/init.c
    src/lwip/src/core/pbuf.c
    src/lwip/src/core/ipv4/icmp.c
    src/lwip/src/core/ipv4/ip4.c
    src/lwip/src/core/ipv4/ip4_addr.c
    src/lwip/src/core/inet_chksum.c)
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/lwip/src/include
)
add_executable(echo examples/echo.cpp)
target_link_libraries(echo n2t)
