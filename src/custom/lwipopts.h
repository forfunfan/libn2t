#ifndef LWIP_LWIPOPTS_H
#define LWIP_LWIPOPTS_H


#define NO_SYS                          1

#define LWIP_TIMERS                     1

/* single threaded */
#define SYS_LIGHTWEIGHT_PROT            0

#define MEM_LIBC_MALLOC                 1

#define MEMP_MEM_MALLOC                 1

#define MEM_ALIGNMENT                   4

#define MEM_SIZE                        1048576

#define MEMP_NUM_PBUF                   100

#define MEMP_NUM_TCP_PCB                1024

#define MEMP_NUM_TCP_PCB_LISTEN         16

#define LWIP_ARP                        0

#define LWIP_IPV4                       1

#define IP_FORWARD                      0

#define LWIP_ICMP                       1

#define LWIP_RAW                        0

#define LWIP_DHCP                       0

#define LWIP_AUTOIP                     0

#define LWIP_IGMP                       0

#define LWIP_DNS                        0

#define LWIP_UDP                        1

#define LWIP_UDPLITE                    0

#define LWIP_TCP                        1

#define LWIP_TCP_SACK_OUT               1

#define TCP_MSS                         1460

#define TCP_SND_BUF                     (8 * TCP_MSS)

#define LWIP_CALLBACK_API               1

#define LWIP_WND_SCALE                  1
#define TCP_RCV_SCALE                   2

#define LWIP_ALTCP                      0

#define LWIP_ALTCP_TLS                  0

/* libn2t uses one netif */
#define LWIP_SINGLE_NETIF               1

#define LWIP_NETIF_API                  0

#define LWIP_HAVE_LOOPIF                0

#define LWIP_NETCONN                    0

#define LWIP_SOCKET                     0

#define LWIP_COMPAT_SOCKETS             1

#define SO_REUSE                        1

#define LWIP_STATS                      0

#define LWIP_IPV6                       1

#define LWIP_PERF                       0

#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS

// needed on 64-bit systems, enable it always so that the same configuration
// is used regardless of the platform
#define IPV6_FRAG_COPYHEADER 1

#define PPP_SUPPORT                     0

#define PPPOE_SUPPORT                   0

#define PPPOL2TP_SUPPORT                0


#endif
