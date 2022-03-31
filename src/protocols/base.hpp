#ifndef UDP_FORWARDER_PROT_BASE_HPP_
#define UDP_FORWARDER_PROT_BASE_HPP_ 1

#include "filters/base.hpp"
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
    // All protocol implementation must talk to each other with this Internal Bridge.
    // This bridge is used to carry message from Inbound to Outbound in the same process.
    struct InternalBridge : rlib::noncopyable {
        explicit InternalBridge(Filters::BaseFilter *ptrFilter) : ptrFilter(ptrFilter) {
            std::tie(ipcPipeInboundSide, ipcPipeOutboundSide) = mk_tcp_pipe();
        }

        void forwardInboundToOutbound(string binaryMessage, string senderId) {
            // Inbound calls this function, to alert the Outbound thread to forward a message.
            rlib::sockIO::send_msg(ipcPipeOutboundSide, senderId);
            rlib::sockIO::send_msg(ipcPipeOutboundSide, ptrFilter->convertForward(binaryMessage));
        }

        void forwardOutboundToInbound(string binaryMessage, string senderId) {
            // Outbound calls this function, to alert the Inbound thread to forward a message.
            rlib::sockIO::send_msg(ipcPipeInboundSide, senderId);
            rlib::sockIO::send_msg(ipcPipeInboundSide, ptrFilter->convertBackward(binaryMessage));
        }

        sockfd_t ipcPipeInboundSide = -1;
        sockfd_t ipcPipeOutboundSide = -1;
        Filters::BaseFilter *ptrFilter = nullptr;
    };

	struct BaseInbound;

	// Outbound holds the senderId=>nextHopFd mapping.
	// senderId is "$ip@$port", for example, `fe80:8100::1@1080`. 
	// Note: this interface works for both TCP and UDP.
	struct BaseOutbound : rlib::noncopyable {
		virtual ~BaseOutbound() = default;

		// Init data structures.
		virtual void loadConfig(string config) {}

		// Listen the PIPE. handleMessage will wake up this thread from epoll.
		// Also listen the connection fileDescriptors.
		virtual void listenForever(BaseInbound *previousHop, InternalBridge *bridge) = 0;
	};

	struct BaseInbound : rlib::noncopyable {
		virtual ~BaseInbound() = default;

		// Init data structures.
		virtual void loadConfig(string config) {}

		// Listen the addr:port in config, for inbound connection.
		// Also listen the accepted connection fileDescriptors, and listen the PIPE.
		virtual void listenForever(BaseOutbound *nextHop, InternalBridge *bridge) = 0;
	};

	// TODO: PIPE only works on linux epoll. The windows epoll only works on SOCKET.
	//       Do this if you would like to support windows. 
}

#endif



