#include <n2t/n2t.h>
using namespace Net2Tr;

int main()
{
    N2T n2t("10.114.51.5", "255.255.255.254");
    for (;;) {
        n2t.process_events();
    }
    return 0;
}
