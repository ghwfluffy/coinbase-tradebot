#pragma once

#include <gtb/DatabaseConnection.h>

namespace gtb
{

namespace Database
{
    DatabaseConnection newConn();

    void init();
    void setFile(
        std::string file);
    void setSchema(
        std::string file);
}

}
