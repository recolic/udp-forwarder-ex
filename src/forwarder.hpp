#ifndef UDP_FORWARDER_DYN_FORWARDER_HPP_
#define UDP_FORWARDER_DYN_FORWARDER_HPP_

#include <rlib/sys/sio.hpp>

#include "utils.hpp"
#include "protocols/base.hpp"
#include "protocols/plain.hpp"

using std::string;



class Forwarder {
public:
    Forwarder(const rlib::string &inboundConfig, const rlib::string &outboundConfig) {
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
    }

    ~Forwarder() {
        if (ptrInbound) delete ptrInbound;
        if (ptrOutbound) delete ptrOutbound;
    }

    [[noreturn]] void runForever() {
        std::thread([this] {ptrInbound->listenForever(ptrOutbound);}).detach();
        ptrOutbound->listenForever(ptrInbound); // Blocks
    }


private:
    Protocols::BaseInbound *ptrInbound;
    Protocols::BaseOutbound *ptrOutbound;
};

#endif
