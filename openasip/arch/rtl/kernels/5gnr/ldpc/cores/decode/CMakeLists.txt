set(ADF_FILE "${OPENASIP_ARCH_DIR}/asic_5gnr_ldpc_decode.adf")
set(IDF_FILE "${OPENASIP_ARCH_DIR}/asic_5gnr_ldpc_decode.idf")

set(RTL_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(RTL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(KERNEL_NAME "5gnr_ldpc_decode")
set(RTL_STAMP "${CMAKE_CURRENT_BINARY_DIR}/${RTL_HDL}_${KERNEL_NAME}_generateprocessor.stamp")

find_program(GENERATEPROCESSOR_EXECUTABLE generateprocessor)

if(NOT GENERATEPROCESSOR_EXECUTABLE)
    message(WARNING
        "generateprocessor was not found. "
        "The rtl target will fail until the OpenASIP environment is available."
    )
endif()

set(RTL_HDL "" CACHE STRING
    "OpenASIP processor Hardware Description Language (HDL): Verilog, VHDL")

set_property(CACHE RTL_HDL PROPERTY STRINGS
    verilog vhdl)

set(RTL_VALID_HDL
    vhdl
    verilog
)

if(NOT RTL_HDL IN_LIST RTL_VALID_HDL)
    message(FATAL_ERROR
        "Invalid RTL_HDL='${RTL_HDL}'. "
        "Expected one of: verilog, vhdl."
    )
endif()

add_custom_command(
    OUTPUT "${RTL_STAMP}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${RTL_OUTPUT_DIR}/${RTL_HDL}" # generateprocessor -o requires the output directory to exist
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${RTL_OUTPUT_DIR}/../common/${RTL_HDL}" # generateprocessor -s requires the shared-files-dir to exist
    COMMAND "${GENERATEPROCESSOR_EXECUTABLE}"
            -t
            -F
            --shared-files-dir=${RTL_OUTPUT_DIR}/../common
            --entity-name=tta0_${KERNEL_NAME}
            --hdb-list=asic_130nm_1.5V.hdb,${HDB_FILE_PATH}
            -i "${IDF_FILE}"
            -l "${RTL_HDL}"
            -o "${RTL_OUTPUT_DIR}/${RTL_HDL}"
            "${ADF_FILE}"
    COMMAND "${CMAKE_COMMAND}" -E touch "${RTL_STAMP}"
    DEPENDS "${ADF_FILE}" "${IDF_FILE}" "${HDB_FILE_PATH}"
    COMMENT "Generating ${RTL_HDL} RTL from ${ADF_FILE}"
    VERBATIM
)

if(RTL_HDL STREQUAL "verilog")
    add_custom_target(verilog-rtl-5gnr-ldpc-decode DEPENDS "${RTL_STAMP}")
endif()

if(RTL_HDL STREQUAL "vhdl")
    add_custom_target(vhdl-rtl-5gnr-ldpc-decode DEPENDS "${RTL_STAMP}")
endif()
