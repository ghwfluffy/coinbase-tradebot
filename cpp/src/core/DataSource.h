#pragma once

namespace gtb
{

class DataSource
{
    public:
        virtual ~DataSource() = default;

        virtual void start() = 0;
        virtual void stop() = 0;

    protected:
        DataSource() = default;
        DataSource(DataSource &&) = delete;
        DataSource(const DataSource &) = delete;
        DataSource &operator=(DataSource &&) = delete;
        DataSource &operator=(const DataSource &) = delete;
};

}
