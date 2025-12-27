if (NOT DEFINED WEBSOCKETPP_INCLUDE)
    set(WEBSOCKETPP_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/../libs/websocketpp")
endif()

# Apply websocketpp patch if needed. The patch is small, so we detect its
# application by checking for the shortened constructor/destructor names.
set(_wspp_header "${WEBSOCKETPP_INCLUDE}/websocketpp/endpoint.hpp")
set(_wspp_needs_patch FALSE)
if (EXISTS "${_wspp_header}")
    file(READ "${_wspp_header}" _wspp_endpoint_content)
    if (_wspp_endpoint_content MATCHES "~endpoint<connection,config>\\(\\)")
        set(_wspp_needs_patch TRUE)
    endif()
endif()

if (_wspp_needs_patch)
    message(STATUS "Applying websocketpp patch from cmake/websockpp.diff")
    execute_process(
        COMMAND patch -p1 -i "${CMAKE_CURRENT_LIST_DIR}/websockpp.diff"
        WORKING_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/../libs/websocketpp"
        RESULT_VARIABLE _wspp_patch_result
        OUTPUT_VARIABLE _wspp_patch_out
        ERROR_VARIABLE  _wspp_patch_err
    )
    if (NOT _wspp_patch_result EQUAL 0)
        message(FATAL_ERROR "Failed to apply websocketpp patch: ${_wspp_patch_err}\n${_wspp_patch_out}")
    endif()
endif()
