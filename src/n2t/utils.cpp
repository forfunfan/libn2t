#include "utils.h"
using namespace std;

namespace Net2Tr {
    pbuf *Utils::str_to_pbuf(const string &str)
    {
        pbuf *p = pbuf_alloc(PBUF_RAW, str.size(), PBUF_POOL);
        if (p == NULL)
            return NULL;
        pbuf_take(p, str.c_str(), str.size());
        return p;
    }

    string Utils::pbuf_to_str(pbuf *p)
    {
        if (p == NULL)
            return string();
        string str;
        str.reserve(p->tot_len);
        for (pbuf *pp = p; pp != NULL; pp = pp->next)
            str += string((const char *) pp->payload, pp->len);
        return str;
    }
}
