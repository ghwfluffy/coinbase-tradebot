project(simple-test VERSION 0.0.1 LANGUAGES CXX)

add_executable(simple-test main.cpp)
target_link_libraries(simple-test PRIVATE TensorFlow)
