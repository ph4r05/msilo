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

    std::vector <std::string> Utils::split(const std::string& str, const std::string& delimiter) {
        std::vector <std::string> tokens;
        std::string::size_type lastPos = 0;
        std::string::size_type pos = str.find(delimiter, lastPos);

        while (std::string::npos != pos) {
            // Found a token, add it to the vector.
            tokens.push_back(str.substr(lastPos, pos - lastPos));
            lastPos = pos + delimiter.size();
            pos = str.find(delimiter, lastPos);
        }

        tokens.push_back(str.substr(lastPos, str.size() - lastPos));
        return tokens;
    }

    int Utils::splitUserName(const std::string &uname, char * u, size_t uSize, char * h, size_t hSize) {
        std::vector<std::string> parts = ph4::Utils::split(uname, "@");
        if (parts.size() != 2){
            return -1;
        }

        // If username cannot safelly fit in to the buffer, report error.
        if (parts[0].size() >= uSize || parts[1].size() >= hSize){
            return -2;
        }

        strncpy(u, parts[0].c_str(), uSize);
        strncpy(h, parts[1].c_str(), hSize);
        return 0;
    }
}
