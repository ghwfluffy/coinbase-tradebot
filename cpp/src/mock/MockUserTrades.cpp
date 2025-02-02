#include <gtb/MockUserTrades.h>
#include <gtb/MockLock.h>

#include <gtb/Log.h>

#include <gtb/CoinbaseOrderBook.h>
#include <gtb/CoinbaseWallet.h>
#include <gtb/CoinbaseFeeTier.h>

using namespace gtb;

namespace
{

constexpr const uint64_t SATOSHI_PER_BTC = 100'000'000ULL;
constexpr const uint64_t PICODOLLARS_PER_CENT = 100'000'000'000ULL;

// Input values:
// spend: total funds in cents
// price: price of 1 BTC in cents
// feeTier: fee in basis points (e.g. 35 means 0.35%)

// Output values:
// purchased: number of satoshis purchased
// beforeFees: cents spent on satoshis
// fees: cents spent on fees
void calcBuyFees(
    uint32_t spend,
    uint32_t price,
    uint32_t feeTier,
    uint64_t &purchased,
    uint64_t &beforeFees,
    uint64_t &fees)
{
    // The fee fraction is feeTier/10000. Thus, the effective multiplier is (1+feeTier/10000) = (10000+feeTier)/10000.
    // If you buy n satoshis, the cost in cents is (price * n)/SATOSHI_PER_BTC.
    // Total cost (in cents) including fee is:
    //    total = cost * (10000+feeTier) / 10000.
    // We require total <= spend.
    //
    // Rearranging:
    //    (price * n / SATOSHI_PER_BTC) * (10000+feeTier) <= spend * 10000
    // so a maximal n is given by:
    purchased = (spend * 10000ULL * SATOSHI_PER_BTC) / (price * (10000ULL+feeTier));

    // Actual cost (in cents) of the BTC purchase:
    uint64_t cost_cents = (price * purchased) / SATOSHI_PER_BTC;
    // Fee (in cents):
    uint64_t fee_cents = (cost_cents * feeTier) / 10000;
    // Total spent (in cents):
    uint64_t total_cents = cost_cents + fee_cents;

    // Convert to picodollars
    fees = fee_cents * PICODOLLARS_PER_CENT;
    beforeFees = total_cents * PICODOLLARS_PER_CENT;
}

// Input values:
// quantity: number of satoshis to sell
// price: price of 1 BTC in cents
// feeTier: fee in basis points (e.g. 35 means 0.35%)

// Output values:
// beforeFees: picodollars received from sale (not including fees)
// fees: picodollars paid in fees
void calcSellFees(
    uint64_t quantity,
    uint32_t price,
    uint32_t feeTier,
    uint64_t &beforeFees,
    uint64_t &fees)
{
    // Gross sale proceeds in cents.
    beforeFees = price * quantity * (PICODOLLARS_PER_CENT / SATOSHI_PER_BTC);

    // Fee is a percentage of the gross sale.
    fees = (beforeFees * feeTier) / 10000;
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
    uint32_t feeTier = ctx.coinbase().getFeeTier();

    // Will make updates
    std::list<CoinbaseOrder> updates;
    CoinbaseWallet::Data walletData = wallet.getData();

    // Check if any open orders are filled
    std::map<std::string, CoinbaseOrder> orders = orderBook.getOrders();
    for (auto &[uuid, order] : orders)
    {
        if (order.state != CoinbaseOrder::State::Open)
            continue;
        if (order.buy && order.priceCents < price.getCents())
            continue;
        if (!order.buy && order.priceCents > price.getCents())
            continue;

        // Update wallet values
        if (order.buy)
        {
            uint64_t quantity = 0;
            calcBuyFees(
                order.valueCents(),
                order.priceCents,
                feeTier,
                quantity,
                order.beforeFees,
                order.fees);

            // USD no longer on hold
            if (order.valueCents() <= walletData.onHoldUsd)
            {
                walletData.onHoldUsd -= order.valueCents();
            }
            else
            {
                log::error("Wallet does not have matching on hold USD for transaction (%llu vs %llu).",
                    static_cast<unsigned long long>(order.valueCents()),
                    static_cast<unsigned long long>(walletData.onHoldUsd));
                walletData.onHoldUsd = 0;
            }

            // USD is spent (Round Up)
            uint32_t minusUsd = static_cast<uint32_t>(
                (order.beforeFees + order.fees + PICODOLLARS_PER_CENT - 1) / PICODOLLARS_PER_CENT);
            if (minusUsd <= walletData.usd)
            {
                walletData.usd -= minusUsd;
            }
            else
            {
                log::error("Wallet does not have matching USD for transaction.");
                walletData.usd = 0;
            }

            // Gained bitcoin
            walletData.btc += quantity;
            order.quantity = quantity;
        }
        else
        {
            calcSellFees(
                order.quantity,
                order.priceCents,
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
                    static_cast<unsigned long long>(order.quantity),
                    static_cast<unsigned long long>(walletData.onHoldBtc));
                walletData.onHoldBtc = 0;
            }

            // BTC is sold
            if (order.quantity <= walletData.btc)
            {
                walletData.btc -= order.quantity;
            }
            else
            {
                log::error("Wallet does not have matching BTC for transaction.");
                walletData.btc = 0;
            }

            // Gained money (Round down)
            walletData.usd += static_cast<uint32_t>(order.beforeFees / PICODOLLARS_PER_CENT);
        }

        // Update order as complete
        order.state = CoinbaseOrder::State::Filled;
        order.cleanupTime = SteadyClock::now() + std::chrono::minutes(2);
        updates.push_back(order);
    }

    // Update state
    if (!updates.empty())
    {
        orderBook.update(updates);
        wallet.update(walletData);
    }
}
