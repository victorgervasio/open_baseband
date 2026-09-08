#!/usr/bin/bash
SCRIPT_DIR="$( cd -- "$( dirname -- "${BASH_SOURCE[0]:-$0}"; )" &> /dev/null && pwd 2> /dev/null; )";
#OpenASIP LLVM Environment Variables
#source me every time before using openasip toolkit!!!
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/local/lib
export PATH=$HOME/local/bin:$PATH
export LDFLAGS=-L$HOME/local/lib

export TCE_OSAL_PATH=${SCRIPT_DIR}/../arch/custom/osal/kernels/5gnr/ldpc

#source "${SCRIPT_DIR}/rtl_generator_completion.sh"
#alias rtl_generator="${SCRIPT_DIR}/rtl_generator.exe"
