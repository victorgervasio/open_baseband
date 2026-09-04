Current project file tree:

```bash
open\_baseband
├── CMakeLists.txt
├── LICENSE
├── Makefile
├── openasip
│   ├── arch
│   │   ├── 5gnr\_ue\_baseband\_proc.adf
│   │   └── rtl
│   │       └── CMakeLists.txt
│   ├── CMakeLists.txt
│   ├── config
│   │   └── tce-env.sh
│   ├── sim
│   │   ├── CMakeLists.txt
│   │   ├── post-proc
│   │   │   └── gen\_dataset.py
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
│       │       │           │   ├── gen\_n0\_table.cpp
│       │       │           │   ├── gen\_n0\_table.sh
│       │       │           │   └── n0\_table.h
│       │       │           ├── gen-random
│       │       │           │   ├── generate\_random\_samples.cpp
│       │       │           │   ├── generate\_random\_samples.sh
│       │       │           │   ├── normal\_samples.h
│       │       │           │   └── norm\_dist.h
│       │       │           ├── LDPC.cpp
│       │       │           ├── main.cpp
│       │       │           └── nrLDPCTables.cpp     
│       │       └── common
│       │           └── lib
│       │               └── cpp
│       └── x86\_64
│           └── kernels
│               ├── 5gnr
│               │   └── ldpc
│               │       ├── CMakeLists.txt
│               │       ├── include
│               │       │   ├── defines.h
│               │       │   └── LDPC.h
│               │       └── src
│               │           ├── gen-n0
│               │           │   ├── gen\_n0\_table.cpp
│               │           │   ├── gen\_n0\_table.sh
│               │           │   └── n0\_table.h
│               │           ├── gen-random
│               │           │   ├── generate\_random\_samples.cpp
│               │           │   ├── generate\_random\_samples.sh
│               │           │   ├── normal\_samples.h
│               │           │   └── norm\_dist.h
│               │           ├── LDPC.cpp
│               │           ├── main.cpp
│               │           └── nrLDPCTables.cpp
│               └── lib
│                   └── cpp
└── README.md

52 directories, 57 files
