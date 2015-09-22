//
// Created by Dusan Klinec on 21.09.15.
//
#include "logic.hpp"
#include <boost/format.hpp>

std::size_t hash_value(ShmString const& b) {
    return boost::hash_range(b.begin(), b.end());
}

namespace ph4 {
    std::string Utils::getUsername(str uname, str host) {
        char buff[256];
        snprintf(buff, sizeof(buff), "%.*s@%.*s", uname.len, uname.s, host.len, host.s);
        return std::string(buff);
    }
}
