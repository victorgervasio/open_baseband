find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Aggregate simulation targets. Concrete kernel targets are attached from
# openasip/targets after these targets have been created.
add_custom_target(simulate-tta)
add_custom_target(simulate-x86_64)
add_custom_target(simulate-almaif)
add_custom_target(simulate)
add_custom_target(ttasim)
add_custom_target(proxim)

add_dependencies(simulate simulate-tta simulate-x86_64 simulate-almaif)

# These are intentionally aggregate targets for now. The concrete dataset and
# analysis workflow can be added without changing the public Make interface.
add_custom_target(dataset)
add_custom_target(analyze)

set(OPENBASEBAND_SIM_PY "${CMAKE_CURRENT_SOURCE_DIR}/sim.py" CACHE FILEPATH
    "Simulation runner")
