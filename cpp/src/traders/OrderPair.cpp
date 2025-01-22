#include <gtb/OrderPair.h>

std::string gtb::to_string(OrderPair::State e)
{
    switch (e)
    {
        default:
        case OrderPair::State::None: break;
        case OrderPair::State::Pending: return "Pending";
        case OrderPair::State::BuyActive: return "BuyActive";
        case OrderPair::State::Holding: return "Holding";
        case OrderPair::State::SellActive: return "SellActive";
        case OrderPair::State::Complete: return "Complete";
        case OrderPair::State::Canceled: return "Canceled";
        case OrderPair::State::Error: return "Error";
    }

    return "None";
}

void gtb::from_string(const std::string &str, OrderPair::State &e)
{
    if (str == "Pending")
        e = OrderPair::State::Pending;
    else if (str == "BuyActive")
        e = OrderPair::State::BuyActive;
    else if (str == "Holding")
        e = OrderPair::State::Holding;
    else if (str == "SellActive")
        e = OrderPair::State::SellActive;
    else if (str == "Complete")
        e = OrderPair::State::Complete;
    else if (str == "Canceled")
        e = OrderPair::State::Canceled;
    else if (str == "Error")
        e = OrderPair::State::Error;
    else
        e = OrderPair::State::None;
}
