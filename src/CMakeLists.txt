add_library(n2t
    n2t/n2s.cpp
    n2t/n2t.cpp
    n2t/socket.cpp
    n2t/tcpsession.cpp
    n2t/udpsession.cpp
    n2t/utils.cpp
    custom/sys_arch.c
    lwip/src/core/init.c
    lwip/src/core/def.c
    lwip/src/core/dns.c
    lwip/src/core/inet_chksum.c
    lwip/src/core/ip.c
    lwip/src/core/mem.c
    lwip/src/core/memp.c
    lwip/src/core/netif.c
    lwip/src/core/pbuf.c
    lwip/src/core/raw.c
    lwip/src/core/stats.c
    lwip/src/core/sys.c
    lwip/src/core/altcp.c
    lwip/src/core/altcp_alloc.c
    lwip/src/core/altcp_tcp.c
    lwip/src/core/tcp.c
    lwip/src/core/tcp_in.c
    lwip/src/core/tcp_out.c
    lwip/src/core/timeouts.c
    lwip/src/core/udp.c
    lwip/src/core/ipv4/autoip.c
    lwip/src/core/ipv4/dhcp.c
    lwip/src/core/ipv4/etharp.c
    lwip/src/core/ipv4/icmp.c
    lwip/src/core/ipv4/igmp.c
    lwip/src/core/ipv4/ip4_frag.c
    lwip/src/core/ipv4/ip4.c
    lwip/src/core/ipv4/ip4_addr.c
    lwip/src/core/ipv6/dhcp6.c
    lwip/src/core/ipv6/ethip6.c
    lwip/src/core/ipv6/icmp6.c
    lwip/src/core/ipv6/inet6.c
    lwip/src/core/ipv6/ip6.c
    lwip/src/core/ipv6/ip6_addr.c
    lwip/src/core/ipv6/ip6_frag.c
    lwip/src/core/ipv6/mld6.c
    lwip/src/core/ipv6/nd6.c)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/custom
    ${CMAKE_CURRENT_SOURCE_DIR}/lwip/src/include
)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(n2t ${CMAKE_THREAD_LIBS_INIT})
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.54.0 REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(n2t ${Boost_LIBRARIES})
