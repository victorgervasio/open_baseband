# Aggregate targets must exist before entering the per-target directories.

# ---------------------------------------------------------------------------
# Common OPENASIP infrastructure
#
# This must be configured before any target-specific kernel directory that
# consumes it.
# ---------------------------------------------------------------------------
if(
    OPENASIP_TARGET STREQUAL "tta" OR
    OPENASIP_TARGET STREQUAL "x86_64" OR
    OPENASIP_TARGET STREQUAL "almaif" OR
    OPENASIP_TARGET STREQUAL "all"
)
    # ---------------------------------------------------------------
    # Process leaf implementation
    #
    # This CMakeLists create:
    #
    #   generate-ldpc-common 
    # ---------------------------------------------------------------
    add_subdirectory(common/kernels/5gnr/ldpc)
endif()

if(OPENASIP_TARGET STREQUAL "tta" OR OPENASIP_TARGET STREQUAL "all")
    # ---------------------------------------------------------------
    # Declare aggregate targets
    # ---------------------------------------------------------------
    # build-tta dependency chain
    #   build-tta-asic dependency chain
    add_custom_target(build-tta-asic-kernels-5gnr)
    add_custom_target(build-tta-asic-kernels)
    add_custom_target(build-tta-asic)
    #   build-tta-sim dependency chain
    add_custom_target(build-tta-sim-kernels-5gnr)
    add_custom_target(build-tta-sim-kernels)
    add_custom_target(build-tta-sim)

    add_custom_target(build-tta)

    # simulate-tta dependency chain
    add_custom_target(simulate-tta-kernels-5gnr)
    add_custom_target(simulate-tta-kernels)
    add_custom_target(simulate-tta)

    add_custom_target(tta)

    # ---------------------------------------------------------------
    # Process leaf implementations
    #
    # These CMakeLists create:
    #
    #   build-tta-sim-kernels-5gnr-ldpc
    #   build-tta-asic-kernels-5gnr-ldpc
    #   simulate-tta-kernels-5gnr-ldpc
    # ---------------------------------------------------------------
    add_subdirectory(tta/sim/kernels/5gnr/ldpc)
    add_subdirectory(tta/asic/kernels/5gnr/ldpc)

    # ---------------------------------------------------------------
    # Complete dependency graph
    # ---------------------------------------------------------------
    # build-tta dependency chain
    #   build-tta-asic dependency chain
    add_dependencies(build-tta-asic-kernels-5gnr-ldpc generate-ldpc-common)
    add_dependencies(build-tta-asic-kernels-5gnr build-tta-asic-kernels-5gnr-ldpc)
    add_dependencies(build-tta-asic-kernels build-tta-asic-kernels-5gnr)
    add_dependencies(build-tta-asic build-tta-asic-kernels)
    #   build-tta-sim dependency chain
    add_dependencies(build-tta-sim-kernels-5gnr-ldpc generate-ldpc-common)
    add_dependencies(build-tta-sim-kernels-5gnr build-tta-sim-kernels-5gnr-ldpc)
    add_dependencies(build-tta-sim-kernels build-tta-sim-kernels-5gnr)
    add_dependencies(build-tta-sim build-tta-sim-kernels)

    add_dependencies(build-tta build-tta-sim build-tta-asic)

    # simulate-tta dependency chain
    add_dependencies(simulate-tta-kernels-5gnr simulate-tta-kernels-5gnr-ldpc)
    add_dependencies(simulate-tta-kernels simulate-tta-kernels-5gnr)
    add_dependencies(simulate-tta simulate-tta-kernels)

    add_dependencies(tta build-tta simulate-tta dataset-tta)
endif()

