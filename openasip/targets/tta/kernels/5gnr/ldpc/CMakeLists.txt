set(KERNEL_TARGET "tta-ldpc")
set(KERNEL_NAME "5gnr_ldpc")

set(ADF_FILE
    "${CMAKE_CURRENT_LIST_DIR}/../../../../../arch/5gnr_ue_baseband_proc.adf")

set(TPEF_FILE
    "${CMAKE_CURRENT_BINARY_DIR}/${KERNEL_NAME}.tpef")

file(GLOB_RECURSE LDPC_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

list(FILTER LDPC_SOURCES EXCLUDE REGEX "/gen-n0/")
list(FILTER LDPC_SOURCES EXCLUDE REGEX "/gen-random/")
list(FILTER LDPC_SOURCES EXCLUDE REGEX "/temp/")

find_program(OACC_EXECUTABLE oacc)
if(NOT OACC_EXECUTABLE)
    message(FATAL_ERROR
        "oacc was not found. Source ${TCE_ENV} before configuring/building the project.")
endif()

# The ETL source tree is kept in the repository's common library area. Both
# include layouts are retained because the OpenASIP-compatible ETL fork used
# by this project exposes headers through both paths.
set(TTA_INCLUDE_FLAGS
    "-I${ETL_ROOT}/include"
    #    "-I${ETL_ROOT}/etl/include"
    "-I${CMAKE_CURRENT_SOURCE_DIR}/include"
    "-I${OPENASIP_ROOT}/targets/tta/kernels/common/lib/cpp"
    "-I${OPENASIP_ROOT}/targets/common/kernels/lib/cpp"
    "-I${ETL_INSTALL_DIR}/include"
)

add_custom_command(
    OUTPUT "${TPEF_FILE}"
    COMMAND "${OACC_EXECUTABLE}"
            --swfp
            -V
            -fno-exceptions
            -O3
            ${TTA_INCLUDE_FLAGS}
            ${LDPC_SOURCES}
            -a "${ADF_FILE}"
            -o "${TPEF_FILE}"
    DEPENDS
        ${LDPC_SOURCES}
        "${ADF_FILE}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Compiling ${KERNEL_NAME} for the OpenASIP TTA"
    VERBATIM
)

add_custom_target(${KERNEL_TARGET} DEPENDS "${TPEF_FILE}")

# sim.py creates the timestamped experiment log and captures the simulator
# output. The simulator is selected explicitly so `make ttasim` and
# `make proxim` cannot accidentally run the other simulator.
add_custom_target(simulate-tta-ldpc
    COMMAND "${Python3_EXECUTABLE}"
            "${OPENBASEBAND_SIM_PY}"
            --simulator "${SIMULATOR}"
            --adf-file "${ADF_FILE}"
            --tpef-file "${TPEF_FILE}"
    DEPENDS ${KERNEL_TARGET}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    USES_TERMINAL
)
add_dependencies(simulate-tta simulate-tta-ldpc)

add_custom_target(simulate-tta-ttasim-ldpc
    COMMAND "${Python3_EXECUTABLE}"
            "${OPENBASEBAND_SIM_PY}"
            --simulator ttasim
            --adf-file "${ADF_FILE}"
            --tpef-file "${TPEF_FILE}"
    DEPENDS ${KERNEL_TARGET}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    USES_TERMINAL
)
add_dependencies(ttasim simulate-tta-ttasim-ldpc)

add_custom_target(simulate-tta-proxim-ldpc
    COMMAND "${Python3_EXECUTABLE}"
            "${OPENBASEBAND_SIM_PY}"
            --simulator proxim
            --adf-file "${ADF_FILE}"
            --tpef-file "${TPEF_FILE}"
    DEPENDS ${KERNEL_TARGET}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    USES_TERMINAL
)
add_dependencies(proxim simulate-tta-proxim-ldpc)

add_custom_target(test-tta-ldpc
    COMMAND "${Python3_EXECUTABLE}"
            "${OPENBASEBAND_SIM_PY}"
            --simulator "${SIMULATOR}"
            --adf-file "${ADF_FILE}"
            --tpef-file "${TPEF_FILE}"
    DEPENDS ${KERNEL_TARGET}
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    USES_TERMINAL
)
add_dependencies(test-tta test-tta-ldpc)
