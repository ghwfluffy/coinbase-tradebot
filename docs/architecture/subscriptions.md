# Subscriptions

Overview
- The bot has a single DataController instance inside BotContext (one per running bot). The DataController is the central registry for all runtime DataModel instances and the subscription lists for each model type.
- DataModel is a lightweight base class that concrete models (e.g., BtcPrice, CoinbaseOrderBook, CoinbaseWallet) derive from. Models own their state and call updated() when their state changes.
- Subscribers are objects that implement a process(const T&) method for a specific DataModel type T. They register with DataController::subscribe<T>(processor) to receive notifications when T changes.

Key components
- BotContext (src/core/BotContext.h): holds the DataController and the ActionThreadPool. This makes the DataController available via dependency injection to components that receive the BotContext.
- DataController (src/core/DataController.h): stores DataModel instances, subscription lists keyed by type id, and an ActionQueue reference used to schedule notifications.
- DataModel (src/core/DataModel.h/.cpp): stores a single listener callback (set by DataController when a model is created). When a model's state changes it calls updated(), which invokes that listener.
- ActionQueue / ActionThreadPool: notifications are queued through the ActionQueue so subscriber processing runs on the action pool rather than directly inside the model change.

How it works (notification flow)
1. A component obtains or lazily creates a model via DataController::get<T>() which will call initData(T()) if needed. initData constructs the model and sets its listener to DataController::updated<T>().
2. A subscriber registers with DataController::subscribe<T>(proc). subscribe binds a callback that will call proc.process(const T&) and stores that callback in the subscription list for type T.
3. When a model instance changes (for example BtcPrice::setPrice or CoinbaseOrderBook::update), the model calls updated().
4. DataModel::updated() calls the listener that DataController attached. DataController::updated<T>() iterates the subscription callbacks for T and enqueues each into the ActionQueue.
5. When the ActionQueue runs the enqueued callback it calls DataController::notify(const T&, proc) which invokes proc.process(data). The subscriber receives the latest model state as a const reference and can react accordingly.

Example sequence (BTC price update)
```cpp
// src/models/BtcPrice.cpp
// inside BtcPrice::setPrice
if (this->price != price)
{
    this->price = price;
    updated(); // triggers notification path
}
```

```cpp
// src/core/DataController.h
// DataController::initData sets the model listener:
model->setListener(std::bind(&DataController::updated<DataType>, this));

// DataController::subscribe creates a bound callback that will call proc.process(data)
dataController.subscribe<BtcPrice>(myProcessor);
```

```cpp
// src/algorithms/ExampleProcessor.h
struct ExampleProcessor
{
    // called when BtcPrice changes
    void process(const gtb::BtcPrice &price)
    {
        // inspect price.getPrice() and react
    }
};
```

Notes and guarantees
- Type mapping: DataController keys models and subscription lists by a TypeInfo id (TypeInfo::getId<T>()), so subscriptions are per-data-type.
- Deferred execution: DataModel::updated() triggers enqueueing of subscriber callbacks via ActionQueue. Subscriber processing runs on the action/thread pool — this prevents doing heavy work synchronously inside the model update.
- Thread-safety: individual DataModel implementations are responsible for internal synchronization (e.g., CoinbaseOrderBook uses a mutex when mutating/accessing orders). DataController itself only queues callbacks and holds maps keyed by type id.
- Subscriber interface: subscribers must expose a process(const T&) method matching the model type they subscribe to.
- Lifecycle: subscribing calls DataController::get<T>() under the hood (through subscribe) if the model isn't already initialized, so models are lazily constructed.

This pattern provides a simple publish/subscribe mechanism: models publish "I changed" by calling updated(), the DataController converts that into queued notifications, and subscribers receive typed, read-only references to the updated model state via process(const T&).
