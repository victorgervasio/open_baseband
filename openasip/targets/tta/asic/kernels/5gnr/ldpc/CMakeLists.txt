set(KERNEL_TARGET "build-tta-asic-kernels-5gnr-ldpc")
set(KERNEL_NAME "asic_5gnr_ldpc")
set(ASIC_ADF_FILE "${CMAKE_CURRENT_LIST_DIR}/../../../../../../arch/5gnr_ue_baseband_proc_asic.adf")
set(ASIC_TPEF_FILE "${CMAKE_CURRENT_BINARY_DIR}/${KERNEL_NAME}.tpef")

# ---------------------------------------------------------------------------
# Common LDPC generated files
# ---------------------------------------------------------------------------
get_property(LDPC_COMMON_GENERATED_DIR GLOBAL PROPERTY LDPC_COMMON_GENERATED_DIR)
get_property(LDPC_COMMON_N0_TABLE GLOBAL PROPERTY LDPC_COMMON_N0_TABLE)
get_property(LDPC_COMMON_NORMAL_SAMPLES GLOBAL PROPERTY LDPC_COMMON_NORMAL_SAMPLES)

if(NOT LDPC_COMMON_GENERATED_DIR)
    message(FATAL_ERROR
        "Common 5G NR LDPC CMake infrastructure was not initialized."
    )
endif()

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------
file(
    GLOB_RECURSE LDPC_TTA_ASIC_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)
file(
    GLOB_RECURSE LDPC_TTA_ASIC_HEADERS
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)
file(
    GLOB_RECURSE LDPC_COMMON_SOURCES
    CONFIGURE_DEPENDS
    "${OPENASIP_ROOT}/targets/common/kernels/5gnr/ldpc/src/*.cpp"
)
file(
    GLOB_RECURSE LDPC_COMMON_HEADERS
    CONFIGURE_DEPENDS
    "${OPENASIP_ROOT}/targets/common/kernels/5gnr/ldpc/include/*.h"
)
file(
    GLOB_RECURSE LDPC_OSED_SOURCES
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/../../../../../../arch/custom/osal/kernels/5gnr/ldpc/*"
)

# Generator programs are built separately by targets/common and must not be
# compiled as part of the TTA kernel.
list(FILTER LDPC_TTA_ASIC_SOURCES EXCLUDE REGEX "/temp/")
list(FILTER LDPC_COMMON_SOURCES EXCLUDE REGEX "/gen-n0/")
list(FILTER LDPC_COMMON_SOURCES EXCLUDE REGEX "/gen-random/")

find_program(OACC_EXECUTABLE oacc)

if(NOT OACC_EXECUTABLE)
    message(FATAL_ERROR
        "oacc was not found. Source ${TCE_ENV} before configuring/building the project."
    )
endif()

# ---------------------------------------------------------------------------
# TTA compiler include paths
# ---------------------------------------------------------------------------
set(TTA_INCLUDE_FLAGS
    "-I${LDPC_COMMON_GENERATED_DIR}"
    "-I${OPENASIP_ROOT}/targets/common/kernels/5gnr/ldpc/include"
    "-I${ETL_ROOT}/include"
    "-I${CMAKE_CURRENT_SOURCE_DIR}/include"
    "-I${OPENASIP_ROOT}/targets/tta/kernels/common/lib/cpp"
    "-I${OPENASIP_ROOT}/targets/common/kernels/lib/cpp"
    "-I${ETL_INSTALL_DIR}/include"

    # Use ETL's numeric implementation in the common norm_dist.h.
    "-DLDPC_USE_ETL"
)

# ---------------------------------------------------------------------------
# Generated .tpef program
# ---------------------------------------------------------------------------
add_custom_command(
    OUTPUT "${ASIC_TPEF_FILE}"
    COMMAND "${OACC_EXECUTABLE}"
            --swfp
            -V
            -DNDEBUG
            -fno-exceptions
            -O3
            ${TTA_INCLUDE_FLAGS}
            ${LDPC_TTA_ASIC_SOURCES}
            ${LDPC_COMMON_SOURCES}
            -a "${ASIC_ADF_FILE}"
            -o "${ASIC_TPEF_FILE}"
    DEPENDS
        ${LDPC_TTA_ASIC_SOURCES}
        ${LDPC_TTA_ASIC_HEADERS}
        ${LDPC_COMMON_SOURCES}
        ${LDPC_COMMON_HEADERS}

        # Generated common headers.
        "${LDPC_COMMON_N0_TABLE}"
        "${LDPC_COMMON_NORMAL_SAMPLES}"

        # TTA architecture.
        "${ASIC_ADF_FILE}"
        "${LDPC_OSED_SOURCES}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT
        "Compiling ${KERNEL_NAME} for the OpenASIP TTA asic architecture"
    VERBATIM
)

add_custom_target(${KERNEL_TARGET}
    DEPENDS "${ASIC_TPEF_FILE}"
)
