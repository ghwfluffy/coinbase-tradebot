cmake_minimum_required(VERSION 3.16)
project(gtb)

# Dependencies
set(WEBSOCKETPP_INCLUDE websocketpp)
set(JWTCPP_INCLUDE jwt-cpp/include)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED system thread)
find_package(SQLite3 REQUIRED)
include(cmake/cpr.cmake)

# Build configuration
include(cmake/cxxflags.cmake)

# Find all source files
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
file(
    GLOB_RECURSE
    SOURCE_FILES
    "${SRC_DIR}/*.cpp"
)
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*main.cpp$")
list(FILTER SOURCE_FILES EXCLUDE REGEX ".*cli/*.cpp$")

# Create header symlinks
include(cmake/headers.cmake)

# Library
add_library(gtb STATIC ${SOURCE_FILES})
add_dependencies(gtb gtb-headers)

target_include_directories(
    gtb
    PUBLIC
        ${HEADER_BASEDIR}
)

target_include_directories(
    gtb
    SYSTEM PUBLIC
        ${CPR_INCLUDE}
        ${WEBSOCKETPP_INCLUDE}
        ${JWTCPP_INCLUDE}
        ${Boost_INCLUDE_DIRS}
        ${SQLite3_INCLUDE_DIRS}
)

target_link_libraries(
    gtb
    PUBLIC
        pthread
        cpr::cpr
        Boost::system
        Boost::thread
        OpenSSL::SSL
        OpenSSL::Crypto
        ${SQLite3_LIBRARIES}
)

# Executables
add_executable(tradebot "src/core/main.cpp")
target_link_libraries(tradebot PRIVATE gtb)

add_executable(order-pairs "src/cli/order_pairs.cpp")
target_link_libraries(order-pairs PRIVATE gtb)
