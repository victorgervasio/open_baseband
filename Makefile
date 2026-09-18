# open_baseband top-level build interface
#
# The Makefile is intentionally a thin wrapper around CMake.  It also owns
# local tool/dependency setup that must happen before CMake is configured.

SHELL := /bin/bash
.ONESHELL:
.DEFAULT_GOAL := all

ROOT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
OPENASIP_DIR := $(ROOT_DIR)/openasip
BUILD_ROOT ?= $(ROOT_DIR)/build/openasip
OPENASIP_TARGET ?= all
BUILD_DIR ?= $(BUILD_ROOT)/build-$(OPENASIP_TARGET)
SIM_OUTPUT_DIR ?= $(BUILD_DIR)/openasip/sim
DATASET_LOG_DIR ?= # to generate dataset for specific target's log (default is the latest)
BACKUP_LOG_DIR ?= $(ROOT_DIR)/backup/log

# OpenASIP environment.  tce-env.sh is kept in the repository under config.
TCE_ENV ?= $(OPENASIP_DIR)/config/tce-env.sh

# ETL is kept as a project-local dependency.  Replace this default with the
# URL of your fork, or invoke make with ETL_REPO=....
#ETL_REPO ?= https://github.com/victorgervasio/etl.git
#ETL_BRANCH ?= openasip_devel
ETL_ROOT ?= $(OPENASIP_DIR)/targets/common/kernels/lib/cpp/etl
ETL_BUILD_DIR ?= $(ETL_ROOT)/build
ETL_INSTALL_DIR ?= $(ETL_ROOT)/install

# OpenASIP simulator used by the generic TTA run target.
SIMULATOR ?= ttasim

# Python environment used by the OpenASIP simulation scripts.
SIM_SOURCE_DIR ?= $(OPENASIP_DIR)/sim
SIM_BUILD_DIR ?= $(BUILD_ROOT)/sim
SIM_VENV ?= $(SIM_BUILD_DIR)/venv
SIM_PYTHON ?= $(SIM_VENV)/bin/python3
SIM_REQUIREMENTS ?= $(SIM_SOURCE_DIR)/requirements.txt

# OpenASIP processor Hardware Description Language.
RTL_HDL ?= vhdl

CMAKE ?= cmake
GIT ?= git
CMAKE_BUILD_TYPE ?= Release # RelWithDebInfo for getting debug symbols
WORKTREE_ROOT ?= $(ROOT_DIR)/.worktrees

NPROCS := $(shell nproc)
NPAR := $(shell expr $(NPROCS) - 1)
ifeq ($(NPAR),0)
    NPAR := 1
endif

.PHONY: all build configure ensure-etl ensure-sim-venv ensure-hdb
.PHONY: build-tta build-x86_64 build-almaif
.PHONY: build-tta-sim build-tta-asic
.PHONY: tta x86_64 almaif
.PHONY: simulate simulate-tta simulate-x86_64 simulate-almaif
.PHONY: verilog-rtl vhdl-rtl 
.PHONY: dataset-tta dataset-x86_64 dataset-almaif dataset
.PHONY: analyze-tta analyze-x86_64 analyze-almaif analyze
.PHONY: backup-logs
.PHONY: clean distclean
.PHONY: format lint help
.PHONY: worktree-add worktree-rm

# Every public recipe enters the OpenASIP environment first.  This does not
# modify the caller's interactive shell; it only affects the shell running
# the current make recipe.  Re-sourcing is harmless and keeps each invocation
# self-contained and reproducible.
define SOURCE_TCE
if [[ ! -f "$(TCE_ENV)" ]]; then
	echo "ERROR: OpenASIP environment file not found: $(TCE_ENV)" >&2
	exit 1
fi
source "$(TCE_ENV)"
endef

all: OPENASIP_TARGET=all
all: configure 
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target all -j$(NPAR) -- --output-sync=line

build: OPENASIP_TARGET=all
build: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target build -j$(NPAR) -- --output-sync=line

configure: ensure-etl ensure-sim-venv ensure-hdb
	$(SOURCE_TCE)
	$(CMAKE) -S "$(ROOT_DIR)" -B "$(BUILD_DIR)" \
		-DETL_ROOT="$(ETL_ROOT)" \
		-DETL_INSTALL_DIR="$(ETL_INSTALL_DIR)" \
		-DTCE_ENV="$(TCE_ENV)" \
		-DSIMULATOR="$(SIMULATOR)" \
		-DOPENASIP_TARGET="$(OPENASIP_TARGET)" \
		-DPython3_EXECUTABLE="$(SIM_PYTHON)" \
		-DRTL_HDL="$(RTL_HDL)" \
		-DSIM_OUTPUT_DIR="$(SIM_OUTPUT_DIR)" \
		-DDATASET_LOG_DIR="$(DATASET_LOG_DIR)" \
		-DCMAKE_BUILD_TYPE="$(CMAKE_BUILD_TYPE)" \
		-DOPENASIP_OPSET_BASE_PATH="$(OPENASIP_OPSET_BASE_PATH)" \
        -DCMAKE_C_FLAGS="-fdiagnostics-color=always" \
        -DCMAKE_CXX_FLAGS="-fdiagnostics-color=always" \
        -DCMAKE_COLOR_DIAGNOSTICS=ON

