project(trading-test VERSION 0.0.1 LANGUAGES CXX)

add_executable(trading-test main.cpp)
target_link_libraries(trading-test PRIVATE TensorFlow)
