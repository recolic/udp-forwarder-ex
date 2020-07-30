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
		explicit ChainedFilters(std::list<Filters::BaseFilter*>&& chainedFilters)
			: chainedFilters(chainedFilters) {}

		// Usually the encrypt/encode/obfs function.
		virtual string convertForward(string binaryDatagram) override {
			for (auto* filterPtr : chainedFilters) {
				binaryDatagram = filterPtr->convertForward(binaryDatagram);
			}
			return binaryDatagram;
		}

		// Usually the decrypt/decode/de-obfs function.
		virtual string convertBackward(string binaryDatagram) override {
			for (auto iter = chainedFilters.rbegin(); iter != chainedFilters.rend(); ++iter) {
				binaryDatagram = (*iter)->convertForward(binaryDatagram);
			}
			return binaryDatagram;
		}

		std::list<Filters::BaseFilter*> chainedFilters;
	};

	struct ReversedFilter : public BaseFilter {
		explicit ReversedFilter(BaseFilter* real, bool I_should_delete = false) : real(real), I_should_delete(I_should_delete) {}

		virtual ~ReversedFilter() {
			if (I_should_delete)
				delete real;
		}

		// Usually the encrypt/encode/obfs function.
		virtual string convertForward(string binaryDatagram) override {
			return real->convertBackward(binaryDatagram);
		}

		// Usually the decrypt/decode/de-obfs function.
		virtual string convertBackward(string binaryDatagram) override {
			return real->convertForward(binaryDatagram);
		}

		bool I_should_delete = false;
		BaseFilter* real;
	};

}

#endif



