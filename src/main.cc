#include <rlib/stdio.hpp>
#include <rlib/opt.hpp>
#include "common.hpp"

rlib::logger rlog(std::cerr);

using namespace rlib::literals;

int main(int argc, char **argv) {
    rlib::opt_parser args(argc, argv);
    if(args.getBoolArg("--help", "-h")) {
        rlib::println("Usage: {} -i $InboundConfig -o $OutboundConfig [--log=error/info/verbose/debug]"_rs.format(args.getSelf()));
        rlib::println("  InboundConfig and OutboundConfig are in this format: ");
        rlib::println("  '$method:$params', available methods: ");
        rlib::println("  'plain:$addr:$port', 'misc:$addr:$portRange:$psk'");
        return 0;
    }
    auto inboundConfig = args.getValueArg("-i");
    auto outboundConfig = args.getValueArg("-o");
    auto log_level = args.getValueArg("--log", false, "info");

    if(log_level == "error")
        rlog.set_log_level(rlib::log_level_t::ERROR);
    else if(log_level == "info")
        rlog.set_log_level(rlib::log_level_t::INFO);
    else if(log_level == "verbose")
        rlog.set_log_level(rlib::log_level_t::VERBOSE);
    else if(log_level == "debug")
        rlog.set_log_level(rlib::log_level_t::DEBUG);
    else
        throw std::runtime_error("Unknown log level: " + log_level);

    // Forwarder(inboundConfig, outboundConfig).run_forever();

    return 0;
}
