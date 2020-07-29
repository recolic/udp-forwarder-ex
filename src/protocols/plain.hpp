#ifndef UDP_FORWARDER_PROT_PLAIN_HPP_
#define UDP_FORWARDER_PROT_PLAIN_HPP_ 1

#include <protocols/base.hpp>
#include <rlib/sys/sio.hpp>
#include <rlib/string.hpp>
#include <utils.hpp>
#include <common.hpp>

#if RLIB_OS_ID == OS_LINUX
#include <linux/sched.h>
#endif

namespace Protocols {
	template <typename ClientIdT, typename ServerIdT>
	struct InjectiveConnectionMapping {
	    std::unordered_map<ClientIdT, ServerIdT> client2server;
	    std::unordered_map<ServerIdT, ClientIdT> server2client;
		void add(const ClientIdT& clientId, const ServerIdT& serverId) {
			client2server[clientId] = serverId;
			server2client[serverId] = clientId;
		}
		void del(const ClientIdT& clientId) {
			const auto& serverId = client2server[clientId];
			server2client.erase(serverId);
			client2server.erase(clientId);
		}
		std::enable_if_t<! std::is_same_v<ClientIdT, ServerIdT>, void> del(const ServerIdT& serverId) {
			const auto& clientId = server2client[serverId];
			client2server.erase(clientId);
			server2client.erase(serverId);
		}
	};

	class PlainInbound : public BaseInbound {
	public:
		virtual void loadConfig(string config) override {
			auto ar = rlib::string(config).split('@'); // Also works for ipv6.
			if (ar.size() != 3)
				throw std::invalid_argument("Wrong parameter string for protocol 'plain'. Example:    plain@fe00:1e10:ce95:1@10809");
			listenAddr = ar[1];
			listenPort = ar[2].as<uint16_t>();
		}
		virtual void forwardMessageToOutbound(string binaryMessage, string senderId) override {
			// Outbound calls this function, to alert the inbound listener thread, for the new msg.
			rlib::sockIO::send_msg(ipcPipe, senderId);
			rlib::sockIO::send_msg(ipcPipe, binaryMessage);
		}
		virtual void listenForever(BaseOutbound* nextHop) override {
			std::tie(this->ipcPipe, nextHop->ipcPipe) = mk_tcp_pipe();

			// ----------------------- Initialization / Setup ------------------------------
			auto listenFd = rlib::quick_listen(listenAddr, listenPort, true);
			rlib_defer([&] {rlib::sockIO::close_ex(listenFd);});

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
                    rlog.debug("Inbound event: from outbound msg. ");
					auto targetClientId = rlib::sockIO::recv_msg(activeFd);
					auto msg = rlib::sockIO::recv_msg(activeFd);

					auto clientAddr = ClientIdUtils::parseClientId(targetClientId);
					auto status = sendto(listenFd, msg.data(), msg.size(), 0, &clientAddr.addr, clientAddr.len);
					dynamic_assert(status != -1, "sendto failed");
				}
				else if (activeFd == listenFd) {
                    rlog.debug("Inbound event: from client msg. ");
					SockAddr clientAddr;
					auto msgLength = recvfrom(activeFd, msgBuffer.data(), msgBuffer.size(), 0, &clientAddr.addr, &clientAddr.len);
					dynamic_assert(msgLength != -1, "recvfrom failed");

					forwardMessageToOutbound(msgBuffer.substr(0, msgLength), ClientIdUtils::makeClientId(clientAddr));
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
		virtual void loadConfig(string config) override {
			auto ar = rlib::string(config).split('@'); // Also works for ipv6.
			if (ar.size() != 3)
				throw std::invalid_argument("Wrong parameter string for protocol 'plain'. Example:    plain@fe00:1e10:ce95:1@10809");
			serverAddr = ar[1];
			serverPort = ar[2].as<uint16_t>();
		}
		// InboundThread calls this function. Check the mapping between senderId and serverConn, wake up listenThread, and deliver the msg. 
		virtual void forwardMessageToInbound(string binaryMessage, string senderId) override {
			rlib::sockIO::send_msg(ipcPipe, senderId);
			rlib::sockIO::send_msg(ipcPipe, binaryMessage);
		}

		// Listen the PIPE. handleMessage will wake up this thread from epoll.
		// Also listen the connection fileDescriptors.
		virtual void listenForever(BaseInbound* previousHop) override {

			// ----------------------- Initialization / Setup ------------------------------
			auto epollFd = epoll_create1(0);
			dynamic_assert((int)epollFd != -1, "epoll_create1 failed");

			while (ipcPipe == -1) {
				; // Sleep until InboundThread initializes the pipe.
#ifdef cond_resched
				cond_resched();
#endif
			}
			epoll_add_fd(epollFd, ipcPipe);

			// ----------------------- Process an event ------------------------------
			std::string msgBuffer(DGRAM_BUFFER_SIZE, '\0');
			auto onEvent = [&](auto activeFd) {
				if (activeFd == ipcPipe) {
                    rlog.debug("Outbound event: from inbound msg. ");
					// Inbound gave me a message to forward! Send it. 
					auto targetClientId = rlib::sockIO::recv_msg(activeFd);
					auto msg = rlib::sockIO::recv_msg(activeFd);

					auto iter = connectionMap.client2server.find(targetClientId);
					if (iter != connectionMap.client2server.end()) {
						// Map contains ClientId.
						rlib::sockIO::quick_send(iter->second, msg); // udp
					}
					else {
                        rlog.debug("create new conn to server. ");
						// This clientId is new. I don't know how to listen many sockets for response, so I just issue `connect` just like TCP does. 
						auto connFd = rlib::quick_connect(serverAddr, serverPort, true);
						epoll_add_fd(epollFd, connFd);
						connectionMap.add(targetClientId, connFd);
						rlib::sockIO::quick_send(connFd, msg); // udp
                        // send(connFd, msg.data(), msg.size(), MSG_DONTWAIT);
					}
				}
				else {
                    rlog.debug("Outbound msg: from server msg. ");
					// Message from some connFd. Read and forward it. 
					auto msgLength = recv(activeFd, msgBuffer.data(), msgBuffer.size(), 0);
					dynamic_assert(msgLength != -1, "recv failed");
					if (msgLength == 0) {
						// TODO: close the socket, and notify Inbound to destory data structures.
						epoll_del_fd(epollFd, activeFd);
						connectionMap.del(activeFd);
						rlib::sockIO::close_ex(activeFd);
					}

					dynamic_assert(connectionMap.server2client.count(activeFd) > 0, "connectionMap MUST contain server connfd. ");

					forwardMessageToInbound(msgBuffer.substr(0, msgLength), connectionMap.server2client.at(activeFd));
				}
			};

			// ----------------------- listener main loop ------------------------------
			epoll_event events[EPOLL_MAX_EVENTS];
			rlog.info("PlainOutbound to {}:{} is up, listening for request ...", serverAddr, serverPort);
			while (true) {
				auto nfds = epoll_wait(epollFd, events, EPOLL_MAX_EVENTS, -1);
				dynamic_assert(nfds != -1, "epoll_wait failed");
				
				for (auto cter = 0; cter < nfds; ++cter) {
					onEvent(events[cter].data.fd);
				}
			}
		}

	private:
		string serverAddr;
		uint16_t serverPort;


		InjectiveConnectionMapping<string, sockfd_t> connectionMap;

	};
}

#endif



