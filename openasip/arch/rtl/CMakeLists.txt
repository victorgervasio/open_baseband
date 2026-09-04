set(OPENASIP_ARCH_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(ADF_FILE "${OPENASIP_ARCH_DIR}/5gnr_ue_baseband_proc.adf")
set(RTL_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(RTL_STAMP "${CMAKE_CURRENT_BINARY_DIR}/generateprocessor.stamp")

find_program(GENERATEPROCESSOR_EXECUTABLE generateprocessor)

if(NOT GENERATEPROCESSOR_EXECUTABLE)
    message(WARNING "generateprocessor was not found. The rtl target will fail until the OpenASIP environment is available.")
endif()

add_custom_command(
    OUTPUT "${RTL_STAMP}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${RTL_SOURCE_DIR}"
    COMMAND "${GENERATEPROCESSOR_EXECUTABLE}"
            -t
            -o "${RTL_SOURCE_DIR}"
            "${ADF_FILE}"
    COMMAND "${CMAKE_COMMAND}" -E touch "${RTL_STAMP}"
    DEPENDS "${ADF_FILE}"
    COMMENT "Generating RTL from ${ADF_FILE}"
    VERBATIM
)

add_custom_target(rtl DEPENDS "${RTL_STAMP}")
