#ifndef UDP_FORWARDER_DYN_FORWARDER_HPP_
#define UDP_FORWARDER_DYN_FORWARDER_HPP_

#include <rlib/sys/sio.hpp>

#include "utils.hpp"
#include "protocols/base.hpp"
#include "protocols/plain.hpp"

#include "filters/base.hpp"
#include "filters/aes_encryption.hpp"
#include "filters/xor_encryption.hpp"

using std::string;


// This function parse the FilterConfig string for you. Register implemented modules here!
inline Filters::BaseFilter* CreateFilterFromConfig(rlib::string filterConfig) {
	Filters::BaseFilter *newFilter = nullptr;
    if (filterConfig.starts_with("aes@"))
        newFilter = new Filters::AESFilter();
    else if (filterConfig.starts_with("xor@"))
        newFilter = new Filters::XorFilter(); // these filters were not deleted. just a note.
	else if (filterConfig.starts_with("reverse@")) {
		auto newFilterConfig = filterConfig.substr(8);
		newFilter = new Filters::ReversedFilter(CreateFilterFromConfig(newFilterConfig), true);
	}
    else
        throw std::invalid_argument("Unknown filter in filterConfig item: " + filterConfig);

    newFilter->loadConfig(filterConfig);
    return newFilter;
}

class Forwarder {
public:
    Forwarder(const rlib::string &inboundConfig, const rlib::string &outboundConfig, const std::list<rlib::string> &filterConfigs) {
        if (inboundConfig.starts_with("plain"))
            ptrInbound = new Protocols::PlainInbound;
        else if (inboundConfig.starts_with("dynport"))
            ptrInbound = nullptr; // TODO
        else
            throw std::invalid_argument("Unknown protocol in inboundConfig " + inboundConfig);
        ptrInbound->loadConfig(inboundConfig);

        if (outboundConfig.starts_with("plain"))
            ptrOutbound = new Protocols::PlainOutbound;
        else if (outboundConfig.starts_with("dynport"))
            ptrOutbound = nullptr; // TODO
        else
            throw std::invalid_argument("Unknown protocol in outboundConfig " + outboundConfig);
        ptrOutbound->loadConfig(outboundConfig);


        std::list<Filters::BaseFilter*> chainedFilters;
        for (auto &&filterConfig : filterConfigs)
            chainedFilters.push_back(CreateFilterFromConfig(filterConfig));
        ptrFilter = new Filters::ChainedFilters(chainedFilters);
    }

    ~Forwarder() {
        if (ptrInbound) delete ptrInbound;
        if (ptrOutbound) delete ptrOutbound;
        if (ptrFilter) delete ptrFilter;
    }

    [[noreturn]] void runForever() {
        std::thread([this] {ptrInbound->listenForever(ptrOutbound, ptrFilter);}).detach();
        ptrOutbound->listenForever(ptrInbound, ptrFilter); // Blocks
    }


private:
    Protocols::BaseInbound *ptrInbound;
    Protocols::BaseOutbound *ptrOutbound;
    Filters::BaseFilter *ptrFilter;
};

#endif
