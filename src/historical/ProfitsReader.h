#pragma once

#include <gtb/BotContext.h>

namespace gtb
{

/**
 * Initialize profits from previous run using database
 */
namespace ProfitsReader
{
    void initProfits(
        BotContext &ctx);
}

}
