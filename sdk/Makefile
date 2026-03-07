# Hexagon LLVM Toolchain Builder
# ======================================

# ---------- Configurable Parameters ----------
LLVM_REPO      := https://github.com/llvm/llvm-project.git
LLVM_BRANCH    := main

LLVM_DIR       := llvm-project
LLVM_SRC       := $(LLVM_DIR)/llvm

BUILD_DIR      := build
INSTALL_DIR    := $(BUILD_DIR)/install
BIN_DIR        := $(INSTALL_DIR)/bin

GENERATOR      := Ninja
BUILD_TYPE     := RelWithDebInfo

LLVM_PROJECTS  := clang;lld
LLVM_TARGETS   := Hexagon;RISCV

CMAKE_BIN      := cmake
NINJA_BIN      := ninja
LLD_BIN        := lld
CLANG_BIN      := clang

UNAME_S := $(shell uname -s)

# ---------- Optional Parameters ----------
SHALLOW ?= 1
LLVM_ENABLE_LLD ?= OFF   

# -------- Install paths for tools ----------
NINJA_PATH := $(shell if command -v $(NINJA_BIN) >/dev/null 2>&1; then dirname $$(command -v $(NINJA_BIN)); else echo ""; fi)
LLD_PATH   := $(shell if command -v $(LLD_BIN) >/dev/null 2>&1; then dirname $$(command -v $(LLD_BIN)); else echo ""; fi)
CLANG_PATH := $(shell if command -v $(CLANG_BIN) >/dev/null 2>&1; then dirname $$(command -v $(CLANG_BIN)); else echo ""; fi)
CMAKE_PATH := $(shell if command -v $(CMAKE_BIN) >/dev/null 2>&1; then dirname $$(command -v $(CMAKE_BIN)); else echo ""; fi)

# ==============================================
# Default target
all: cmake clang lld ninja llvm configure build install 

# -------- Ensure CMake is installed --------
cmake:
	@echo "Checking for CMake..."
	@if [ -n "$(CMAKE_PATH)" ] && [ -x "$(CMAKE_PATH)/$(CMAKE_BIN)" ]; then \
		echo "CMake already installed at $(CMAKE_PATH)"; \
	else \
		echo "Installing CMake..."; \
		if [ "$(UNAME_S)" = "Linux" ]; then \
			sudo apt update && sudo apt install -y cmake; \
		elif [ "$(UNAME_S)" = "Darwin" ]; then \
			brew install cmake; \
		else \
			echo "Please install CMake manually."; exit 1; \
		fi; \
		CMAKE_PATH=$$(dirname $$(command -v $(CMAKE_BIN))); \
	fi

# -------- Ensure Clang is installed --------
clang:
	@echo "Checking for Clang..."
	@if [ -n "$(CLANG_PATH)" ] && [ -x "$(CLANG_PATH)/$(CLANG_BIN)" ]; then \
		echo "Clang already installed at $(CLANG_PATH)"; \
	else \
		echo "Installing Clang..."; \
		if [ "$(UNAME_S)" = "Linux" ]; then \
			sudo apt update && sudo apt install -y clang; \
		elif [ "$(UNAME_S)" = "Darwin" ]; then \
			brew install llvm; \
		else \
			echo "Please install Clang manually."; exit 1; \
		fi; \
		CLANG_PATH=$$(dirname $$(command -v $(CLANG_BIN))); \
	fi

# -------- Ensure LLD is installed (optional) --------
lld:
	@if [ "$(LLVM_ENABLE_LLD)" = "ON" ]; then \
		echo "Checking for LLD..."; \
		if [ -n "$(LLD_PATH)" ] && [ -x "$(LLD_PATH)/$(LLD_BIN)" ]; then \
			echo "LLD already installed at $(LLD_PATH)"; \
		else \
			echo "Installing LLD..."; \
			if [ "$(UNAME_S)" = "Linux" ]; then \
				sudo apt update && sudo apt install -y lld; \
			elif [ "$(UNAME_S)" = "Darwin" ]; then \
				brew install llvm; \
			else \
				echo "Please install LLD manually."; exit 1; \
			fi; \
			LLD_PATH=$$(dirname $$(command -v $(LLD_BIN))); \
		fi \
	else \
		echo "Skipping LLD installation (LLVM_ENABLE_LLD=$(LLVM_ENABLE_LLD))"; \
	fi

