#include "n2t.h"
#include <cstdio>
#include <lwip/init.h>
#include <lwip/ip.h>
#include <lwip/timeouts.h>
using namespace std;

namespace Net2Tr {
    N2T::N2T(string ip_addr, string netmask, uint16_t mtu)
    {
        init();
        ip4_addr_t addr;
        ip4_addr_t mask;
        ip4_addr_t gw;
        ip4addr_aton(ip_addr.c_str(), &addr);
        ip4addr_aton(netmask.c_str(), &mask);
        ip4_addr_set_any(&gw);
        netif_add(&ni, &addr, &mask, &gw, this, [](netif *ni) -> err_t
        {
            ni->name[0] = 't';
            ni->name[1] = '0';
            ni->output = [](netif *ni, pbuf *p, const ip4_addr_t *) -> err_t
            {
                return output_cb(ni, p);
            };
            return ERR_OK;
        }, &ip_input);
        ni.mtu = mtu;
        netif_set_up(&ni);
        netif_set_link_up(&ni);
        netif_set_default(&ni);
        listen_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
        tcp_bind_netif(listen_pcb, &ni);
        tcp_bind(listen_pcb, IP_ANY_TYPE, 14514);
        listen_pcb = tcp_listen(listen_pcb);
        tcp_arg(listen_pcb, this);
        tcp_accept(listen_pcb, [](void *arg, tcp_pcb *newpcb, err_t err) -> err_t
        {
            N2T *n2t = (N2T *) arg;
            if (err == ERR_OK) {
                Socket *s = new Socket(newpcb);
                if (n2t->new_connection) {
                    n2t->new_connection(*s);
                } else {
                    printf("%s: new tcp connection", __func__);
                    delete s;
                }
            }
            return ERR_OK;
        });
    }

    N2T::~N2T()
    {
        tcp_close(listen_pcb);
        netif_remove(&ni);
    }

    void N2T::output_handler(const OutputHandler &handler)
    {
        output = handler;
    }

    void N2T::input(const string &packet)
    {
        pbuf *p = pbuf_alloc(PBUF_RAW, packet.size(), PBUF_POOL);
        if (p == NULL)
            return;
        pbuf_take(p, packet.data(), packet.size());
        if (ni.input(p, &ni) != ERR_OK)
            pbuf_free(p);
    }

    void N2T::process_events()
    {
        sys_check_timeouts();
    }

    void N2T::new_connection_handler(const NewConnectionHandler &handler)
    {
        new_connection = handler;
    }

    void N2T::init()
    {
        static bool inited = false;
        if (!inited)
            lwip_init();
        inited = true;
    }

    err_t N2T::output_cb(netif *ni, pbuf *p)
    {
        N2T *n2t = (N2T *) (ni->state);
        string packet;
        packet.reserve(p->tot_len);
        for (pbuf *pp = p; pp != NULL; pp = pp->next)
            packet += string((const char *) pp->payload, pp->len);
        if (n2t->output)
            n2t->output(packet);
        else
            printf("%s: received a packet of length %u\n", __func__, packet.size());
        return ERR_OK;

    }
}
