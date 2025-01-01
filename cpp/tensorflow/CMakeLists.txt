cmake_minimum_required(VERSION 3.10)

# Include GNU standard install directories module
include(GNUInstallDirs)

# Get & build tensorflow
include(tensorflow.cmake)

# Test binary
add_subdirectory(rl-test)