# -------- Install Ninja --------
ninja:
	@echo "Checking for Ninja..."
	@if [ -n "$(NINJA_PATH)" ] && [ -x "$(NINJA_PATH)/$(NINJA_BIN)" ]; then \
		echo "Ninja already installed at $(NINJA_PATH)"; \
	else \
		echo "Installing Ninja..."; \
		if [ "$(UNAME_S)" = "Linux" ]; then \
			sudo apt update && sudo apt install -y ninja-build; \
		elif [ "$(UNAME_S)" = "Darwin" ]; then \
			brew install ninja; \
		else \
			echo "Please install Ninja manually."; exit 1; \
		fi; \
		NINJA_PATH=$$(dirname $$(command -v $(NINJA_BIN))); \
	fi

# -------- Clone LLVM --------
llvm:
	@if [ -d "$(LLVM_DIR)" ]; then \
		echo "LLVM repository already exists."; \
	else \
		echo "Cloning LLVM into $(LLVM_DIR)..."; \
		if [ "$(SHALLOW)" = "1" ]; then \
			git clone --depth 1 --branch $(LLVM_BRANCH) $(LLVM_REPO); \
		else \
			git clone --branch $(LLVM_BRANCH) $(LLVM_REPO); \
		fi; \
	fi

# -------- Configure --------
configure:
	@echo "Configuring LLVM build (LLVM_ENABLE_LLD=$(LLVM_ENABLE_LLD))..."
	@PATH=$(CMAKE_PATH):$(CLANG_PATH):$(LLD_PATH):$(NINJA_PATH):$$PATH \
	CC=$(CLANG_PATH)/clang \
	CXX=$(CLANG_PATH)/clang++ \
	$(CMAKE_BIN) -S $(LLVM_SRC) -B $(BUILD_DIR) -G Ninja \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(INSTALL_DIR) \
		-DLLVM_ENABLE_PROJECTS="$(LLVM_PROJECTS)" \
		-DLLVM_TARGETS_TO_BUILD="$(LLVM_TARGETS)" \
		-DLLVM_ENABLE_LLD=$(LLVM_ENABLE_LLD) \
		-DLLVM_ENABLE_ASSERTIONS=OFF \
		-DLLVM_ENABLE_RTTI=ON \
		-DLLVM_ENABLE_TERMINFO=OFF

# -------- Build --------
build:
	$(NINJA_BIN) -C $(BUILD_DIR) -j$$(nproc --ignore=1 2>/dev/null || sysctl -n hw.ncpu)
#	$(NINJA_BIN) -C $(BUILD_DIR) -j12

# -------- Install --------
install:
	$(NINJA_BIN) -C $(BUILD_DIR) -j$$(nproc --ignore=1 2>/dev/null || sysctl -n hw.ncpu) install 
#	$(NINJA_BIN) -C $(BUILD_DIR) -j12 install

# -------- Clean --------
clean:
	rm -rf $(BUILD_DIR)

# -------- Distclean --------
distclean: clean
	rm -rf $(LLVM_DIR)
	@BASHRC="$(HOME)/.bashrc"; \
	if grep -q "# >>> Hexagon LLVM toolchain >>>" "$$BASHRC" 2>/dev/null; then \
		sed -i.bak '/# >>> Hexagon LLVM toolchain >>>/,/# <<< Hexagon LLVM toolchain <<</d' "$$BASHRC"; \
		echo "Removed Hexagon toolchain PATH from $$BASHRC (backup saved as $$BASHRC.bak)"; \
	fi

# ==============================================
.PHONY: all clang lld ninja llvm configure build install clean distclean