# Clone and build/install ETL locally.  CMake configuration of open_baseband
# depends on this target so `make`, `make all`, and every build-* target are
# self-contained.
#ensure-etl:
#	$(SOURCE_TCE)
#	if [[ -z "$(ETL_REPO)" ]]; then
#		echo "ERROR: ETL_REPO is not set." >&2
#		echo "Set it to your ETL fork, for example:" >&2
#		echo "  make ETL_REPO=https://github.com/victorgervasio/etl.git" >&2
#		exit 1
#	fi
#	if [[ ! -d "$(ETL_ROOT)/.git" ]]; then
#		echo "==> Cloning ETL ($(ETL_BRANCH)) into $(ETL_ROOT)"
#		mkdir -p "$$(dirname "$(ETL_ROOT)")"
#		$(GIT) clone --branch "$(ETL_BRANCH)" "$(ETL_REPO)" "$(ETL_ROOT)"
#	else
#		echo "==> ETL already cloned: $(ETL_ROOT)"
#	fi
#	if [[ ! -f "$(ETL_ROOT)/CMakeLists.txt" ]]; then
#		echo "ERROR: ETL source at $(ETL_ROOT) does not contain CMakeLists.txt" >&2
#		exit 1
#	fi
#	if [[ ! -f "$(ETL_INSTALL_DIR)/.open_baseband_etl_installed" ]]; then
#		echo "==> Configuring ETL"
#		$(CMAKE) -S "$(ETL_ROOT)" -B "$(ETL_BUILD_DIR)" \
#			-DCMAKE_INSTALL_PREFIX="$(ETL_INSTALL_DIR)"
#		echo "==> Building/installing ETL"
#		$(CMAKE) --build "$(ETL_BUILD_DIR)" --target install -j$(NPAR) -- --output-sync=line
#		mkdir -p "$(ETL_INSTALL_DIR)"
#		touch "$(ETL_INSTALL_DIR)/.open_baseband_etl_installed"
#	else
#		echo "==> ETL already built/installed: $(ETL_INSTALL_DIR)"
#	fi
ensure-etl:
	$(SOURCE_TCE)

	echo "==> Initializing ETL submodule"
	$(GIT) submodule update --init --recursive "$(ETL_ROOT)"

	if [[ ! -f "$(ETL_ROOT)/CMakeLists.txt" ]]; then
		echo "ERROR: ETL source at $(ETL_ROOT) does not contain CMakeLists.txt" >&2
		echo "Make sure the ETL submodule is initialized correctly." >&2
		exit 1
	fi

	ETL_COMMIT="$$( $(GIT) -C "$(ETL_ROOT)" rev-parse HEAD )"
	ETL_STAMP="$(ETL_INSTALL_DIR)/.open_baseband_etl_installed"

	echo "==> ETL submodule revision: $$ETL_COMMIT"

	ETL_INSTALLED_COMMIT=""
	if [[ -f "$$ETL_STAMP" ]]; then
		ETL_INSTALLED_COMMIT="$$(cat "$$ETL_STAMP")"
	fi

	if [[ "$$ETL_INSTALLED_COMMIT" != "$$ETL_COMMIT" ]]; then
		if [[ -n "$$ETL_INSTALLED_COMMIT" ]]; then
			echo "==> ETL revision changed:"
			echo "    installed: $$ETL_INSTALLED_COMMIT"
			echo "    requested: $$ETL_COMMIT"
		else
			echo "==> ETL has not been installed yet"
		fi

		echo "==> Configuring ETL"
		$(CMAKE) -S "$(ETL_ROOT)" -B "$(ETL_BUILD_DIR)" \
			-DCMAKE_INSTALL_PREFIX="$(ETL_INSTALL_DIR)"

		echo "==> Building/installing ETL"
		$(CMAKE) --build "$(ETL_BUILD_DIR)" --target install \
			-j$(NPAR) -- --output-sync=line

		mkdir -p "$(ETL_INSTALL_DIR)"
		printf '%s\n' "$$ETL_COMMIT" > "$$ETL_STAMP"
	else
		echo "==> ETL already built/installed for commit $$ETL_COMMIT"
	fi

