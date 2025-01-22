#include <gtb/ActionThreadPool.h>
#include <gtb/Log.h>

#include <string.h>
#include <pthread.h>

using namespace gtb;

ActionThreadPool::ActionThreadPool()
{
    running = false;
}

ActionThreadPool::~ActionThreadPool()
{
    stop();
}

void ActionThreadPool::start(unsigned int count)
{
    std::lock_guard<std::mutex> lock(mtx);
    if (running && count == threads.size())
        return;

    stop();

    log::info("Starting action thread pool.");

    running = true;
    while (threads.size() < count)
    {
        auto thread = std::make_unique<std::thread>(std::bind(&ActionThreadPool::runThread, this, threads.size()));
        threads.push_back(std::move(thread));
    }
}

void ActionThreadPool::stop()
{
    if (!running)
        return;

    log::info("Stopping action thread pool.");
    {
        std::lock_guard<std::mutex> lock(mtx);
        running = false;
        cond.notify_all();
    }

    while (!threads.empty())
    {
        threads.front()->join();
        threads.pop_front();
    }
}

void ActionThreadPool::queue(std::function<void()> action)
{
    std::lock_guard<std::mutex> lock(mtx);

    Action a;
    a.action = std::move(action);
    actions.push_back(std::move(a));

    cond.notify_one();
}

void ActionThreadPool::queue(std::function<void(Any)> action, Any mem)
{
    std::lock_guard<std::mutex> lock(mtx);

    Action a;
    a.mem = std::move(mem);
    a.anyAction = std::move(action);
    actions.push_back(std::move(a));

    cond.notify_one();
}

void ActionThreadPool::runThread(size_t id)
{
    char szName[32] = {};
    snprintf(szName, sizeof(szName), "Action %zu", id);
    pthread_setname_np(pthread_self(), szName);

    while (running)
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (!running)
            break;

        // Wait for an action
        if (actions.empty())
        {
            cond.wait_for(lock, std::chrono::seconds(1));
            continue;
        }

        // Get next action
        Action action = std::move(actions.front());
        actions.pop_front();

        // Run it
        lock.unlock();
        if (action.action)
            action.action();
        else
            action.anyAction(std::move(action.mem));
    }
}
