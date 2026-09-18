set(OPENASIP_ARCH_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
#set(ADF_FILE "${OPENASIP_ARCH_DIR}/5gnr_ue_baseband_proc_asic.adf")
#set(IDF_FILE "${OPENASIP_ARCH_DIR}/5gnr_ue_baseband_proc_asic.idf")

#set(RTL_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")
#set(RTL_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(RTL_STAMP "${CMAKE_CURRENT_BINARY_DIR}/${RTL_HDL}_generateprocessor.stamp")

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

# ==================
# hdb databases
# ==================
set(OPENASIP_ARCH_DIR "${OPENASIP_ROOT}/arch")
set(HDB_DIR "${OPENASIP_ARCH_DIR}/custom/hdb")
set(HDB_FILE_PATH "${HDB_DIR}/5gnr_ue_baseband_proc_custom.hdb")

if(NOT EXISTS "${HDB_FILE_PATH}")
    message(FATAL_ERROR
        "Required OpenASIP HDB file was not found:\n"
        "  ${HDB_FILE_PATH}\n\n"
        "If this repository uses Git submodules, initialize them with:\n"
        "  git submodule update --init --recursive\n"
    )
endif()

file(READ "${HDB_FILE_PATH}" HDB_HEADER LIMIT 100)

if(HDB_HEADER MATCHES "version https://git-lfs")
    message(FATAL_ERROR
        "The HDB file is a Git LFS pointer rather than the actual binary:\n"
        "  ${HDB_FILE_PATH}\n\n"
        "Run:\n"
        "  git lfs pull\n"
    )
endif()

message(STATUS "Using custom HDB:")
message(STATUS "  ${HDB_FILE_PATH}")

#add_custom_command(
#    OUTPUT "${RTL_STAMP}"
#    COMMAND "${CMAKE_COMMAND}" -E make_directory "${RTL_OUTPUT_DIR}/${RTL_HDL}" # generateprocessor -o requires the output directory to exist
#    COMMAND "${GENERATEPROCESSOR_EXECUTABLE}"
#            -t
#            -F
#            --hdb-list=asic_130nm_1.5V.hdb,${RTL_SOURCE_DIR}/../custom/hdb/5gnr_ue_baseband_proc_custom.hdb
#            -i "${IDF_FILE}"
#            -l "${RTL_HDL}"
#            -o "${RTL_OUTPUT_DIR}/${RTL_HDL}"
#            "${ADF_FILE}"
#    COMMAND "${CMAKE_COMMAND}" -E touch "${RTL_STAMP}"
#    DEPENDS "${ADF_FILE}" "${IDF_FILE}"
#    COMMENT "Generating ${RTL_HDL} RTL from ${ADF_FILE}"
#    VERBATIM
#)
#
if(RTL_HDL STREQUAL "verilog")
    #add_custom_target(verilog-rtl DEPENDS "${RTL_STAMP}")
    add_custom_target(verilog-rtl)
endif()

if(RTL_HDL STREQUAL "vhdl")
    add_custom_target(vhdl-rtl)
endif()

add_subdirectory(kernels/5gnr/ldpc)

if(RTL_HDL STREQUAL "verilog")
    #add_custom_target(verilog-rtl DEPENDS "${RTL_STAMP}")
    add_dependencies(verilog-rtl verilog-rtl-5gnr-ldpc)
endif()

if(RTL_HDL STREQUAL "vhdl")
    add_dependencies(vhdl-rtl vhdl-rtl-5gnr-ldpc)
endif()
