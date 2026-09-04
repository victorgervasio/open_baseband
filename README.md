Current project file tree:

```bash
open_baseband
├── CMakeLists.txt
├── LICENSE
├── Makefile
├── openasip
│   ├── arch
│   │   ├── 5gnr_ue_baseband_proc.adf
│   │   └── rtl
│   │       └── CMakeLists.txt
│   ├── CMakeLists.txt
│   ├── config
│   │   └── tce-env.sh
│   ├── sim
│   │   ├── CMakeLists.txt
│   │   ├── post-proc
│   │   │   └── gen_dataset.py
│   │   ├── requirements.txt
│   │   └── sim.py
│   └── targets
│       ├── almaif
│       │   └── kernels
│       │       ├── 5gnr
│       │       │   └── ldpc
│       │       │       ├── CMakeLists.txt
│       │       │       ├── custom
│       │       │       ├── include
│       │       │       └── src
│       │       └── lib
│       │           └── cpp
│       ├── CMakeLists.txt
│       ├── common
│       │   └── kernels
│       │       └── lib
│       │           └── cpp
│       ├── tta
│       │   └── kernels
│       │       ├── 5gnr
│       │       │   └── ldpc
│       │       │       ├── CMakeLists.txt
│       │       │       ├── custom
│       │       │       ├── include
│       │       │       │   ├── defines.h
│       │       │       │   └── LDPC.h
│       │       │       └── src
│       │       │           ├── gen-n0
│       │       │           │   ├── gen_n0_table.cpp
│       │       │           │   ├── gen_n0_table.sh
│       │       │           │   └── n0_table.h
│       │       │           ├── gen-random
│       │       │           │   ├── generate_random_samples.cpp
│       │       │           │   ├── generate_random_samples.sh
│       │       │           │   ├── normal_samples.h
│       │       │           │   └── norm_dist.h
│       │       │           ├── LDPC.cpp
│       │       │           ├── main.cpp
<<<<<<< HEAD
│       │       │           └── nrLDPCTables.cpp
||||||| 3c8049c
│       │       │           └── nrLDPCTables.cpp
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
│       │       │           
=======
│       │       │           └── nrLDPCTables.cpp     
>>>>>>> origin/main
│       │       └── common
│       │           └── lib
│       │               └── cpp
│       └── x86_64
│           └── kernels
│               ├── 5gnr
│               │   └── ldpc
│               │       ├── CMakeLists.txt
│               │       ├── include
│               │       │   ├── defines.h
│               │       │   └── LDPC.h
│               │       └── src
│               │           ├── gen-n0
│               │           │   ├── gen_n0_table.cpp
│               │           │   ├── gen_n0_table.sh
│               │           │   └── n0_table.h
│               │           ├── gen-random
│               │           │   ├── generate_random_samples.cpp
│               │           │   ├── generate_random_samples.sh
│               │           │   ├── normal_samples.h
│               │           │   └── norm_dist.h
│               │           ├── LDPC.cpp
│               │           ├── main.cpp
│               │           └── nrLDPCTables.cpp
│               └── lib
│                   └── cpp
└── README.md

52 directories, 57 files
