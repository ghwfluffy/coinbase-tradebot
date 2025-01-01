set(PYTHON_VERSION 3.12)

# Paths to TensorFlow include and libraries
set(TENSORFLOW_VENV_DIR "${CMAKE_SOURCE_DIR}/tensorflow-venv")
set(TENSORFLOW_LIB_DIR "${TENSORFLOW_VENV_DIR}/lib/python${PYTHON_VERSION}/site-packages/tensorflow")
set(TENSORFLOW_INCLUDE_DIR "${TENSORFLOW_VENV_DIR}/lib/python${PYTHON_VERSION}/site-packages/tensorflow/include")

# TensorFlow libraries
set(TENSORFLOW_CC_LIB "${TENSORFLOW_LIB_DIR}/libtensorflow_cc.so")
set(TENSORFLOW_FRAMEWORK_LIB "${TENSORFLOW_LIB_DIR}/libtensorflow_framework.so")

set(TENSORFLOW_CC_LIB_2 "${TENSORFLOW_CC_LIB}.2")
set(TENSORFLOW_FRAMEWORK_LIB_2 "${TENSORFLOW_FRAMEWORK_LIB}.2")

# Custom command to create a virtual environment and install TensorFlow
add_custom_command(
    OUTPUT ${TENSORFLOW_CC_LIB} ${TENSORFLOW_FRAMEWORK_LIB}
    BYPRODUCTS ${TENSORFLOW_CC_LIB_2} ${TENSORFLOW_FRAMEWORK_LIB_2}
    COMMAND bash -c "[ -f \"${TENSORFLOW_CC_LIB}\" -a -f \"${TENSORFLOW_FRAMEWORK_LIB}\" ] || (
        python${PYTHON_VERSION} -m venv ${TENSORFLOW_VENV_DIR} &&
        ${TENSORFLOW_VENV_DIR}/bin/pip install tensorflow &&
        ln -sf \"${TENSORFLOW_CC_LIB_2}\" \"${TENSORFLOW_CC_LIB}\" &&
        ln -sf \"${TENSORFLOW_FRAMEWORK_LIB_2}\" \"${TENSORFLOW_FRAMEWORK_LIB}\" )"
    COMMENT "Setting up Python virtual environment and installing TensorFlow"
    VERBATIM
)

# Custom target to trigger TensorFlow installation
add_custom_target(SetupTensorFlow DEPENDS ${TENSORFLOW_CC_LIB} ${TENSORFLOW_FRAMEWORK_LIB})


# Target for TensorFlow
add_library(TensorFlow INTERFACE)

# Include directories
target_include_directories(TensorFlow SYSTEM INTERFACE "${TENSORFLOW_INCLUDE_DIR}")

# Link libraries
target_link_libraries(TensorFlow INTERFACE
    "${TENSORFLOW_CC_LIB}"
    "${TENSORFLOW_FRAMEWORK_LIB}"
)

add_dependencies(
    TensorFlow
    SetupTensorFlow
)
