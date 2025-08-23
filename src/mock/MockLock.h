#pragma once

#include <gtb/DataModel.h>

#include <mutex>

namespace gtb
{

/**
 * Used to make actions that update multiple data models atomic
 * since the local state is the source of truth, unlike the non-mocked
 * state where Coinbase is the source of truth.
 */
class MockLock : public DataModel
{
    public:
        MockLock();
        MockLock(MockLock &&);
        MockLock(const MockLock &) = delete;
        MockLock &operator=(MockLock &&) = delete;
        MockLock &operator=(const MockLock &) = delete;
        ~MockLock() final = default;

        std::unique_lock<std::mutex> lock();

    private:
        std::mutex mtx;
};

}
