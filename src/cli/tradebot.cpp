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
    parser.addSwitch("no-trade-logs", "Disable trade-level logging (default: enabled)");

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

    bool tradeLogs = true;
    if (args.hasArg("no-trade-logs"))
        tradeLogs = false;
    gtb::log::setTradeLoggingEnabled(tradeLogs);

    gtb::TradeBot bot;
    if (!gtb::AlgorithmFactory::provision(bot, version, mock))
        return 2;

    return bot.run();
}
