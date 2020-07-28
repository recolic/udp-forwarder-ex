#ifndef UDP_FORWARDER_PROT_PLAIN_HPP_
#define UDP_FORWARDER_PROT_PLAIN_HPP_ 1

#include <protocols/base.hpp>
#include <rlib/sys/sio.hpp>
#include <rlib/string.hpp>

namespace Protocols {
	class PlainInboundListener : public BaseListener {
	public:
		virtual loadConfig(string config) override {
			auto ar = rlib::string(config).split('@'); // Also works for ipv6.
			if (ar.size() != 3)
				throw std::invalid_argument("Wrong parameter string for protocol 'plain'. Example:    plain@fe00:1e10:ce95:1@10809");
			auto listenAddr = ar[1];
			auto listenPort = ar[2].as<uint16_t>();

			listenFd = rlib::quick_listen(listenAddr, listenPort, true);
		}
		virtual listenForever(BaseHandler* nextHop) override {

		}

	private:
		fd_t listenFd;
	};

	using PlainOutboundListener = PlainInboundListener;

	class PlainOutboundHandler {

	};
}

#endif



