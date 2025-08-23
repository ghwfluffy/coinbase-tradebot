#pragma once

#include <gtb/DataModel.h>

namespace gtb
{

/**
 * Check if we are running a mock simulation
 */
class MockMode : public DataModel
{
    public:
        MockMode(bool mock = false);
        MockMode(MockMode &&) = default;
        MockMode(const MockMode &) = delete;
        MockMode &operator=(MockMode &&) = delete;
        MockMode &operator=(const MockMode &) = delete;
        ~MockMode() final = default;

        operator bool() const;

    private:
        bool mock;
};

}
