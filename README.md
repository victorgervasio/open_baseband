Current project source file tree (omitting libraries such as ETL):

```bash
openasip
├── arch
│   ├── 5gnr_ue_baseband_proc_asic.adf
│   ├── 5gnr_ue_baseband_proc_asic.idf
│   ├── 5gnr_ue_baseband_proc_sim.adf
│   ├── custom
│   │   └── hdb
│   │       ├── 5gnr_ue_baseband_proc_custom.hdb
│   │       └── rtl
│   │           └── FUs
│   │               └── rtc
│   │                   └── rtc_rtimer_always_1.vhd
│   └── rtl
│       └── CMakeLists.txt
├── CMakeLists.txt
├── config
│   └── tce-env.sh
├── sim
│   ├── CMakeLists.txt
│   ├── post-proc
│   │   ├── analysis
│   │   │   └── CMakeLists.txt
│   │   └── parsing
│   │       ├── CMakeLists.txt
│   │       ├── find_latest_log.cmake
│   │       └── gen_dataset.py
│   ├── requirements.txt
│   └── sim.py
└── targets
    ├── almaif
    │   └── kernels
    │       ├── 5gnr
    │       │   └── ldpc
    │       │       ├── CMakeLists.txt
    │       │       ├── custom
    │       │       ├── include
    │       │       └── src
    │       └── lib
    │           └── cpp
    ├── CMakeLists.txt
    ├── common
    │   └── kernels
    │       └── lib
    │           └── cpp
    ├── tta
    │   ├── asic
    │   │   └── kernels
    │   │       ├── 5gnr
    │   │       │   └── ldpc
    │   │       │       ├── CMakeLists.txt
    │   │       │       ├── custom
    │   │       │       ├── include
    │   │       │       │   ├── defines.h
    │   │       │       │   └── LDPC.h
    │   │       │       └── src
    │   │       │           ├── gen-n0
    │   │       │           │   ├── gen_n0_table.cpp
    │   │       │           │   ├── gen_n0_table.sh
    │   │       │           │   └── n0_table.h
    │   │       │           ├── gen-random
    │   │       │           │   ├── generate_random_samples.cpp
    │   │       │           │   ├── generate_random_samples.sh
    │   │       │           │   ├── normal_samples.h
    │   │       │           │   └── norm_dist.h
    │   │       │           ├── LDPC.cpp
    │   │       │           ├── main.cpp
    │   │       │           └── nrLDPCTables.cpp
    │   │       └── common
    │   │           └── lib
    │   │               └── cpp
    │   └── sim
    │       └── kernels
    │           ├── 5gnr
    │           │   └── ldpc
    │           │       ├── CMakeLists.txt
    │           │       ├── custom
    │           │       ├── include
    │           │       │   ├── defines.h
    │           │       │   └── LDPC.h
    │           │       └── src
    │           │           ├── gen-n0
    │           │           │   ├── gen_n0_table.cpp
    │           │           │   ├── gen_n0_table.sh
    │           │           │   └── n0_table.h
    │           │           ├── gen-random
    │           │           │   ├── generate_random_samples.cpp
    │           │           │   ├── generate_random_samples.sh
    │           │           │   ├── normal_samples.h
    │           │           │   └── norm_dist.h
    │           │           ├── LDPC.cpp
    │           │           ├── main.cpp
    │           │           └── nrLDPCTables.cpp
    │           └── common
    │               └── lib
    │                   └── cpp
    └── x86_64
        └── kernels
            ├── 5gnr
            │   └── ldpc
            │       ├── CMakeLists.txt
            │       ├── include
            │       │   ├── defines.h
            │       │   └── LDPC.h
            │       └── src
            │           ├── gen-n0
            │           │   ├── gen_n0_table.cpp
            │           │   ├── gen_n0_table.sh
            │           │   └── n0_table.h
            │           ├── gen-random
            │           │   ├── generate_random_samples.cpp
            │           │   ├── generate_random_samples.sh
            │           │   ├── normal_samples.h
            │           │   └── norm_dist.h
            │           ├── LDPC.cpp
            │           ├── main.cpp
            │           └── nrLDPCTables.cpp
            └── lib
                └── cpp

62 directories, 56 files
```

Current project build file tree (omitting libraries and binaries, mostly from venv):
Build commands for reproducing this build state:

```bash
make build-tta && make simulate-tta
make build-x86_64 && make simulate-x86_64
make dataset-tta && make dataset-x86_64
make vhdl-rtl && make verilog-rtl
```

