set(KERNEL_TARGET "almaif-ldpc")

file(GLOB_RECURSE LDPC_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

list(FILTER LDPC_SOURCES EXCLUDE REGEX "/gen-n0/")
list(FILTER LDPC_SOURCES EXCLUDE REGEX "/gen-random/")
list(FILTER LDPC_SOURCES EXCLUDE REGEX "/temp/")

# ALMAIF is compiled by a host compiler, but the implementation is expected
# to contain the appropriate ALMAIF/OpenASIP intrinsics. Keep the compiler
# selection in the normal CMake toolchain mechanism rather than hard-coding a
# compiler path here.
add_executable(${KERNEL_TARGET} ${LDPC_SOURCES})

target_include_directories(${KERNEL_TARGET}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${ETL_ROOT}/include"
        "${ETL_ROOT}/etl/include"
        "${ETL_INSTALL_DIR}/include"
        "${OPENASIP_ROOT}/targets/almaif/lib/cpp"
)

# Optional flags can be supplied with:
#   cmake -DALMAIF_CXX_FLAGS="..."
set(ALMAIF_CXX_FLAGS "" CACHE STRING
    "Additional C++ flags for the ALMAIF target")
if(ALMAIF_CXX_FLAGS)
    separate_arguments(ALMAIF_CXX_FLAGS_LIST NATIVE_COMMAND "${ALMAIF_CXX_FLAGS}")
    target_compile_options(${KERNEL_TARGET} PRIVATE ${ALMAIF_CXX_FLAGS_LIST})
endif()

add_custom_target(simulate-almaif-ldpc
    COMMAND $<TARGET_FILE:${KERNEL_TARGET}>
    DEPENDS ${KERNEL_TARGET}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    USES_TERMINAL
)
add_dependencies(simulate-almaif simulate-almaif-ldpc)

add_custom_target(test-almaif-ldpc
    COMMAND $<TARGET_FILE:${KERNEL_TARGET}>
    DEPENDS ${KERNEL_TARGET}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    USES_TERMINAL
)
add_dependencies(test-almaif test-almaif-ldpc)
