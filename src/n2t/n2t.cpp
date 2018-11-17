#include "n2t.h"
#include <lwip/init.h>
#include <lwip/netif.h>
#include <lwip/ip.h>
#include <lwip/tcp.h>
#include <lwip/timeouts.h>
#include "utils.h"
using namespace std;

namespace Net2Tr {
    class N2T::N2TInternal {
    public:
        netif ni;
        tcp_pcb *listen_pcb;
        static err_t output_cb(netif *ni, pbuf *p)
        {
            N2T *n2t = (N2T *) (ni->state);
            string packet = Utils::pbuf_to_str(p);
            if (n2t->output)
                n2t->output(packet);
            return ERR_OK;
        }
    };

    N2T::N2T(string ip_addr, string netmask, uint16_t mtu)
    {
        init();
        internal = new N2TInternal();
        ip4_addr_t addr;
        ip4_addr_t mask;
        ip4_addr_t gw;
        ip4addr_aton(ip_addr.c_str(), &addr);
        ip4addr_aton(netmask.c_str(), &mask);
        ip4_addr_set_any(&gw);
        netif_add(&internal->ni, &addr, &mask, &gw, this, [](netif *ni) -> err_t
        {
            ni->name[0] = 't';
            ni->name[1] = '0';
            ni->output = [](netif *ni, pbuf *p, const ip4_addr_t *) -> err_t
            {
                return N2TInternal::output_cb(ni, p);
            };
            return ERR_OK;
        }, &ip_input);
        internal->ni.mtu = mtu;
        netif_set_up(&internal->ni);
        netif_set_link_up(&internal->ni);
        netif_set_default(&internal->ni);
        internal->listen_pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
        tcp_bind_netif(internal->listen_pcb, &internal->ni);
        tcp_bind(internal->listen_pcb, IP_ANY_TYPE, 14514);
        internal->listen_pcb = tcp_listen(internal->listen_pcb);
        tcp_arg(internal->listen_pcb, this);
        tcp_accept(internal->listen_pcb, [](void *arg, tcp_pcb *newpcb, err_t err) -> err_t
        {
            N2T *n2t = (N2T *) arg;
            if (err == ERR_OK) {
                Socket *s = new Socket(newpcb);
                if (n2t->new_connection)
                    n2t->new_connection(*s);
                else
                    delete s;
            }
            return ERR_OK;
        });
    }

    N2T::~N2T()
    {
        tcp_close(internal->listen_pcb);
        netif_remove(&internal->ni);
        delete internal;
    }

    void N2T::input(const string &packet)
    {
        pbuf *p = Utils::str_to_pbuf(packet);
        if (p == NULL)
            return;
        if (internal->ni.input(p, &internal->ni) != ERR_OK)
            pbuf_free(p);
    }

    void N2T::set_output_handler(const OutputHandler &handler)
    {
        output = handler;
    }

    void N2T::set_new_connection_handler(const NewConnectionHandler &handler)
    {
        new_connection = handler;
    }

    void N2T::process_events()
    {
        sys_check_timeouts();
    }

    void N2T::init()
    {
        static bool inited = false;
        if (!inited)
            lwip_init();
        inited = true;
    }
}