```bash
build
└── openasip
    ├── build-tta
    │   └── openasip
    │       ├── arch
    │       │   └── rtl
    │       │       ├── verilog
    │       │       │   ├── gcu_ic
    │       │       │   │   ├── decoder.v
    │       │       │   │   ├── gcu_opcodes_pkg.vh
    │       │       │   │   ├── ic.v
    │       │       │   │   ├── idecompressor.v
    │       │       │   │   ├── ifetch.v
    │       │       │   │   ├── ifetch.vhdl
    │       │       │   │   ├── input_mux_4.v
    │       │       │   │   ├── output_socket_1_1.v
    │       │       │   │   └── output_socket_4_1.v
    │       │       │   ├── iverilog_compile.sh
    │       │       │   ├── iverilog_simulate.sh
    │       │       │   ├── modsim_compile.sh
    │       │       │   ├── modsim_simulate.sh
    │       │       │   └── verilog
    │       │       │       ├── tce_util_pkg.vh
    │       │       │       ├── tta0_globals_pkg.vh
    │       │       │       ├── tta0_params_pkg.vh
    │       │       │       └── tta0.v
    │       │       ├── verilog_generateprocessor.stamp
    │       │       ├── vhdl
    │       │       │   ├── gcu_ic
    │       │       │   │   ├── decoder.vhdl
    │       │       │   │   ├── gcu_opcodes_pkg.vhdl
    │       │       │   │   ├── ic.vhdl
    │       │       │   │   ├── idecompressor.vhdl
    │       │       │   │   ├── ifetch.vhdl
    │       │       │   │   ├── input_mux_4.vhdl
    │       │       │   │   ├── output_socket_1_1.vhdl
    │       │       │   │   └── output_socket_4_1.vhdl
    │       │       │   ├── ghdl_compile.sh
    │       │       │   ├── ghdl_simulate.sh
    │       │       │   ├── modsim_compile.sh
    │       │       │   ├── modsim_simulate.sh
    │       │       │   └── vhdl
    │       │       │       ├── lsu_le.vhdl
    │       │       │       ├── minimal_alu.vhd
    │       │       │       ├── rf_1wr_1rd_always_1_guarded_0.vhd
    │       │       │       ├── rtc_rtimer_always_1.vhd
    │       │       │       ├── shl_shr_shru.vhdl
    │       │       │       ├── tce_util_pkg.vhdl
    │       │       │       ├── tta0_globals_pkg.vhdl
    │       │       │       ├── tta0_params_pkg.vhdl
    │       │       │       ├── tta0.vhdl
    │       │       │       └── util_pkg.vhdl
    │       │       └── vhdl_generateprocessor.stamp
    │       ├── sim
    │       │   ├── log
    │       │   │   └── 2026_09_06_01_13_29
    │       │   │       ├── app.log
    │       │   │       ├── dataset
    │       │   │       │   ├── checkNodeOperation.csv
    │       │   │       │   ├── decode.csv
    │       │   │       │   ├── encode.csv
    │       │   │       │   ├── rate_matching.csv
    │       │   │       │   └── rate_recover.csv
    │       │   │       ├── run_info.json
    │       │   │       └── term.log
    │       │   └── post-proc
    │       │       ├── analysis
    │       │       └── parsing
    │       └── targets
    │           └── tta
    │               ├── asic
    │               │   └── kernels
    │               │       └── 5gnr
    │               │           └── ldpc
    │               │               └── 5gnr_ldpc_asic.tpef
    │               └── sim
    │                   └── kernels
    │                       └── 5gnr
    │                           └── ldpc
    │                               └── 5gnr_ldpc_sim.tpef
    ├── build-x86_64
    │   └── openasip
    │       ├── sim
    │       │   ├── log
    │       │   │   └── 2026_09_06_01_45_54
    │       │   │       ├── app.log
    │       │   │       ├── dataset
    │       │   │       │   ├── checkNodeOperation.csv
    │       │   │       │   ├── decode.csv
    │       │   │       │   ├── encode.csv
    │       │   │       │   ├── rate_matching.csv
    │       │   │       │   └── rate_recover.csv
    │       │   │       ├── run_info.json
    │       │   │       └── term.log
    │       │   └── post-proc
    │       │       ├── analysis
    │       │       └── parsing
    │       └── targets
    │           └── x86_64
    │               └── kernels
    │                   └── 5gnr
    │                       └── ldpc
    │                           └── x86_64-ldpc
    └── sim
        └── venv
            ├── include
            │   └── python3.12
            ├── lib64 -> lib
            └── pyvenv.cfg

48 directories, 61 files
```bash
