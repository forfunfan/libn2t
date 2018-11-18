#include <cstdio>
#include <cstring>
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
    }

    ~EchoSession()
    {
        printf("connection closed\n");
    }
private:
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
        s.async_err([this, self](signed char err)
        {
            printf("received error: %d\n", err);
            s.cancel();
        });
    }
};

class EchoService {
public:
    EchoService(string ip_addr, string netmask, const char *tun) : n2t(ip_addr, netmask)
    {
        fd = tun_alloc(tun);
    }

    void start()
    {
        async_output();
        async_accept();
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
            printf("new connection\n");
            session->start();
            async_accept();
        });
    }
};

int main()
{
    EchoService service("10.114.51.5", "255.255.255.254", "tun0");
    service.start();
    return 0;
}
