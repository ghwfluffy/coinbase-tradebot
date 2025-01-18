cmake_minimum_required(VERSION 3.16)
project(gtb)

set(CMAKE_CXX_STANDARD 17)

set(WEBSOCKETPP_INCLUDE websocketpp)
set(JWTCPP_INCLUDE jwt-cpp/include)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED system thread)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Find all source files
file(
    GLOB_RECURSE
    SOURCE_FILES
    "${SRC_DIR}/*.cpp"
)

# Find all header files
file(
    GLOB_RECURSE
    HEADER_FILES
    "${SRC_DIR}/*.h"
)

# Stage header files with fluffycoin/lib/ prefix
set(HEADER_BASEDIR ${CMAKE_CURRENT_BINARY_DIR}/dist/include/)
set(HEADER_DIR ${HEADER_BASEDIR}/gtb)
set(HEADER_TIMESTAMP ${CMAKE_CURRENT_BINARY_DIR}/.headers.timestamp)
add_custom_command(
    OUTPUT
        ${HEADER_TIMESTAMP}
    COMMAND
        rm -rf ${HEADER_DIR}
    COMMAND
        mkdir -p ${HEADER_DIR}
    COMMAND
        ln -s ${HEADER_FILES} ${HEADER_DIR}/
    COMMAND
        touch ${HEADER_TIMESTAMP}
    DEPENDS
        ${HEADER_FILES}
    COMMENT
        "Setting up headers"
)

set_source_files_properties(
    ${HEADER_TIMESTAMP}
    PROPERTIES GENERATED TRUE)
add_custom_target(
    gtb-headers
    DEPENDS
        ${HEADER_TIMESTAMP}
)

add_executable(tradebot ${SOURCE_FILES})
add_dependencies(tradebot gtb-headers)

target_include_directories(
    tradebot
    PRIVATE
        ${HEADER_BASEDIR}
        ${WEBSOCKETPP_INCLUDE}
        ${JWTCPP_INCLUDE}
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries(
    tradebot
    PRIVATE
        pthread
        Boost::system
        Boost::thread
        OpenSSL::SSL
        OpenSSL::Crypto
)
