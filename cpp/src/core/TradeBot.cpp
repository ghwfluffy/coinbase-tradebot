#include <gtb/TradeBot.h>
#include <gtb/Log.h>

#include <unistd.h>
#include <signal.h>

using namespace gtb;

namespace
{

struct SignalState
{
    unsigned int &interrupts;
    std::function<void()> stop;

    SignalState(
        unsigned int &interrupts,
        std::function<void()> stop)
            : interrupts(interrupts)
            , stop(std::move(stop))
    {}
};

std::unique_ptr<SignalState> signalState;
void handleSignal(int signal)
{
    constexpr const unsigned int MAX_INTERRUPTS = 5;
    if (signal == SIGINT)
    {
        if (signalState->interrupts++ == 0)
        {
            log::info("Interrupt signal received. Shutting down gracefully.");
            signalState->stop();
        }
        else if (signalState->interrupts >= MAX_INTERRUPTS)
        {
            log::info("Immediate shutdown requested. Terminating immediately.");
            _Exit(128);
        }
    }
    else
    {
        log::info("Terminate signal received. Shutting down gracefully.");
        signalState->stop();
    }
}

}

TradeBot::TradeBot()
{
    running = false;
    initSignals();
    ctx.historicalDb.init("historical.sqlite", "./schema/historical.sql");
}

TradeBot::~TradeBot()
{
    cleanupSignals();
}

BotContext &TradeBot::getCtx()
{
    return ctx;
}

void TradeBot::addSource(std::unique_ptr<DataSource> source)
{
    if (source)
        sources.push_back(std::move(source));
}

int TradeBot::run()
{
    log::info("Starting Ghw Trade Bot.");
    running = true;

    ctx.actionPool.start();
    for (auto &source : sources)
        source->start();

    // Wait for signal
    while (running)
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (running)
            cond.wait_for(lock, std::chrono::seconds(60));
    }

    log::info("Stopping Ghw Trade Bot.");

    ctx.actionPool.stop();
    for (auto &source : sources)
        source->stop();

    log::info("Ghw Trade Bot Shutdown.");

    return 0;
}

void TradeBot::stop()
{
    running = false;
    cond.notify_one();
}

void TradeBot::initSignals()
{
    interrupts = 0;
    signalState = std::make_unique<SignalState>(
        interrupts,
        std::bind(&TradeBot::stop, this));

    // Catch SIGTERM/SIGINT
    struct sigaction handler = {};
    handler.sa_handler = handleSignal;
    sigaction(SIGTERM, &handler, nullptr);
    sigaction(SIGINT, &handler, nullptr);

    // Ignore SIGPIPE
    handler.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &handler, nullptr);
}

void TradeBot::cleanupSignals()
{
    struct sigaction handler = {};
    handler.sa_handler = SIG_DFL;

    sigaction(SIGTERM, &handler, nullptr);
    sigaction(SIGINT, &handler, nullptr);
    sigaction(SIGPIPE, &handler, nullptr);

    signalState.reset();
}
