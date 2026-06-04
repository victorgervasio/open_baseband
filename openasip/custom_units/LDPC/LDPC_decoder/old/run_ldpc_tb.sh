# Compile with warnings suppressed for unsupported features
(iverilog -g2012 \
    -Wno-implicit \
    -o ldpc_tb \
    bn_proc.v \
    cn_proc.v \
    ldpc_tb.sv 2>&1) \
& vvp ldpc_tb
# vvp only if compilation succedes
