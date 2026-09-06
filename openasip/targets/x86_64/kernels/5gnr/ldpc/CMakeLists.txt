set(KERNEL_TARGET "x86_64-ldpc")
set(KERNEL_NAME "5gnr_ldpc")

set(x86_64_PROGRAM "${CMAKE_CURRENT_BINARY_DIR}/${KERNEL_TARGET}")

file(GLOB_RECURSE LDPC_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# Do not compile the standalone table/random generators as part of the kernel
# executable. They are development utilities with their own mains.
list(FILTER LDPC_SOURCES EXCLUDE REGEX "/gen-n0/")
list(FILTER LDPC_SOURCES EXCLUDE REGEX "/gen-random/")
list(FILTER LDPC_SOURCES EXCLUDE REGEX "/temp/")

add_executable(${KERNEL_TARGET} ${LDPC_SOURCES})

target_include_directories(${KERNEL_TARGET}
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${ETL_ROOT}/include"
        "${ETL_ROOT}/etl/include"
        "${ETL_INSTALL_DIR}/include"
)

target_compile_options(${KERNEL_TARGET}
    PRIVATE
        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
        $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
)

target_link_options(${KERNEL_TARGET}
    PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

# sim.py creates the timestamped experiment log and captures the simulator
# output. The simulator is selected explicitly so `make ttasim` and
# `make proxim` cannot accidentally run the other simulator.
add_custom_target(simulate-x86_64-ldpc
    COMMAND "${Python3_EXECUTABLE}"
            "${OPENBASEBAND_SIM_PY}"
            --target x86_64
            --sim_root "${SIM_OUTPUT_DIR}"
            --x86_64_program "${x86_64_PROGRAM}"
    DEPENDS ${KERNEL_TARGET}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    USES_TERMINAL
)
add_dependencies(simulate-x86_64 simulate-x86_64-ldpc)

add_custom_target(test-x86_64-ldpc
    COMMAND "${Python3_EXECUTABLE}"
            "${OPENBASEBAND_SIM_PY}"
            --target x86_64
            --sim_root "${SIM_OUTPUT_DIR}"
            --x86_64_program "${x86_64_PROGRAM}"
    DEPENDS ${KERNEL_TARGET}
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    USES_TERMINAL
)
add_dependencies(test-x86_64 test-x86_64-ldpc)
