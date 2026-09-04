# Top-level CMake description for the OpenASIP portion of open_baseband.

set(OPENASIP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

if(NOT DEFINED Python3_EXECUTABLE)
    message(FATAL_ERROR
        "Python3_EXECUTABLE was not provided. "
        "Configure the project through the top-level Makefile so that "
        "the OpenASIP simulation virtual environment is used.")
endif()

if(NOT EXISTS "${Python3_EXECUTABLE}")
    message(FATAL_ERROR
        "Python interpreter does not exist: ${Python3_EXECUTABLE}")
endif()

message(STATUS "OpenASIP Python interpreter: ${Python3_EXECUTABLE}")

if(NOT EXISTS "${ETL_ROOT}")
    message(FATAL_ERROR
        "ETL_ROOT does not exist: ${ETL_ROOT}\n"
        "Run 'make' so the ETL dependency can be cloned/built, or set ETL_ROOT manually.")
endif()

# Select which OpenASIP target is being configured.
set(OPENASIP_TARGET "" CACHE STRING
    "OpenASIP target to configure: tta, x86_64, almaif, all")

set_property(CACHE OPENASIP_TARGET PROPERTY STRINGS
    tta x86_64 almaif all)

set(OPENASIP_VALID_TARGETS
    tta
    x86_64
    almaif
    all
)

if(NOT OPENASIP_TARGET IN_LIST OPENASIP_VALID_TARGETS)
    message(FATAL_ERROR
        "Invalid OPENASIP_TARGET='${OPENASIP_TARGET}'. "
        "Expected one of: tta, x86_64, almaif, all."
    )
endif()

# Everything that uses .adf files.
set(OPENASIP_ADF_TARGETS
    tta
    almaif
)

if(OPENASIP_TARGET IN_LIST OPENASIP_ADF_TARGETS
   OR OPENASIP_TARGET STREQUAL "all")
    add_subdirectory(arch/rtl)
endif()

# Simulation infrastructure.
if(OPENASIP_TARGET STREQUAL "tta"
   OR OPENASIP_TARGET STREQUAL "x86_64"
   OR OPENASIP_TARGET STREQUAL "almaif"
   OR OPENASIP_TARGET STREQUAL "all")
    add_subdirectory(sim)
endif()

# Per-target kernels.
add_subdirectory(targets)
