project(rltest VERSION 0.0.1 LANGUAGES CXX)

add_executable(tf_example_trainer example_trainer.cc)
target_link_libraries(tf_example_trainer PRIVATE TensorFlow)
