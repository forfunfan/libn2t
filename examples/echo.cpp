#include <cstdio>
#include <cstring>
#include <cstdint>
#include <string>
#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <n2t/n2t.h>
using namespace Net2Tr;
using namespace std;

int tun_alloc(const char *dev)
{
    ifreq ifr;
    int fd, err;
    if((fd = open("/dev/net/tun", O_RDWR)) < 0)
        return fd;
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strncpy(ifr.ifr_name, dev, IFNAMSIZ);
    if((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
        close(fd);
        return err;
    }
    return fd;
}

class EchoSession : public enable_shared_from_this<EchoSession> {
public:
    Socket s;

    void start()
    {
        async_err();
        async_recv();
        endpoints = s.src_addr() + ':' + to_string(s.src_port()) + " -> " + s.dst_addr() + ':' + to_string(s.dst_port());
        printf("new connection: %s\n", endpoints.c_str());
    }

    ~EchoSession()
    {
        printf("connection closed: %s\n", endpoints.c_str());
    }
private:
    string endpoints;

    void async_recv()
    {
        auto self = shared_from_this();
        s.async_recv([this, self](const string &packet)
        {
            if (packet.size() == 0) {
                printf("received eof\n");
                s.cancel();
            } else {
                printf("received packet: %s\n", packet.c_str());
                async_send(packet);
            }
        });
    }

    void async_send(const string &packet)
    {
        auto self = shared_from_this();
        printf("send packet: %s\n", packet.c_str());
        s.async_send(packet, [this, self, packet]()
        {
            printf("sent packet: %s\n", packet.c_str());
            async_recv();
        });
    }

    void async_err()
    {
        auto self = shared_from_this();
        s.async_err([this, self](int8_t err)
        {
            printf("received error: %d\n", err);
            s.cancel();
        });
    }
};

class EchoService {
public:
    EchoService(string ip_addr, string netmask, string ip6_addr, const char *tun) : n2t(ip_addr, netmask, ip6_addr)
    {
        fd = tun_alloc(tun);
    }

    void start()
    {
        async_output();
        async_accept();
        async_udp_recv();
        for (;;) {
            char buf[1500];
            int len = read(fd, buf, sizeof(buf));
            printf("input a packet of length %d\n", len);
            n2t.input(string(buf, len));
            n2t.process_events();
        }
    }
private:
    N2T n2t;
    int fd;

    void async_output()
    {
        n2t.async_output([this](const string &packet)
        {
            printf("output a packet of length %lu\n", packet.size());
            write(fd, packet.c_str(), packet.size());
            async_output();
        });
    }

    void async_accept()
    {
        auto session = make_shared<EchoSession>();
        n2t.async_accept(&session->s, [this, session](Socket *)
        {
            session->start();
            async_accept();
        });
    }

    void async_udp_recv()
    {
        n2t.async_udp_recv([this](const UDPPacket &packet)
        {
            printf("received UDP packet (%s:%u -> %s:%u): %s\n", packet.src_addr.c_str(), packet.src_port, packet.dst_addr.c_str(), packet.dst_port, packet.data.c_str());
            UDPPacket t;
            t.src_addr = packet.dst_addr;
            t.src_port = packet.dst_port;
            t.dst_addr = packet.src_addr;
            t.dst_port = packet.src_port;
            t.data = packet.data;
            n2t.udp_send(t);
            async_udp_recv();
        });
    }
};

int main()
{
    EchoService service("10.114.51.5", "255.255.255.254", "fd00:114:514::1", "tun0");
    service.start();
    return 0;
}
