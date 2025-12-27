#include <gtb/TradeBot.h>
#include <gtb/ArgParser.h>
#include <gtb/AlgorithmFactory.h>
#include <gtb/Log.h>

namespace
{

constexpr const unsigned int DEFAULT_VERSION = 1;

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

    // Parse
    gtb::Args args = parser.parse(argc, argv);
    if (args.hasError())
        return 1;

    unsigned int version = DEFAULT_VERSION;
    if (args.hasArg("version"))
        version = args.getUInt("version");

    bool mock = false;
    if (args.hasArg("mock"))
        mock = true;

    bool tradeLogs = false;
    if (args.hasArg("trade-logs"))
        tradeLogs = true;
    gtb::log::setTradeLoggingEnabled(tradeLogs);
    if (args.hasArg("debug-logs"))
        gtb::log::setDebugLoggingEnabled(true);

    gtb::TradeBot bot;
    if (!gtb::AlgorithmFactory::provision(bot, version, mock))
        return 2;

    return bot.run();
}
