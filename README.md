Current project file tree:

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
│       │       │           ├── nrLDPCTables.cpp
│       │       │           └── temp
│       │       │               ├── compiling\_tests
│       │       │               │   └── llvm\_background
│       │       │               │       ├── large.ll
│       │       │               │       ├── memset2.cpp
│       │       │               │       ├── memset2.ll
│       │       │               │       ├── memset.cpp
│       │       │               │       ├── rint2.cpp
│       │       │               │       ├── rint3.cpp
│       │       │               │       ├── rint4.cpp
│       │       │               │       ├── rint5.cpp
│       │       │               │       └── rint.cpp
│       │       │               ├── dev-support
│       │       │               │   ├── headers\_test.cpp
│       │       │               │   └── headers\_test.sh
│       │       │               └── openasip\_tools
│       │       │                   ├── compile
│       │       │                   │   ├── host\_compile.sh
│       │       │                   │   └── oacc\_compile.sh
│       │       │                   ├── debug
│       │       │                   │   └── foo.py
│       │       │                   └── simulate
│       │       │                       ├── proxim\_simulate.sh
│       │       │                       └── ttasim\_simulate.sh
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
├── README.md
└── temp
    └── structure.txt

52 directories, 57 files
