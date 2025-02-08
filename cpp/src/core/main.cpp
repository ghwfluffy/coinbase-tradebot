#include <gtb/TradeBot.h>
#include <gtb/Version1.h>

#include <string.h>

int main(int argc, const char *argv[])
{
    bool mock = false;
    if (argc > 1 && !strcmp(argv[1], "-m"))
        mock = true;

    gtb::TradeBot bot;
    gtb::Version1::init(bot, mock);
    return bot.run();
}
