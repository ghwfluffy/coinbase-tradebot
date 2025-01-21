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

# Create header symlinks
include(cmake/headers.cmake)

# Executable
add_executable(tradebot ${SOURCE_FILES})
add_dependencies(tradebot gtb-headers)

target_include_directories(
    tradebot
    PRIVATE
        ${HEADER_BASEDIR}
)

target_include_directories(
    tradebot
    SYSTEM PRIVATE
        ${CPR_INCLUDE}
        ${WEBSOCKETPP_INCLUDE}
        ${JWTCPP_INCLUDE}
        ${Boost_INCLUDE_DIRS}
        ${SQLite3_INCLUDE_DIRS}
)

target_link_libraries(
    tradebot
    PRIVATE
        pthread
        cpr::cpr
        Boost::system
        Boost::thread
        OpenSSL::SSL
        OpenSSL::Crypto
        ${SQLite3_LIBRARIES}
)
