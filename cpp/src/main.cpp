#include <gtb/TradeBot.h>
#include <gtb/Version1.h>

int main(int argc, const char *argv[])
{
    (void)argc;
    (void)argv;

    gtb::TradeBot bot;
    gtb::Version1::init(bot);
    return bot.run();
}
