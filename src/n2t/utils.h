#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <lwip/pbuf.h>

namespace Net2Tr {
    class Utils {
    public:
        static pbuf *str_to_pbuf(const std::string &str);
        static std::string pbuf_to_str(pbuf *p);
    };
}

#endif // _UTILS_H_
