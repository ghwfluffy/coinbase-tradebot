#pragma once

namespace gtb
{

// XXX: Can get rid of this interface, only used for memory management
class DataProcessor
{
    public:
        virtual ~DataProcessor() = default;

    protected:
        DataProcessor() = default;
        DataProcessor(DataProcessor &&) = delete;
        DataProcessor(const DataProcessor &) = delete;
        DataProcessor &operator=(DataProcessor &&) = delete;
        DataProcessor &operator=(const DataProcessor &) = delete;
};

}
