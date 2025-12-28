#include <gtb/TradeBot.h>
#include <gtb/ArgParser.h>
#include <gtb/AlgorithmFactory.h>
#include <gtb/Log.h>

namespace
{

constexpr const unsigned int DEFAULT_VERSION = 2;

}

int main(int argc, const char *argv[])
{
    // Setup argument parser
    gtb::ArgParser parser;
    parser.addCategory("Algorithm");
    parser.addParam('v', "version", "Algorithm version to use (default %u)", DEFAULT_VERSION);

    parser.addCategory("Testing");
    parser.addSwitch('m', "mock", "Run the algorithm against historical data");
    parser.addSwitch("trade-logs", "Enable trade-level logging (default: disabled)");
    parser.addSwitch("debug-logs", "Enable debug-level logging (off by default)");
    parser.addCategory("Logging");
    parser.addParam("log-file", "Write logs to this file (also prints to stdout)");

    // Parse
    gtb::Args args = parser.parse(argc, argv);
    if (args.hasError())
        return 1;

    // Logging
    if (args.hasArg("log-file"))
        gtb::log::setLogFile(args.getArg("log-file"));
    if (args.hasArg("trade-logs"))
        gtb::log::setTradeLoggingEnabled(true);
    if (args.hasArg("debug-logs"))
        gtb::log::setDebugLoggingEnabled(true);

    // Setup tradebot
    unsigned int version = DEFAULT_VERSION;
    if (args.hasArg("version"))
        version = args.getUInt("version");

    bool mock = false;
    if (args.hasArg("mock"))
        mock = true;

    gtb::TradeBot bot;
    if (!gtb::AlgorithmFactory::provision(bot, version, mock))
        return 2;

    return bot.run();
}
