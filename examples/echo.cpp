#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <string>
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

int main()
{
    N2T n2t("10.114.51.5", "255.255.255.254");
    int fd = tun_alloc("tun0");
    n2t.set_output_handler([fd](const string &packet)
    {
        write(fd, packet.c_str(), packet.size());
    });
    n2t.set_new_connection_handler([](Socket &s)
    {
        s.set_recv_handler([&s](const string &packet)
        {
            if (packet.size() == 0) {
                delete &s;
                return;
            }
            s.send(packet);
        });
        s.set_err_handler([&s](int8_t)
        {
            delete &s;
        });
    });
    for (;;) {
        char buf[1500];
        int len = read(fd, buf, sizeof(buf));
        n2t.input(string(buf, len));
        n2t.process_events();
    }
    return 0;
}
