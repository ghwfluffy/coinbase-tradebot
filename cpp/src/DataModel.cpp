#include <gtb/DataModel.h>

using namespace gtb;

void DataModel::setListener(std::function<void()> cb)
{
    this->cb = std::move(cb);
}

void DataModel::updated()
{
    if (cb)
        cb();
}
