#ifndef UDP_FORWARDER_PROT_BASE_HPP_
#define UDP_FORWARDER_PROT_BASE_HPP_ 1

#include <rlib/class_decorator.hpp>
#include <string>
using std::string;

/*
User
      |----------------------|       |----------------------|       
 ---> |PlainInbound          |  /==> |MiscInbound --PIPE-\  |
      |       \--PIPE---\    |  |    |                    | |
      |          MiscOutbound| =/    |         PlainOutbound| ----> UDP App
      |----------------------|       |----------------------|       
         UDP Forwarder Client          UDP Forwarder Server
*/

namespace Protocols {
	// Outbound holds the senderId=>nextHopFd mapping.
	// senderId is "$ip@$port", for example, `fe80:8100::1@1080`. 
	// Note: this interface works for both TCP and UDP.
	struct BaseOutbound : rlib::noncopyable {
		BaseOutbound(string outboundConfig) {
			loadConfig(outboundConfig);
		}
		virtual ~BaseOutbound() = default;

		// Init data structures.
		virtual void loadConfig(string config) = 0;

		// InboundThread calls this function. Check the mapping between senderId and serverConn, wake up listenThread, and deliver the msg. 
		virtual void forwardMessageToInbound(string binaryMessage, string senderId) = 0;

		// Listen the PIPE. handleMessage will wake up this thread from epoll.
		// Also listen the connection fileDescriptors.
		virtual void listenForever(BaseInbound *previousHop) = 0;

		// Inbound.listenForever MUST initialize this field. 
		sockfd_t ipcPipe = -1;
	};

	struct BaseInbound : rlib::noncopyable {
		BaseInbound(string inboundConfig) {
			loadConfig(inboundConfig);
		}
		virtual ~BaseInbound() = default;

		// Init data structures.
		virtual void loadConfig(string config) = 0;

		// OutboundThread calls this function. Wake up 'listenForever' thread, and send back a message. Outbound provides the senderId.
		virtual void forwardMessageToOutbound(string binaryMessage, string senderId) = 0;

		// Listen the addr:port in config, for inbound connection.
		// Also listen the accepted connection fileDescriptors, and listen the PIPE.
		virtual void listenForever(BaseOutbound *nextHop) = 0;

		// Inbound.listenForever MUST initialize this field. 
		sockfd_t ipcPipe = -1;
	};

	// TODO: PIPE only works on linux epoll. The windows epoll only works on SOCKET. 
	//       Do this if you would like to support windows. 
}

#endif



