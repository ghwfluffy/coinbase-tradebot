#pragma once

#include <gtb/TypeInfo.h>
#include <gtb/DataModel.h>
#include <gtb/ActionQueue.h>

#include <map>
#include <list>
#include <memory>

namespace gtb
{

class DataController
{
    public:
        DataController(
            ActionQueue &actions);
        DataController(DataController &&) = delete;
        DataController(const DataController &) = delete;
        DataController &operator=(DataController &&) = delete;
        DataController &operator=(const DataController &) = delete;
        ~DataController() = default;

        template<typename DataType>
        DataType &get()
        {
            size_t type = TypeInfo::getId<DataType>();
            auto iter = dataModels.find(type);
            if (iter == dataModels.end())
            {
                initData(DataType());
                iter = dataModels.find(type);
            }

            return static_cast<DataType &>(*iter->second);
        }

        template<typename DataType>
        void initData(DataType data)
        {
            static_assert(std::is_base_of<DataModel, DataType>::value, "data must derive from DataModel");

            auto model = std::make_unique<DataType>(std::move(data));
            model->setListener(std::bind(&DataController::updated<DataType>, this));
            dataModels[TypeInfo::getId<DataType>()] = std::move(model);
        }

        template<typename DataType, typename DataProcessor>
        void subscribe(DataProcessor &proc)
        {
            static_assert(std::is_base_of<DataModel, DataType>::value, "data must derive from DataModel");

            std::function<void()> cb =
                std::bind(
                    &DataController::notify<DataType, DataProcessor>,
                    std::cref(get<DataType>()),
                    std::ref(proc));

            size_t type = TypeInfo::getId<DataType>();
            subscriptions[type].push_back(std::move(cb));
        }

    private:
        template<typename DataType>
        void updated()
        {
            size_t type = TypeInfo::getId<DataType>();
            for (const auto sub : subscriptions[type])
                actions.queue(sub);
        }

        template<typename DataType, typename DataProcessor>
        static void notify(const DataType &data, DataProcessor &proc)
        {
            proc.process(data);
        }

        ActionQueue &actions;

        std::map<size_t, std::unique_ptr<DataModel>> dataModels;
        std::map<size_t, std::list<std::function<void()>>> subscriptions;
};

}
