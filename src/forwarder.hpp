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
            ptrInbound = new Protocols::PlainInbound(inboundConfig);
        else if (inboundConfig.starts_with("misc"))
            ptrInbound = nullptr; // TODO

        if (outboundConfig.starts_with("plain"))
            ptrOutbound = nullptr; // TODO
        else if (outboundConfig.starts_with("misc"))
            ptrOutbound = nullptr; // TODO
    }

    ~Forwarder() {
        if (ptrInbound) delete ptrInbound;
        if (ptrOutbound) delete ptrOutbound;
    }


private:
    Protocols::BaseInbound *ptrInbound;
    Protocols::BaseOutbound *ptrOutbound;
};

#endif
