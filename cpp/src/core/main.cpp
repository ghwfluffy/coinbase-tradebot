#include <gtb/TradeBot.h>
#include <gtb/Version1.h>

int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    const bool mock = false;

    gtb::TradeBot bot;
    gtb::Version1::init(bot, mock);
    return bot.run();
}