ensure-sim-venv:
	$(SOURCE_TCE)
	if [[ ! -d "$(SIM_VENV)" ]]; then
		echo "==> Creating Python virtual environment: $(SIM_VENV)"
		python3 -m venv "$(SIM_VENV)"
	fi

	if [[ ! -x "$(SIM_PYTHON)" ]]; then
		echo "ERROR: Python virtual environment is missing: $(SIM_PYTHON)" >&2
		exit 1
	fi

	if [[ ! -f "$(SIM_REQUIREMENTS)" ]]; then
		echo "ERROR: Python requirements file not found: $(SIM_REQUIREMENTS)" >&2
		exit 1
	fi

	echo "==> Activating Python virtual environment"
	source "$(SIM_VENV)/bin/activate"

	echo "==> Installing Python dependencies"
	"$(SIM_PYTHON)" -m pip install -r "$(SIM_REQUIREMENTS)"

# Initialize the custom OpenASIP HDB submodule and check out the
# exact revision required by this open_baseband revision.
ensure-hdb:
	$(SOURCE_TCE)

	echo "==> Initializing custom OpenASIP HDB submodule"
	$(GIT) submodule update --init --recursive "$(OPENASIP_DIR)/arch/custom/hdb"

	if [[ ! -d "$(OPENASIP_DIR)/arch/custom/hdb" ]]; then
		echo "ERROR: Custom HDB submodule was not initialized." >&2
		exit 1
	fi

	echo "==> Custom OpenASIP HDB revision:"
	$(GIT) -C "$(OPENASIP_DIR)/arch/custom/hdb" rev-parse HEAD

	echo "==> Fetching Git LFS objects for custom OpenASIP HDB"
	$(GIT) -C "$(OPENASIP_DIR)/arch/custom/hdb" lfs pull

build-tta: OPENASIP_TARGET=tta
build-tta: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target build-tta -j$(NPAR) -- --output-sync=line
	#$(CMAKE) --build "$(BUILD_DIR)" --target build-tta

build-tta-sim: OPENASIP_TARGET=tta
build-tta-sim: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target build-tta-sim -j$(NPAR) -- --output-sync=line

build-tta-asic: OPENASIP_TARGET=tta
build-tta-asic: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target build-tta-asic -j$(NPAR) -- --output-sync=line

build-x86_64: OPENASIP_TARGET=x86_64
build-x86_64: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target build-x86_64 -j$(NPAR) -- --output-sync=line

build-almaif: OPENASIP_TARGET=almaif
build-almaif: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target build-almaif -j$(NPAR) -- --output-sync=line

tta: OPENASIP_TARGET=tta
tta: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target tta -j$(NPAR) -- --output-sync=line

x86_64: OPENASIP_TARGET=x86_64
x86_64: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target x86_64 -j$(NPAR) -- --output-sync=line

almaif: OPENASIP_TARGET=almaif
almaif: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target almaif -j$(NPAR) -- --output-sync=line

simulate: OPENASIP_TARGET=all
simulate: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target simulate -j$(NPAR) -- --output-sync=line

simulate-tta: OPENASIP_TARGET=tta
simulate-tta: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target simulate-tta -j$(NPAR) -- --output-sync=line

simulate-x86_64: OPENASIP_TARGET=x86_64
simulate-x86_64: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target simulate-x86_64 -j$(NPAR) -- --output-sync=line

simulate-almaif: OPENASIP_TARGET=almaif
simulate-almaif: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target simulate-almaif -j$(NPAR) -- --output-sync=line

verilog-rtl: OPENASIP_TARGET=tta
verilog-rtl: RTL_HDL=verilog
verilog-rtl: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target verilog-rtl -j$(NPAR) -- --output-sync=line

vhdl-rtl: OPENASIP_TARGET=tta
vhdl-rtl: RTL_HDL=vhdl
vhdl-rtl: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target vhdl-rtl -j$(NPAR) -- --output-sync=line

dataset-tta: OPENASIP_TARGET=tta
dataset-tta: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target dataset-tta -j$(NPAR) -- --output-sync=line

dataset-x86_64: OPENASIP_TARGET=x86_64
dataset-x86_64: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target dataset-x86_64 -j$(NPAR) -- --output-sync=line

dataset-almaif: OPENASIP_TARGET=almaif
dataset-almaif: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target dataset-almaif -j$(NPAR) -- --output-sync=line

build: BUILD_DIR=$(BUILD_ROOT)/all
build: OPENASIP_TARGET=all
dataset: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target dataset -j$(NPAR) -- --output-sync=line

analyze: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target analyze -j$(NPAR) -- --output-sync=line

format: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target format -j$(NPAR) -- --output-sync=line

lint: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target lint -j$(NPAR) -- --output-sync=line

