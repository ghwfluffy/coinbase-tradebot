#pragma once

#include <memory>
#include <typeinfo>
#include <type_traits>

namespace gtb
{

class Any
{
    public:
        Any() = default;
        Any(Any &&) = default;
        Any(const Any &) = delete;
        Any &operator=(Any &&) = default;
        Any &operator=(const Any &) = delete;
        ~Any() = default;

        template<typename T>
        T &get() &
        {
            if (type != hash<T>())
                set(T());
            return *reinterpret_cast<T *>(data->get());
        }

        template<typename T,
            std::enable_if_t<std::is_move_constructible<T>::value, bool> = true>
        T get() &&
        {
            if (type != hash<T>())
                return T();
            T ret = std::move(*reinterpret_cast<T *>(data->get()));
            clear();
            return std::move(ret);
        }

        template<typename T>
        const T &get() const &
        {
            return const_cast<Any *>(this)->get<T>();
        }

        template<typename T>
        void set(T &&t)
        {
            using TType = typename std::remove_reference<T>::type;
            type = hash<TType>();
            data = std::make_unique<ObjHolder<TType>>(std::forward<T>(t));
        }

        template<typename T>
        void set(std::unique_ptr<T> pt)
        {
            type = hash<T>();
            data = std::make_unique<ObjHolder<T>>(std::move(pt));
        }

        template<typename T>
        bool has() const
        {
            return type == hash<T>();
        }

        void clear()
        {
            type = 0;
            data.reset();
        }

    private:
        template<typename T>
        static std::size_t hash()
        {
            return typeid(T).hash_code();
        }

        class IObjHolder
        {
            public:
                virtual ~IObjHolder() = default;

                virtual void *get() = 0;
                virtual void *get() const = 0;

            protected:
                IObjHolder() = default;
                IObjHolder(IObjHolder &&) = default;
                IObjHolder(const IObjHolder &) = default;
                IObjHolder &operator=(IObjHolder &&) = default;
                IObjHolder &operator=(const IObjHolder &) = default;
        };

        template<typename T>
        class ObjHolder : public IObjHolder
        {
            public:
                ObjHolder();
                ObjHolder(const T &t)
                {
                    pt = std::make_unique<T>(t);
                }
                ObjHolder(T &&t)
                {
                    using TType = typename std::remove_reference<T>::type;
                    pt = std::make_unique<T>(std::forward<TType>(t));
                }
                ObjHolder(std::unique_ptr<T> pt)
                {
                    this->pt = std::move(pt);
                }
                ObjHolder(ObjHolder &&) = default;
                ObjHolder(const ObjHolder &t)
                {
                    operator=(t);
                }
                ObjHolder &operator=(ObjHolder &&) = default;
                ObjHolder &operator=(const ObjHolder &t)
                {
                    if (this != &t)
                        pt = std::make_unique<T>(t);
                    return (*this);
                }
                ~ObjHolder() final = default;

                void *get() final
                {
                    return reinterpret_cast<void *>(pt.get());
                }

                void *get() const final
                {
                    return reinterpret_cast<void *>(pt.get());
                }

                std::unique_ptr<T> pt;
        };

        size_t type = 0;
        std::unique_ptr<IObjHolder> data;
};

}
