find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(SIM_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE PATH
    "Root directory for simulation outputs")
set(DATASET_LOG_DIR "" CACHE PATH
    "Simulation log directory to use for dataset generation. Empty = latest log.")
set(OPENBASEBAND_SIM_PY "${CMAKE_CURRENT_SOURCE_DIR}/sim.py" CACHE FILEPATH
    "Simulation runner")

# Aggregate simulation targets. Concrete kernel targets are attached from
# openasip/targets after these targets have been created.
add_custom_target(simulate-tta)
add_custom_target(simulate-x86_64)
add_custom_target(simulate-almaif)
add_custom_target(simulate)
add_custom_target(ttasim)
add_custom_target(proxim)

add_dependencies(simulate simulate-tta simulate-x86_64 simulate-almaif)

# These are intentionally aggregate targets for now. The concrete post-proc
# workflow can be added without changing the public Make interface.
add_custom_target(post-proc)

add_subdirectory(post-proc/parsing)
add_subdirectory(post-proc/analysis)
