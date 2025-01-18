#pragma once

#include <functional>

namespace gtb
{

class DataModel
{
    public:
        virtual ~DataModel() = default;

        void setListener(std::function<void()> cb);

    protected:
        DataModel() = default;
        DataModel(DataModel &&) = default;
        DataModel(const DataModel &) = delete;
        DataModel &operator=(DataModel &&) = delete;
        DataModel &operator=(const DataModel &) = delete;

        void updated();

    private:
        std::function<void()> cb;
};

}
