cmake_minimum_required(VERSION 3.10)
project(ghw-tradebot-tensorflow VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)

# Include GNU standard install directories module
include(GNUInstallDirs)

# Get & build tensorflow
include(tensorflow.cmake)

# Test binary
add_subdirectory(rl-test)
add_subdirectory(trading-test)
