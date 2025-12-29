#include <gtb/MockUserTrades.h>
#include <gtb/MockLock.h>

#include <gtb/Log.h>
#include <gtb/IntegerUtils.h>
#include <gtb/Profits.h>
#include <gtb/CoinbaseFeeTier.h>
#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseWallet.h>

using namespace gtb;

namespace
{

// Input values:
// spend: Amount to spend
// price: BTC price
// feeTier: Coinbase trade fee

// Output values:
// purchased: Number of bitcoins purchased
// beforeFees: Amount spent on bitcoin
// fees: Amount spent on fees
void calcBuyFees(
    usd_t spend,
    usd_t price,
    pp_t feeTier,
    btc_t &purchased,
    usd_t &beforeFees,
    usd_t &fees)
{
    usd_t maxSpend = spend / (100_Percent + feeTier);
    purchased = IntegerUtils::getSatoshiForPrice(price, maxSpend);
    beforeFees = IntegerUtils::getValue(price, purchased);
    fees = beforeFees * feeTier;
}

// Input values:
// quantity: Amount of BTC to sell
// price: Price of BTC
// feeTier: Coinbase trade fee

// Output values:
// beforeFees: Amount received from sale (before paying fees)
// fees: Amount paid in fees
void calcSellFees(
    btc_t quantity,
    usd_t price,
    pp_t feeTier,
    usd_t &beforeFees,
    usd_t &fees)
{
    // Gross sale proceeds
    beforeFees = IntegerUtils::getValue(price, quantity);

    // Fee is a percentage of the gross sale.
    fees = beforeFees * feeTier;
}

}

MockUserTrades::MockUserTrades(
    BotContext &ctx)
        : ctx(ctx)
{
    ctx.data.subscribe<BtcPrice>(*this);
}

void MockUserTrades::process(
    const BtcPrice &price)
{
    auto lock = ctx.data.get<MockLock>().lock();

    CoinbaseOrderBook &orderBook = ctx.data.get<CoinbaseOrderBook>();
    CoinbaseWallet &wallet = ctx.data.get<CoinbaseWallet>();
    pp_t feeTier = ctx.coinbase().getFeeTier();
    ctx.data.get<CoinbaseFeeTier>().setFeeTier(feeTier);

    // Will make updates
    std::list<CoinbaseOrder> updates;
    CoinbaseWallet::Data walletData = wallet.getData();

    // Check if any open orders are filled
    std::map<std::string, CoinbaseOrder> orders = orderBook.getOrders();
    for (auto &[uuid, order] : orders)
    {
        if (order.state != CoinbaseOrder::State::Open)
            continue;
        if (order.buy && order.price < price.getPrice())
            continue;
        if (!order.buy && order.price > price.getPrice())
            continue;

        // Update wallet values
        if (order.buy)
        {
            btc_t quantity;
            calcBuyFees(
                order.value(),
                order.price,
                feeTier,
                quantity,
                order.beforeFees,
                order.fees);

            // USD no longer on hold
            if (order.value() <= walletData.onHoldUsd)
            {
                walletData.onHoldUsd -= order.value();
            }
            else
            {
                log::error("Wallet does not have matching on hold USD for transaction (%llu vs %llu).",
                    static_cast<unsigned long long>(order.value().value()),
                    static_cast<unsigned long long>(walletData.onHoldUsd.value()));
                walletData.onHoldUsd = {};
            }

            // USD is spent (Round Up)
            usd_t minusUsd = order.beforeFees + order.fees;
            if (minusUsd <= walletData.usd)
            {
                walletData.usd -= minusUsd;
            }
            else
            {
                log::error("Wallet does not have matching USD for transaction.");
                walletData.usd = {};
            }

            // Gained bitcoin
            walletData.btc += quantity;
            order.quantity = quantity;
        }
        else
        {
            calcSellFees(
                order.quantity,
                order.price,
                feeTier,
                order.beforeFees,
                order.fees);

            // BTC no longer on hold
            if (order.quantity <= walletData.onHoldBtc)
            {
                walletData.onHoldBtc -= order.quantity;
            }
            else
            {
                log::error("Wallet does not have matching on hold BTC for transaction (%llu vs %llu).",
                    static_cast<unsigned long long>(order.quantity.value()),
                    static_cast<unsigned long long>(walletData.onHoldBtc.value()));
                walletData.onHoldBtc = {};
            }

            // BTC is sold
            if (order.quantity <= walletData.btc)
            {
                walletData.btc -= order.quantity;
            }
            else
            {
                log::error("Wallet does not have matching BTC for transaction.");
                walletData.btc = {};
            }

            // Gained money (Round down)
            walletData.usd += order.beforeFees;
        }

        // Update order as complete
        order.state = CoinbaseOrder::State::Filled;
        order.cleanupTime = SteadyClock::now() + std::chrono::minutes(2);
        // Record realized P&L
        if (order.buy)
            ctx.data.get<Profits>().addOrderPair(order.beforeFees, usd_t(), order.fees, usd_t());
        else
            ctx.data.get<Profits>().addOrderPair(usd_t(), order.beforeFees, usd_t(), order.fees);

        // Track rolling volume (count notional of the trade) and update fee tier model.
        ctx.coinbase().recordVolume(big_usd_t(order.beforeFees), ctx.data.get<Time>().getTime());
        updates.push_back(order);
    }

    // Update state
    if (!updates.empty())
    {
        orderBook.update(updates);
        wallet.update(walletData);
    }
}
