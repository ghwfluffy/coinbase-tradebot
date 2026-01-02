#include <gtb/CoinbaseOrder.h>

std::string gtb::to_string(CoinbaseOrder::State state)
{
    switch (state)
    {
        case CoinbaseOrder::State::Open: return "Open";
        case CoinbaseOrder::State::Filled: return "Filled";
        case CoinbaseOrder::State::Canceled: return "Canceled";
        case CoinbaseOrder::State::Error: return "Error";
        default:
        case CoinbaseOrder::State::None:
            break;
    }

    return "None";
}