worktree:
	@if [[ -z "$(branch)" ]]; then \
		echo "ERROR: branch is required."; \
		echo "Usage: make worktree branch=<branch-name>"; \
		exit 1; \
	fi
	@if ! $(GIT) show-ref --verify --quiet "refs/heads/$(branch)"; then \
		echo "ERROR: local branch '$(branch)' does not exist."; \
		exit 1; \
	fi
	@if [[ -e "$(WORKTREE_ROOT)/$(branch)" ]]; then \
		echo "ERROR: worktree path already exists:"; \
		echo "  $(WORKTREE_ROOT)/$(branch)"; \
		exit 1; \
	fi
	mkdir -p "$(WORKTREE_ROOT)"
	echo "==> Creating worktree for branch '$(branch)'"
	$(GIT) worktree add "$(WORKTREE_ROOT)/$(branch)" "$(branch)"
	echo "==> Worktree created:"
	echo "    $(WORKTREE_ROOT)/$(branch)"

worktree-rm:
	@if [[ -z "$(branch)" ]]; then \
		echo "ERROR: branch is required."; \
		echo "Usage: make worktree-remove branch=<branch-name>"; \
		exit 1; \
	fi
	@if [[ ! -e "$(WORKTREE_ROOT)/$(branch)" ]]; then \
		echo "ERROR: worktree does not exist:"; \
		echo "  $(WORKTREE_ROOT)/$(branch)"; \
		exit 1; \
	fi
	echo "==> Removing worktree '$(branch)'"
	$(GIT) worktree remove "$(WORKTREE_ROOT)/$(branch)"

backup-logs:
	@echo "==> Backing up simulation logs"
	@if [[ ! -d "$(BUILD_ROOT)" ]]; then \
		echo "ERROR: build directory does not exist: $(BUILD_ROOT)" >&2; \
		exit 1; \
	fi
	@mkdir -p "$(BACKUP_LOG_DIR)/openasip"

	find "$(BUILD_ROOT)" -type d -path '*/sim/log' -print0 | \
	while IFS= read -r -d '' logdir; do \
		rel="$${logdir#$(BUILD_ROOT)/}"; \
		dest="$(BACKUP_LOG_DIR)/openasip/$$(dirname "$$rel")/log"; \
		echo "    $$logdir -> $$dest"; \
		mkdir -p "$$dest"; \
		rsync -a "$$logdir/" "$$dest/"; \
	done

	@echo "==> Logs backed up to:"
	@echo "    $(BACKUP_LOG_DIR)/openasip"

clean:
	$(SOURCE_TCE)
	for dir in "$(BUILD_ROOT)"/*; do
		if [[ -d "$$dir" ]]; then
			$(CMAKE) --build "$$dir" --target clean -j$(NPAR) -- --output-sync=line
		fi
	done

# Remove the CMake build tree.  ETL is deliberately preserved because it is a
# source dependency and should not be recloned merely because the build tree
# was cleaned.
distclean:
	$(SOURCE_TCE)
	rm -rf "$(BUILD_ROOT)"

help:
	@echo "open_baseband build targets:"
	@echo "  make / make all          Configure dependencies and build everything"
	@echo "  make build               Build everything"
	@echo "  make build-tta           Build TTA target"
	@echo "  make build-tta-sim       Build TTA target compiling simulation kernels"
	@echo "  make build-tta-asic      Build TTA target compiling asic kernels"
	@echo "  make build-x86_64        Build x86_64 target"
	@echo "  make build-almaif        Build ALMAIF target"
	@echo "  make tta                 Build TTA and run it"
	@echo "  make x86_64              Build x86_64 and run it"
	@echo "  make almaif              Build ALMAIF and run it"
	@echo "  make simulate[-<target>] Run an already configured simulation target"
	@echo "  make verilog-rtl         Generate processor RTL in Verilog"
	@echo "  make vhdl-rtl            Generate processor RTL in VHDL"
	@echo "  make dataset             Generate timing CSVs"
	@echo "  make analyze             Run analysis"
	@echo "  make backup-logs         Backup simulation logs outside build tree"
	@echo "  make clean               Clean CMake build artifacts"
	@echo "  make distclean           Remove the CMake build tree"
	@echo ""
	@echo "Configuration variables:"
	@echo "  TCE_ENV=$(TCE_ENV)"
	#@echo "  ETL_REPO=$(ETL_REPO)"
	#@echo "  ETL_BRANCH=$(ETL_BRANCH)"
	@echo "  ETL_ROOT=$(ETL_ROOT)"
	@echo "  SIMULATOR=$(SIMULATOR)"
	@echo "  BUILD_DIR=$(BUILD_DIR)"
	@echo "  SIM_VENV=$(SIM_VENV)"
	@echo "  SIM_PYTHON=$(SIM_PYTHON)"
