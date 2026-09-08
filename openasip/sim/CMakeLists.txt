find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(SIM_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH
    "Root directory for simulation outputs")
set(DATASET_LOG_DIR "" CACHE PATH
    "Simulation log directory to use for dataset generation. Empty = latest log.")
set(OPENBASEBAND_SIM_PY "${CMAKE_CURRENT_SOURCE_DIR}/sim.py" CACHE FILEPATH
    "Simulation runner")

# These are intentionally aggregate targets for now. The concrete post-proc
# workflow can be added without changing the public Make interface.
add_custom_target(post-proc)

add_subdirectory(post-proc/parsing)
add_subdirectory(post-proc/analysis)
