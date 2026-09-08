set(KERNEL_BUILD_TARGET "build-x86_64-kernels-5gnr-ldpc")
set(KERNEL_SIM_TARGET "simulate-x86_64-kernels-5gnr-ldpc")
set(KERNEL_NAME "5gnr_ldpc")
set(x86_64_PROGRAM "${CMAKE_CURRENT_BINARY_DIR}/${KERNEL_NAME}")

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------
file(
    GLOB_RECURSE LDPC_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
file(
    GLOB_RECURSE LDPC_COMMON_SOURCES
    CONFIGURE_DEPENDS
    "${OPENASIP_ROOT}/targets/common/kernels/5gnr/ldpc/src/*.cpp"
)

# Generator programs are built separately by targets/common and must not be
# compiled as part of the TTA kernel.
list(FILTER LDPC_SOURCES EXCLUDE REGEX "/temp/")
list(FILTER LDPC_COMMON_SOURCES EXCLUDE REGEX "/gen-n0/")
list(FILTER LDPC_COMMON_SOURCES EXCLUDE REGEX "/gen-random/")

add_executable(
    ${KERNEL_BUILD_TARGET}
    ${LDPC_SOURCES}
    ${LDPC_COMMON_SOURCES}
)

set_target_properties(
    ${KERNEL_BUILD_TARGET}
    PROPERTIES
        OUTPUT_NAME "${KERNEL_NAME}"
)

# ---------------------------------------------------------------------------
# Include paths
# ---------------------------------------------------------------------------
target_include_directories(
    ${KERNEL_BUILD_TARGET}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${ETL_ROOT}/include"
        "${ETL_ROOT}/etl/include"
        "${ETL_INSTALL_DIR}/include"
)

# ldpc-common supplies:
#
#   targets/common/.../include
#   build-<target>/targets/common/.../include
#
# and the dependency on generation of n0_table.h and normal_samples.h.
target_link_libraries(
    ${KERNEL_BUILD_TARGET}
    PRIVATE
        ldpc-common
)

# ---------------------------------------------------------------------------
# Compiler options
# ---------------------------------------------------------------------------
target_compile_options(
    ${KERNEL_BUILD_TARGET}
    PRIVATE
        $<$<CONFIG:Debug>:-g3>
        $<$<CONFIG:RelWithDebInfo>:-g3>
        $<$<CONFIG:RelWithDebInfo>:-fno-omit-frame-pointer>
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
)

target_link_options(
    ${KERNEL_BUILD_TARGET}
    PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

# ---------------------------------------------------------------------------
# x86-64 simulation
# ---------------------------------------------------------------------------
add_custom_target(${KERNEL_SIM_TARGET}
    COMMAND "${Python3_EXECUTABLE}"
            "${OPENBASEBAND_SIM_PY}"
            --target x86_64
            --sim_root "${SIM_OUTPUT_DIR}"
            --x86_64_program "$<TARGET_FILE:${KERNEL_BUILD_TARGET}>"
    #DEPENDS ${KERNEL_BUILD_TARGET}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    USES_TERMINAL
)
