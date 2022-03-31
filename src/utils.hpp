#ifndef UDP_FORWARDER_DYN_UTILs_HPP_
#define UDP_FORWARDER_DYN_UTILs_HPP_ 1

#include <rlib/sys/os.hpp>
#include <rlib/sys/sio.hpp>
#include <thread>
#include <unordered_map>
#include "common.hpp"

#if RLIB_OS_ID == OS_LINUX
#include <sys/epoll.h>
#elif RLIB_OS_ID == OS_WINDOWS
#include <wepoll.h>
#endif

#include <string>
using std::string;

struct SockAddr {
    union {
        sockaddr_storage addr_storage;
        sockaddr addr;
        sockaddr_in in4;
        sockaddr_in6 in6;
    };
    socklen_t len = sizeof(sockaddr_storage);
};

struct ClientIdUtils {
    static string makeClientId(const SockAddr &osStruct) {
        // ClientId is a binary string.
        static_assert(sizeof(osStruct) == sizeof(SockAddr), "error: programming error detected.");
        string result(sizeof(osStruct), '\0');
		// cross-platform TODO: byte-order problem.
        std::memcpy((void *)result.data(), &osStruct, sizeof(osStruct));
        return result;
    }
    static SockAddr parseClientId(const string &clientId) {
        SockAddr result;
        if (clientId.size() != sizeof(result))
            throw std::invalid_argument("parseClientId, invalid input binary string length.");
		// cross-platform TODO: byte-order problem.
        std::memcpy(&result, clientId.data(), sizeof(result));
        return result;
    }
};

#define dynamic_assert(expr, msg) do { \
    if(!(expr)) { rlog.error("Runtime Assertion Failed: AT " __FILE__ ":{} F({}), {}. Errno={}, strerror={}", __LINE__, __func__, (msg), errno, strerror(errno)); throw std::runtime_error("dynamic_assert failed. See rlog.error."); } \
} while(false)

inline void epoll_add_fd(fd_t epollFd, sockfd_t fd) {
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    auto ret1 = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event);
    if(ret1 == -1)
        throw std::runtime_error("epoll_ctl failed.");
}
inline void epoll_del_fd(fd_t epollFd, sockfd_t fd) {
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    auto ret1 = epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &event); // Can be nullptr since linux 2.6.9
    if(ret1 == -1)
        throw std::runtime_error("epoll_ctl failed.");
}

/*
#if RLIB_OS_ID == OS_WINDOWS
#error This code "mkpipe" is POSIX only. You may disable this error while developing on windows. 
// Even you make pipe working on windows, this code may still crash. because wepoll doesn't support pipe on windows!
#else
#include <unistd.h>
inline auto mkpipe() {
    int pipefd[2];
    if(0 != pipe(pipefd))
        throw std::runtime_error("mkpipe failed.");
    return std::make_pair(pipefd[0], pipefd[1]);
}
#endif
*/

inline auto mk_tcp_pipe() {
    sockfd_t connfd_cli_side, connfd_srv_side;
    auto tmp_port = get_tmp_tcp_port_number();
    auto listenfd = rlib::quick_listen("::1", tmp_port); // We have no UnixSocket on Windows.
    auto serverThread = std::thread([&] {
        connfd_srv_side = rlib::quick_accept(listenfd);
    });
    connfd_cli_side = rlib::quick_connect("::1", tmp_port);
    serverThread.join();
    rlib::sockIO::close_ex(listenfd);
    return std::make_pair(connfd_cli_side, connfd_srv_side);
}

// // Disabled because posix pipe is not bidirectional.
// inline auto make_posix_pipe() {
//     int pipefd[2];
//     auto ret = pipe(pipefd);
//     dynamic_assert(ret != -1, "Failed in make_posix_pipe, pipe call failed. ");
//     return std::make_pair(pipefd[0], pipefd[1]);
// }


template <size_t result_length>
string pskToKey(string psk) {
	// Convert user-provided variable-length psk to a 32-byte key.
    using HashType = decltype(std::hash<std::string>{}.operator()(std::string())); // std::result_of_t<std::hash<std::string>::operator()>;
	static_assert(result_length % sizeof(HashType) == 0, "pskToKey: result_length MUST be multiply of HashResultSize.");

	string result(result_length, '\0');
	auto hashObject = std::hash<std::string>{};
	for (auto i = 0; i < result.size(); i += sizeof(HashType)) {
		auto hashResult = hashObject(psk);
		// cross-platform TODO: byte-order problem.
		std::memcpy(result.data() + i, &hashResult, sizeof(HashType));
	}
	return result;
}


#endif

