#include <gtest/gtest.h>

#include <gtb/Profits.h>
#include <gtb/IntLiterals.h>

#include "GTestPrinters.h"

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

    EXPECT_EQ(p.getProfit().value().toInt64(), static_cast<int64_t>(usd_t(4_Dollars + 75_Cents).value()));
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

    EXPECT_EQ(p.getProfit().value().toInt64(), static_cast<int64_t>(usd_t(7_Dollars).value()) * -1L);
}
