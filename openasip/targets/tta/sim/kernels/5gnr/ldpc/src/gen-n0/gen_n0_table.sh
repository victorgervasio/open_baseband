PROJECT_ROOT="/home/victor/openasip_personal_designs/ldpc_cn_project/tce_tutorials/tce_tour/LDPC/NR5G"
g++ ${PROJECT_ROOT}/5G-NR-LDPC-CPP/source_files/gen-n0/gen_n0_table.cpp \
    -I${PROJECT_ROOT}/5G-NR-LDPC-CPP/source_files --std=c++14 -g -O3 -o \
    ${PROJECT_ROOT}/5G-NR-LDPC-CPP/source_files/gen-n0/gen_n0_table