if(OPENASIP_TARGET STREQUAL "x86_64" OR OPENASIP_TARGET STREQUAL "all")
    # ---------------------------------------------------------------
    # Declare aggregate targets
    # ---------------------------------------------------------------
    # build-x86_64 dependency chain
    add_custom_target(build-x86_64-kernels-5gnr)
    add_custom_target(build-x86_64-kernels)
    add_custom_target(build-x86_64)

    # simulate-x86_64 dependency chain
    add_custom_target(simulate-x86_64-kernels-5gnr)
    add_custom_target(simulate-x86_64-kernels)
    add_custom_target(simulate-x86_64)

    add_custom_target(x86_64)

    # ---------------------------------------------------------------
    # Process leaf implementation
    #
    # This CMakeLists create:
    #
    #   build-x86_64-kernels-5gnr-ldpc
    #   simulate-x86_64-kernels-5gnr-ldpc
    # ---------------------------------------------------------------
    add_subdirectory(x86_64/kernels/5gnr/ldpc)
    
    # ---------------------------------------------------------------
    # Complete dependency graph
    # ---------------------------------------------------------------
    # build-x86_64 dependency chain
    add_dependencies(build-x86_64-kernels-5gnr-ldpc generate-ldpc-common)
    add_dependencies(build-x86_64-kernels-5gnr build-x86_64-kernels-5gnr-ldpc)
    add_dependencies(build-x86_64-kernels build-x86_64-kernels-5gnr)
    add_dependencies(build-x86_64 build-x86_64-kernels)

    # simulate-x86_64 dependency chain
    add_dependencies(simulate-x86_64-kernels-5gnr simulate-x86_64-kernels-5gnr-ldpc)
    add_dependencies(simulate-x86_64-kernels simulate-x86_64-kernels-5gnr)
    add_dependencies(simulate-x86_64 simulate-x86_64-kernels)

    add_dependencies(x86_64 build-x86_64 simulate-x86_64 dataset-x86_64)
endif()

if(OPENASIP_TARGET STREQUAL "almaif" OR OPENASIP_TARGET STREQUAL "all")
    # ---------------------------------------------------------------
    # Declare aggregate targets
    # ---------------------------------------------------------------
    # build-almaif dependency chain
    add_custom_target(build-almaif-kernels-5gnr)
    add_custom_target(build-almaif-kernels)
    add_custom_target(build-almaif)

    # simulate-almaif dependency chain
    add_custom_target(simulate-almaif-kernels-5gnr)
    add_custom_target(simulate-almaif-kernels)
    add_custom_target(simulate-almaif)

    add_custom_target(almaif)

    # ---------------------------------------------------------------
    # Process leaf implementation
    #
    # This CMakeLists create:
    #
    #   build-almaif-kernels-5gnr-ldpc
    #   simulate-almaif-kernels-5gnr-ldpc
    # ---------------------------------------------------------------
    add_subdirectory(almaif/kernels/5gnr/ldpc)
    
    # ---------------------------------------------------------------
    # Complete dependency graph
    # ---------------------------------------------------------------
    # build-almaif dependency chain
    add_dependencies(build-almaif-kernels-5gnr-ldpc generate-ldpc-common)
    add_dependencies(build-almaif-kernels-5gnr build-almaif-kernels-5gnr-ldpc)
    add_dependencies(build-almaif-kernels build-almaif-kernels-5gnr)
    add_dependencies(build-almaif build-almaif-kernels)

    # simulate-almaif dependency chain
    add_dependencies(simulate-almaif-kernels-5gnr simulate-almaif-kernels-5gnr-ldpc)
    add_dependencies(simulate-almaif-kernels simulate-almaif-kernels-5gnr)
    add_dependencies(simulate-almaif simulate-almaif-kernels)

    add_dependencies(almaif build-almaif simulate-almaif dataset-almaif)
endif()

if(OPENASIP_TARGET STREQUAL "all")
    # ---------------------------------------------------------------
    # Declare aggregate targets
    # ---------------------------------------------------------------
    add_custom_target(build)
    add_custom_target(simulate)
    add_custom_target(dataset)
    add_custom_target(all)

    # ---------------------------------------------------------------
    # Complete dependency graph
    # ---------------------------------------------------------------
    add_dependencies(
        build
        build-tta
        build-x86_64
        build-almaif
    )

    add_dependencies(
        simulate 
        simulate-tta
        simulate-x86_64
        simulate-almaif
    )

    add_dependencies(
        dataset 
        dataset-tta
        dataset-x86_64
        dataset-almaif
    )

    add_dependencies(
        all
        build
        simulate
        dataset
    )
endif()
