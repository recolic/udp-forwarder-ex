#ifndef UDP_FORWARDER_PROT_BASE_HPP_
#define UDP_FORWARDER_PROT_BASE_HPP_ 1

#include <rlib/class_decorator.hpp>
#include <string>
using std::string;

namespace Protocols {
	class BaseHandler : rlib::noncopyable {
		BaseHandler(string outboundConfig) {
			loadConfig(outboundConfig);
		}
		virtual ~BaseOutboundHandler = default;

		// Interfaces
		virtual loadConfig(string config) = 0;
		virtual handleMessage(string binaryMessage) = 0;
	};

	class BaseListener : rlib::noncopyable {
		BaseListener(string inboundConfig) {
			loadConfig(inboundConfig);
		}
		virtual loadConfig(string config) = 0;
		virtual listenForever(BaseHandler *nextHop) = 0;
	};
}

#endif



