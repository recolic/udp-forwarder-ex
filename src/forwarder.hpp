#ifndef UDP_FORWARDER_DYN_FORWARDER_HPP_
#define UDP_FORWARDER_DYN_FORWARDER_HPP_

#include <unordered_map>
#include <rlib/sys/sio.hpp>

#include "utils.hpp"
using std::string;

struct ConnectionMapping {
    std::unordered_map<string, fd_t> client2server;
    std::unordered_multimap<fd_t, string> server2client;
    static string clientInfoAsKey(string ip, uint16_t port) {
        // Also works for ipv6. We just want to eliminate duplication, rather than make it easy to read. 
        return ip + '@' + port;
    }
};

class Forwarder {
public:
    Forwarder(const std::string& inboundConfig, const std::string& outboundConfig) {

    }


private:
};

#endif
