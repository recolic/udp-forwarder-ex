#ifndef UDP_FORWARDER_DYN_COMMON_HPP_
#define UDP_FORWARDER_DYN_COMMON_HPP_ 1

#include <rlib/log.hpp>

extern rlib::logger rlog;

// epoll buffer size.
constexpr size_t EPOLL_MAX_EVENTS = 32;
// DGRAM packet usually smaller than 1400B.
constexpr size_t DGRAM_BUFFER_SIZE = 20480;

// Change a connection on every n seconds,
//   to reset the GFW deep-packet-inspection process.
// ( Only if server side is encrypted, so nothing happens
//   to the real openvpn server.
constexpr size_t SERVER_ENCRYPT_CONNECTION_TIMEOUT_SECONDS = 60;

#include <random> 
// MAGIC PORT NUMBER! Warning! Used for Inbound - Outbound IPC talking. Windows wepoll doesn't support PIPE, so I have to use this. 
uint16_t get_tmp_tcp_port_number() {
	std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(40000, 60000);
    return dist(mt);
}

#endif

