# Whitespace & Formatting Examples

- Indent with 4 spaces (no tabs).
- Prefer blank lines between logical blocks; avoid trailing whitespace.
- Break long parameter lists one-per-line, aligned under the first argument.
- Allman braces: opening brace on its own line for functions and control blocks.
- Brace even single-line bodies for clarity.

Function definition (arguments one-per-line in declarations/definitions)
```cpp
void ActionThreadPool::queue(
    std::function<void()> action)
{
    std::lock_guard<std::mutex> lock(mtx);

    Action a;
    a.action = std::move(action);
    actions.push_back(std::move(a));

    cond.notify_one();
}
```

Control flow
```cpp
if (mock)
    initMock(bot);
else
    initProd(bot);
```

Calls with many arguments (split one-per-line, aligned under the first argument)
```cpp
OrderPair pair = OrderPairMarketEngine::newSpread(
    conf,
    price.getPrice(),
    ctx.data.get<Time>().getTime(),
    conf.spread);
```

Constructor layout
```cpp
SpreadTrader::SpreadTrader(
    BotContext &ctx,
    Config confIn)
        : OrderPairTrader(ctx, confIn)
        , conf(std::move(confIn))
{
}
```

Switch/cases
```cpp
switch (version)
{
    case 1:
        Version1::init(bot, mock);
        return true;
    case 2:
        Version2::init(bot, mock);
        return true;
    default:
        log::error("Invalid algorithm version %u.", version);
        break;
}
```
