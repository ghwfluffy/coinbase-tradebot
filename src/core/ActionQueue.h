#pragma once

#include <gtb/Any.h>

#include <functional>

namespace gtb
{

/**
 * Queue actions to be handled asynchronously from the calling thread
 */
class ActionQueue
{
    public:
        virtual ~ActionQueue() = default;

        virtual void queue(std::function<void()> action) = 0;
        virtual void queue(std::function<void(Any)> action, Any mem) = 0;
};

}
