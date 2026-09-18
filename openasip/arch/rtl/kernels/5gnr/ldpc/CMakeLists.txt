if(RTL_HDL STREQUAL "verilog")
        add_custom_target(verilog-rtl-5gnr-ldpc)
endif()

if(RTL_HDL STREQUAL "vhdl")
    add_custom_target(vhdl-rtl-5gnr-ldpc)
endif()

add_subdirectory(cores/encode)
add_subdirectory(cores/decode)

if(RTL_HDL STREQUAL "verilog")
    add_dependencies(verilog-rtl-5gnr-ldpc verilog-rtl-5gnr-ldpc-encode verilog-rtl-5gnr-ldpc-decode)
endif()

if(RTL_HDL STREQUAL "vhdl")
    add_dependencies(vhdl-rtl-5gnr-ldpc vhdl-rtl-5gnr-ldpc-encode vhdl-rtl-5gnr-ldpc-decode)
endif()
