#include <gtb/ThreadedDataSource.h>
#include <gtb/Log.h>

#include <functional>

#include <pthread.h>

using namespace gtb;

ThreadedDataSource::ThreadedDataSource(
    std::string name)
        : name(std::move(name))
        , running(false)
{
}

ThreadedDataSource::~ThreadedDataSource()
{
    stop();
}

void ThreadedDataSource::start()
{
    std::lock_guard<std::mutex> lock(mtx);
    if (running)
        return;

    log::info("Starting data source thread '%s'.", name.c_str());

    running = true;
    thread = std::make_unique<std::thread>(std::bind(&ThreadedDataSource::runThread, this));
}

void ThreadedDataSource::stop()
{
    if (!running)
        return;

    log::info("Stopping data source thread '%s'.", name.c_str());
    {
        std::lock_guard<std::mutex> lock(mtx);
        running = false;
        cond.notify_one();
    }

    shutdown();

    if (thread)
    {
        thread->join();
        thread.reset();
    }
}

void ThreadedDataSource::shutdown()
{
}

void ThreadedDataSource::runThread()
{
    pthread_setname_np(pthread_self(), name.c_str());

    while (running)
        process();
}
