# Aggregate targets must exist before entering the per-target directories.

if(OPENASIP_TARGET STREQUAL "all")
    add_custom_target(build-all)
    add_custom_target(test)
endif()

if(OPENASIP_TARGET STREQUAL "tta" OR OPENASIP_TARGET STREQUAL "all")
    add_custom_target(tta-build)
    add_custom_target(tta)
    add_custom_target(test-tta)

    add_subdirectory(tta/sim/kernels/5gnr/ldpc)
    add_subdirectory(tta/asic/kernels/5gnr/ldpc)

    add_dependencies(tta-build sim-tta-build asic-tta-build)

    add_dependencies(tta sim-tta-build simulate-tta)

    add_dependencies(test-tta tta)
endif()

if(OPENASIP_TARGET STREQUAL "x86_64" OR OPENASIP_TARGET STREQUAL "all")
    add_custom_target(x86_64-build)
    add_custom_target(x86_64)
    add_custom_target(test-x86_64)

    add_subdirectory(x86_64/kernels/5gnr/ldpc)

    add_dependencies(x86_64-build x86_64-ldpc)
    add_dependencies(x86_64 x86_64-ldpc simulate-x86_64)
    add_dependencies(test-x86_64 x86_64)
endif()

if(OPENASIP_TARGET STREQUAL "almaif" OR OPENASIP_TARGET STREQUAL "all")
    add_custom_target(almaif-build)
    add_custom_target(almaif)
    add_custom_target(test-almaif)

    add_subdirectory(almaif/kernels/5gnr/ldpc)

    add_dependencies(almaif-build almaif-ldpc)
    add_dependencies(almaif almaif-ldpc simulate-almaif)
    add_dependencies(test-almaif almaif)
endif()


if(OPENASIP_TARGET STREQUAL "all")
    add_dependencies(
        test
        test-tta
        test-x86_64
        test-almaif
    )

    add_dependencies(
        build-all
        tta-build
        x86_64-build
        almaif-build
    )
endif()
