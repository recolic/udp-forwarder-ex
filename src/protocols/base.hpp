#ifndef UDP_FORWARDER_PROT_BASE_HPP_
#define UDP_FORWARDER_PROT_BASE_HPP_ 1

#include <rlib/class_decorator.hpp>
#include <string>
using std::string;

namespace Protocols {
	// Handler holds the senderId=>nextHopFd mapping.
	// senderId is "$ip@$port", for example, `fe80:8100::1@1080`. 
	// Misc protocol may use duplicateSenderId to work on port migration.
	// Any listener may use removeSenderId to disconnect a sender.
	// Note: this interface works for both TCP and UDP.
	struct BaseHandler : rlib::noncopyable {
		BaseHandler(string outboundConfig) {
			loadConfig(outboundConfig);
		}
		virtual ~BaseHandler = default;

		// Interfaces
		virtual void loadConfig(string config) = 0;
		virtual void handleMessage(string binaryMessage, string senderId) = 0;
		virtual void duplicateSenderId(string newSenderId, string oldSenderId) = 0;
		virtual void removeSenderId(string senderId) = 0;
	};

	struct BaseListener : rlib::noncopyable {
		BaseListener(string inboundConfig) {
			loadConfig(inboundConfig);
		}
		virtual ~BaseListener = default;

		virtual void loadConfig(string config) = 0;
		virtual void listenForever(BaseHandler *nextHop) = 0;
	};
}

#endif



