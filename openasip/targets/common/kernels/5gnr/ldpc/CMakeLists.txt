set(LDPC_COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(LDPC_COMMON_INCLUDE_DIR "${LDPC_COMMON_DIR}/include")

# Generated files belong to this build tree.
#
# For example:
#
#   build-tta/targets/common/kernels/5gnr/ldpc/include/
#   build-x86_64/targets/common/kernels/5gnr/ldpc/include/
#
set(LDPC_COMMON_GENERATED_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/include"
)

set(LDPC_COMMON_N0_TABLE
    "${LDPC_COMMON_GENERATED_DIR}/n0_table.h"
)

set(LDPC_COMMON_NORMAL_SAMPLES
    "${LDPC_COMMON_GENERATED_DIR}/normal_samples.h"
)

# ---------------------------------------------------------------------------
# N0 table generator
# ---------------------------------------------------------------------------

set(LDPC_N0_GEN_DIR
    "${LDPC_COMMON_DIR}/src/gen-n0"
)

add_executable(ldpc-n0-generator
    "${LDPC_N0_GEN_DIR}/gen_n0_table.cpp"
)

target_include_directories(ldpc-n0-generator
    PRIVATE
        "${LDPC_COMMON_INCLUDE_DIR}"
)

add_custom_command(
    OUTPUT "${LDPC_COMMON_N0_TABLE}"

    COMMAND "${CMAKE_COMMAND}" -E make_directory
            "${LDPC_COMMON_GENERATED_DIR}"

    COMMAND "$<TARGET_FILE:ldpc-n0-generator>"
            "${LDPC_COMMON_N0_TABLE}"

    DEPENDS
        ldpc-n0-generator
        "${LDPC_COMMON_INCLUDE_DIR}/defines.h"

    COMMENT "Generating ${LDPC_COMMON_N0_TABLE}"

    VERBATIM
)

add_custom_target(generate-ldpc-n0
    DEPENDS "${LDPC_COMMON_N0_TABLE}"
)

# ---------------------------------------------------------------------------
# Normal-distribution sample generator
# ---------------------------------------------------------------------------

set(LDPC_RANDOM_GEN_DIR
    "${LDPC_COMMON_DIR}/src/gen-random"
)

add_executable(ldpc-random-generator
    "${LDPC_RANDOM_GEN_DIR}/generate_random_samples.cpp"
)

add_custom_command(
    OUTPUT "${LDPC_COMMON_NORMAL_SAMPLES}"

    COMMAND "${CMAKE_COMMAND}" -E make_directory
            "${LDPC_COMMON_GENERATED_DIR}"

    COMMAND "$<TARGET_FILE:ldpc-random-generator>"
            "${LDPC_COMMON_NORMAL_SAMPLES}"

    DEPENDS
        ldpc-random-generator

    COMMENT "Generating ${LDPC_COMMON_NORMAL_SAMPLES}"

    VERBATIM
)

add_custom_target(generate-ldpc-random
    DEPENDS "${LDPC_COMMON_NORMAL_SAMPLES}"
)

# ---------------------------------------------------------------------------
# Common generated artifacts
# ---------------------------------------------------------------------------

add_custom_target(generate-ldpc-common)
add_dependencies(
    generate-ldpc-common
    generate-ldpc-n0
    generate-ldpc-random
)

# ---------------------------------------------------------------------------
# Common LDPC interface
# ---------------------------------------------------------------------------
#
# Normal CMake targets can consume this target to obtain:
#
#   - source common headers
#   - generated common headers
#   - generation dependency
#
add_library(ldpc-common INTERFACE)

target_include_directories(ldpc-common
    INTERFACE
        "${LDPC_COMMON_GENERATED_DIR}"
        "${LDPC_COMMON_INCLUDE_DIR}"
)

add_dependencies(
    ldpc-common
    generate-ldpc-common
)

# ---------------------------------------------------------------------------
# Export generated paths to the other target-specific CMake directories.
#
# CMake directory variables do not propagate from this subdirectory to its
# siblings, so use global properties for the paths consumed by custom
# commands such as the TTA oacc invocation.
# ---------------------------------------------------------------------------

set_property(
    GLOBAL PROPERTY LDPC_COMMON_GENERATED_DIR
    "${LDPC_COMMON_GENERATED_DIR}"
)

set_property(
    GLOBAL PROPERTY LDPC_COMMON_N0_TABLE
    "${LDPC_COMMON_N0_TABLE}"
)

set_property(
    GLOBAL PROPERTY LDPC_COMMON_NORMAL_SAMPLES
    "${LDPC_COMMON_NORMAL_SAMPLES}"
)
