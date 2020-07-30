#ifndef UDP_FWD_FILTER_AES_
#define UDP_FWD_FILTER_AES_ 1

#include "filters/base.hpp"
#include "lib/plusaes.hpp"
#include <rlib/string.hpp>
#include <unordered_map>
#include "utils.hpp"

namespace Filters {
	enum class AESFilterMode {
		CBC,
		ECB,
		CTR
	};

	template <size_t BlockSize = 128, AESFilterMode mode = AESFilterMode::CBC>
	class AESFilter : public BaseFilter {
	public:
		virtual void loadConfig(string config) override {
			auto ar = rlib::string(config).split('@'); // Also works for ipv6.
			if (ar.size() != 2)
				throw std::invalid_argument("Wrong parameter string for filter 'aes'. Example: aes@MyPassword");
			auto tmpKey = pskToKey<16>(ar[1]);
			char aesKey[17] = {0};
			static_assert(BlockSize == 128, "TODO: Change key size on compilation time to support more AES algo. ");
			std::copy(tmpKey.begin(), tmpKey.end(), std::begin(aesKey));
			key = plusaes::key_from_string(&aesKey);
		}

		static_assert(mode == AESFilterMode::CBC || mode == AESFilterMode::ECB, "Only supporting CBC and ECB");

		// Encrypt
		virtual string convertForward(string datagram) override {
			const unsigned long encrypted_size = plusaes::get_padded_encrypted_size(datagram.size());
			std::string result(encrypted_size, '\0');
			if constexpr (mode == AESFilterMode::CBC) {
				plusaes::encrypt_cbc((unsigned char *)datagram.data(), datagram.size(), key.data(), key.size(), &iv, (unsigned char *)result.data(), result.size(), true);
			}
			if constexpr (mode == AESFilterMode::ECB) {
				plusaes::encrypt_ecb((unsigned char *)datagram.data(), datagram.size(), key.data(), key.size(), (unsigned char *)result.data(), result.size(), true);
			}
			return result;
		}

		// Decrypt
		virtual string convertBackward(string datagram) override {
			std::string result(datagram.size(), '\0');
			unsigned long padded_size = 0;
			if constexpr (mode == AESFilterMode::CBC) {
				plusaes::decrypt_cbc((unsigned char *)datagram.data(), datagram.size(), key.data(), key.size(), &iv, (unsigned char *)result.data(), result.size(), &padded_size);
			}
			if constexpr (mode == AESFilterMode::ECB) {
				plusaes::decrypt_ecb((unsigned char *)datagram.data(), datagram.size(), key.data(), key.size(), (unsigned char *)result.data(), result.size(), &padded_size);
			}

			return result.substr(0, datagram.size() - padded_size);
		}

	private:
		std::vector<unsigned char> key;

		static_assert(BlockSize == 128, "TODO: Change IV size on compilation time to support more AES algo. ");
		unsigned char iv[16] = {
			0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
			0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		};

	};
}

#endif

