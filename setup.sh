export OPEN_BASEBAND_ROOT_DIR_PATH="$PWD"
make -C "$OPEN_BASEBAND_ROOT_DIR_PATH/sdk"
source ~/.bashrc
make -C "$OPEN_BASEBAND_ROOT_DIR_PATH/sdk" LLVM_ENABLE_LLD=ON
source ~/.bashrc
