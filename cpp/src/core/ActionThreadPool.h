#pragma once

#include <gtb/Any.h>
#include <gtb/ActionQueue.h>

#include <list>
#include <mutex>
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <functional>
#include <condition_variable>

namespace gtb
{

/**
 * Queue actions to be handled asynchronously from the calling thread
 */
class ActionThreadPool : public ActionQueue
{
    public:
        ActionThreadPool();
        ActionThreadPool(ActionThreadPool &&) = delete;
        ActionThreadPool(const ActionThreadPool &) = delete;
        ActionThreadPool &operator=(ActionThreadPool &&) = delete;
        ActionThreadPool &operator=(const ActionThreadPool &) = delete;
        ~ActionThreadPool() final;

        void start(unsigned int threads = 4);
        void stop();

        void queue(std::function<void()> action) final;
        void queue(std::function<void(Any)> action, Any mem) final;

    private:
        void runThread(size_t id);

        std::mutex mtx;
        std::atomic<bool> running;
        std::condition_variable cond;
        std::list<std::unique_ptr<std::thread>> threads;

        struct Action
        {
            Any mem;
            std::function<void()> action;
            std::function<void(Any)> anyAction;
        };
        std::list<Action> actions;
};

}
