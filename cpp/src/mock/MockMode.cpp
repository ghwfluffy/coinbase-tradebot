#include <gtb/MockMode.h>

using namespace gtb;

MockMode::MockMode(
    bool mock)
        : mock(mock)
{
}

MockMode::operator bool() const
{
    return mock;
}
