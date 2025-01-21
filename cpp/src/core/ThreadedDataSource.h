#pragma once

#include <gtb/DataSource.h>

#include <mutex>
#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <condition_variable>

namespace gtb
{

/**
 * Handles thread management for a single-thread data source
 */
class ThreadedDataSource : public DataSource
{
    public:
        ThreadedDataSource(
            std::string name);
        ThreadedDataSource(ThreadedDataSource &&) = delete;
        ThreadedDataSource(const ThreadedDataSource &) = delete;
        ThreadedDataSource &operator=(ThreadedDataSource &&) = delete;
        ThreadedDataSource &operator=(const ThreadedDataSource &) = delete;
        ~ThreadedDataSource() override;

        void start() final;
        void stop() final;

    protected:
        template<typename duration>
        bool sleep(duration d)
        {
            std::unique_lock<std::mutex> lock(mtx);
            if (running)
                cond.wait_for(lock, d);

            return running;
        }

        virtual void process() = 0;
        virtual void shutdown();

    private:
        void runThread();

        std::string name;

        std::mutex mtx;
        std::atomic<bool> running;
        std::condition_variable cond;
        std::unique_ptr<std::thread> thread;
};

}
