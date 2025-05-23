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
