#ifndef UDP_FORWARDER_PROT_PLAIN_HPP_
#define UDP_FORWARDER_PROT_PLAIN_HPP_ 1

#include <protocols/base.hpp>
#include <rlib/sys/sio.hpp>
#include <rlib/string.hpp>
#include <utils.hpp>
#include <common.hpp>

namespace Protocols {
	class PlainInbound : public BaseInbound {
	public:
		using BaseInbound::BaseInbound;
		virtual void loadConfig(string config) override {
			auto ar = rlib::string(config).split('@'); // Also works for ipv6.
			if (ar.size() != 3)
				throw std::invalid_argument("Wrong parameter string for protocol 'plain'. Example:    plain@fe00:1e10:ce95:1@10809");
			listenAddr = ar[1];
			listenPort = ar[2].as<uint16_t>();
		}
		virtual void forwardMessageToOutbound(string binaryMessage, string senderId) override {
			// Outbound calls this function, to alert the inbound listener thread, for the new msg.


		}
		virtual void listenForever(BaseOutbound* nextHop) override {
			std::tie(this->ipcPipe, nextHop->ipcPipe) = mk_tcp_pipe();

			auto listenFd = rlib::quick_listen(listenAddr, listenPort, true);
			rlib_defer([&] {close(listenFd);});

			auto epollFd = epoll_create1(0);
			dynamic_assert((int)epollFd != -1, "epoll_create1 failed");
			epoll_add_fd(epollFd, listenFd);
			epoll_add_fd(epollFd, ipcPipe);

			// ----------------------- Process an event ------------------------------
			auto udpSenderSocket = socket(AF_INET, SOCK_DGRAM, 0);
			dynamic_assert(udpSenderSocket > 0, "socket create failed.");
			std::string msgBuffer(DGRAM_BUFFER_SIZE, '\0');
			// WARN: If you want to modify this program to work for TCP, PLEASE use rlib::sockIO::recv instead of fixed buffer.
			auto onEvent = [&](auto activeFd) {
				if (activeFd == ipcPipe) {
					// Outbound gave me a message to forward! Send it. 
					auto targetClientId = rlib::sockIO::recv_msg(activeFd);
					auto msg = rlib::sockIO::recv_msg(activeFd);

					auto clientAddr = ConnectionMapping::parseClientId(targetClientId);
					auto status = sendto(udpSenderSocket, msg.data(), msg.size(), 0, &clientAddr.addr, clientAddr.len);
					dynamic_assert(status != -1, "sendto failed");
				}
				else if (activeFd == listenFd) {
					SockAddr clientAddr;
					auto msgLength = recvfrom(activeFd, msgBuffer.data(), msgBuffer.size(), 0, &clientAddr.addr, &clientAddr.len);
					dynamic_assert(msgLength != -1, "recvfrom failed");

					forwardMessageToOutbound(msgBuffer.substr(msgLength), ConnectionMapping::makeClientId(clientAddr));
				}
			};

			// ----------------------- listener main loop ------------------------------
			epoll_event events[EPOLL_MAX_EVENTS];
			rlog.info("PlainListener listening InboundPort [{}]:{} ...", listenAddr, listenPort);
			while (true) {
				auto nfds = epoll_wait(epollFd, events, EPOLL_MAX_EVENTS, -1);
				dynamic_assert(nfds != -1, "epoll_wait failed");
				
				for (auto cter = 0; cter < nfds; ++cter) {
					onEvent(events[cter].data.fd);
				}
			}
		}

	private:
		string listenAddr;
		uint16_t listenPort;
	};


	class PlainOutbound : public BaseOutbound {
	public:
		using BaseOutbound::BaseOutbound;
		virtual void loadConfig(string config) override {

		}

	};
}

#endif



