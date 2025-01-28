#pragma once

#include <gtb/BotContext.h>

#include <gtb/BtcPrice.h>
#include <gtb/CoinbaseWallet.h>

#include <mutex>

namespace gtb
{

/**
 * Write wallet value updates to the database
 */
class WalletHistoricalWriter
{
    public:
        WalletHistoricalWriter(
            BotContext &ctx);
        WalletHistoricalWriter(WalletHistoricalWriter &&) = delete;
        WalletHistoricalWriter(const WalletHistoricalWriter &) = delete;
        WalletHistoricalWriter &operator=(WalletHistoricalWriter &&) = delete;
        WalletHistoricalWriter &operator=(const WalletHistoricalWriter &) = delete;
        ~WalletHistoricalWriter() = default;

        void process(
            const BtcPrice &price);

        void process(
            const CoinbaseWallet &wallet);

    private:
        void write(
            const std::lock_guard<std::mutex> &lock);

        BotContext &ctx;

        std::mutex mtx;
        uint64_t prevTime;
};

}
