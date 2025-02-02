#include <gtb/MockLock.h>

using namespace gtb;

MockLock::MockLock()
{
}

MockLock::MockLock(MockLock &&rhs)
{
    (void)rhs;
}

std::unique_lock<std::mutex> MockLock::lock()
{
    return std::unique_lock<std::mutex>(mtx);
}
