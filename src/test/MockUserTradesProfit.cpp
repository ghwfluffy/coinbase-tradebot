#include <gtest/gtest.h>

#include <gtb/Profits.h>
#include <gtb/IntLiterals.h>

using namespace gtb;

// Positive profit scenario
TEST(MockUserTrades_Profits, PositiveProfitCalculation)
{
    Profits p;
    // purchased, sold, buyFees, sellFees are in pico-dollars
    usd_t purchased = 10_Dollars;
    usd_t sold      = 15_Dollars;
    usd_t buyFees   = 10_Cents;
    usd_t sellFees  = 15_Cents;

    p.addOrderPair(purchased, sold, buyFees, sellFees);

    int32_t profitCents = p.getProfit();
    EXPECT_EQ(profitCents, (4_Dollars + 75_Cents) / 1_Cents);
}

// Negative profit scenario
TEST(MockUserTrades_Profits, NegativeProfitCalculation)
{
    Profits p;
    // purchased > sold to force a loss
    usd_t purchased = 15_Dollars;
    usd_t sold      = 10_Dollars;
    usd_t buyFees   = 1_Dollars;
    usd_t sellFees  = 1_Dollars;

    p.addOrderPair(purchased, sold, buyFees, sellFees);

    int32_t profitCents = p.getProfit();
    EXPECT_EQ(profitCents, static_cast<int64_t>(7_Dollars / 1_Cents) * -1);
}
