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



class Forwarder {
public:
    Forwarder(const rlib::string &inboundConfig, const rlib::string &outboundConfig, const std::list<rlib::string> &filterConfigs) {
        if (inboundConfig.starts_with("plain"))
            ptrInbound = new Protocols::PlainInbound;
        else if (inboundConfig.starts_with("misc"))
            ptrInbound = nullptr; // TODO
        else
            throw std::invalid_argument("Unknown protocol in inboundConfig " + inboundConfig);
        ptrInbound->loadConfig(inboundConfig);

        if (outboundConfig.starts_with("plain"))
            ptrOutbound = new Protocols::PlainOutbound;
        else if (outboundConfig.starts_with("misc"))
            ptrOutbound = nullptr; // TODO
        else
            throw std::invalid_argument("Unknown protocol in outboundConfig " + outboundConfig);
        ptrOutbound->loadConfig(outboundConfig);


        std::list<Filters::BaseFilter*> chainedFilters;
        for (auto &&filterConfig : filterConfigs) {
            Filters::BaseFilter *newFilter = nullptr;
            if (filterConfig.starts_with("aes"))
                newFilter = new Filters::AESFilter();
            else if (filterConfig.starts_with("xor"))
                newFilter = new Filters::XorFilter(); // these filters were not deleted. just a note.
            else
                throw std::invalid_argument("Unknown filter in filterConfig item: " + filterConfig);

            newFilter->loadConfig(filterConfig);
            chainedFilters.push_back(newFilter);
        }

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
