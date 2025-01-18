#pragma once

#include <string>
#include <typeinfo>

#include <cxxabi.h>

namespace gtb
{

namespace TypeInfo
{
    template<typename T>
    size_t getId()
    {
        return reinterpret_cast<std::size_t>(&typeid(T));
    }

    template<typename T>
    std::string name(const T &t)
    {
        int status = 0;
        char *realname = abi::__cxa_demangle(typeid(t).name(), nullptr, nullptr, &status);
        std::string result(realname);
        free(realname);
        return result;
    }
}

}
