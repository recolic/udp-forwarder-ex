#ifndef UDP_FWD_FILTER_XOR_
#define UDP_FWD_FILTER_XOR_ 1

#include "filters/base.hpp"
#include <unordered_map>
#include <rlib/string.hpp>
#include "utils.hpp"

namespace Filters {
	template <size_t BlockSize = 32>
	class XorFilter : public BaseFilter {
	public:
		virtual void loadConfig(string config) override {
			auto ar = rlib::string(config).split('@'); // Also works for ipv6.
			if (ar.size() != 2)
				throw std::invalid_argument("Wrong parameter string for protocol 'plain'. Example:    XOR@MyPassword");
			key = pskToKey<BlockSize>(ar[1]);
		}

		// Encrypt
		virtual string convertForward(string datagram) override {
			auto curr_key_digit = 0;
			for (auto offset = 0; offset < datagram.size(); ++offset) {
				datagram[0] ^= key[curr_key_digit++];
			}
			return datagram;
		}

		// Decrypt
		virtual string convertBackward(string datagram) override {
			return convertForward(datagram);
		}

	private:
		string key;


	};
}

#endif

