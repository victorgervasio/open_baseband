cmake_minimum_required(VERSION 3.20)
project(open_baseband LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(ETL_ROOT "${CMAKE_SOURCE_DIR}/openasip/targets/common/kernels/lib/cpp/etl" CACHE PATH
    "Root of the OpenASIP-compatible ETL installation/source tree")

set(ETL_INSTALL_DIR "${ETL_ROOT}/install" CACHE PATH
    "Local installation prefix for the OpenASIP-compatible ETL")

set(TCE_ENV "${CMAKE_SOURCE_DIR}/openasip/config/tce-env.sh" CACHE FILEPATH
    "Path to OpenASIP tce-env.sh; the top-level Makefile sources it")

set(SIMULATOR "ttasim" CACHE STRING "TTA simulator to use for generic simulation")
set_property(CACHE SIMULATOR PROPERTY STRINGS ttasim proxim)

if(NOT SIMULATOR STREQUAL "ttasim" AND NOT SIMULATOR STREQUAL "proxim")
    message(FATAL_ERROR "SIMULATOR must be either ttasim or proxim (got '${SIMULATOR}')")
endif()

add_subdirectory(openasip)
