#ifndef UDP_FORWARDER_FILT_BASE_HPP_
#define UDP_FORWARDER_FILT_BASE_HPP_ 1

#include <rlib/class_decorator.hpp>
#include <string>
#include <list>
#include <cstring>
using std::string;

/*
				UDP Forwarder
    |--------------------------------------|
---> Inbound ====|FILTER|===> Outbound ---> NextHop
    |        <===|FILTER|====              |
    |--------------------------------------|
*/

namespace Filters {
	struct BaseFilter : rlib::noncopyable {
		virtual ~BaseFilter() = default;

		// Init data structures.
		virtual void loadConfig(string config) {}

		// Usually the encrypt/encode/obfs function.
		virtual string convertForward(string binaryDatagram) = 0;

		// Usually the decrypt/decode/de-obfs function.
		virtual string convertBackward(string binaryDatagram) = 0;
	};

	struct ChainedFilters : public BaseFilter {
		ChainedFilters(const std::list<Filters::BaseFilter*>& chainedFilters)
			: chainedFilters(chainedFilters) {}

		// Usually the encrypt/encode/obfs function.
		virtual string convertForward(string binaryDatagram) override {
			for (auto* filterPtr : chainedFilters) {
				binaryDatagram = filterPtr->convertForward(binaryDatagram);
			}
			return binaryDatagram;
		}

		// Usually the decrypt/decode/de-obfs function.
		virtual string convertBackward(string binaryDatagram) {
			for (auto iter = chainedFilters.rbegin(); iter != chainedFilters.rend(); ++iter) {
				binaryDatagram = (*iter)->convertForward(binaryDatagram);
			}
			return binaryDatagram;
		}

		const std::list<Filters::BaseFilter*>& chainedFilters;
	};
}

#endif



