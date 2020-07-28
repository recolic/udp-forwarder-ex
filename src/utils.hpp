#ifndef UDP_FORWARDER_DYN_UTILs_HPP_
#define UDP_FORWARDER_DYN_UTILs_HPP_ 1

#include <rlib/sys/os.hpp>

#if RLIB_OS_ID == OS_LINUX
#include <sys/epoll.h>
#elif RLIB_OS_ID == OS_WINDOWS
#include <wepoll.h>
#endif


inline void epoll_add_fd(fd_t epollFd, fd_t fd) {
    epoll_event event {
        .events = EPOLLIN,
        .data = {
                .fd = fd,
        }
    };
    auto ret1 = epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event);
    if(ret1 == -1)
        throw std::runtime_error("epoll_ctl failed.");
}
inline void epoll_del_fd(fd_t epollFd, fd_t fd) {
    epoll_event event {
        .events = EPOLLIN,
        .data = {
                .fd = fd,
        }
    };
    auto ret1 = epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, &event); // Can be nullptr since linux 2.6.9
    if(ret1 == -1)
        throw std::runtime_error("epoll_ctl failed.");
}



#endif

