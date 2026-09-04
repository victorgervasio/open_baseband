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

# OpenASIP environment.  tce-env.sh is kept in the repository under config.
TCE_ENV ?= $(OPENASIP_DIR)/config/tce-env.sh

# ETL is kept as a project-local dependency.  Replace this default with the
# URL of your fork, or invoke make with ETL_REPO=....
ETL_REPO ?= https://github.com/victorgervasio/etl.git
ETL_BRANCH ?= openasip_devel
ETL_ROOT ?= $(OPENASIP_DIR)/targets/common/kernels/lib/cpp/etl
ETL_BUILD_DIR ?= $(ETL_ROOT)/build
ETL_INSTALL_DIR ?= $(ETL_ROOT)/install

# OpenASIP simulator used by the generic TTA run target.
SIMULATOR ?= ttasim

# Python environment used by the OpenASIP simulation scripts.
SIM_DIR ?= $(OPENASIP_DIR)/sim
SIM_VENV ?= $(SIM_DIR)/venv
SIM_PYTHON ?= $(SIM_VENV)/bin/python
SIM_REQUIREMENTS ?= $(SIM_DIR)/requirements.txt

CMAKE ?= cmake
GIT ?= git

.PHONY: all build configure ensure-etl ensure-sim-venv
.PHONY: build-tta build-x86_64 build-almaif
.PHONY: tta x86_64 almaif
.PHONY: simulate simulate-tta simulate-x86_64 simulate-almaif
.PHONY: ttasim proxim
.PHONY: rtl dataset analyze
.PHONY: clean distclean test test-tta test-x86_64 test-almaif
.PHONY: format lint help

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

all: build

build: BUILD_DIR=$(BUILD_ROOT)/all
build: OPENASIP_TARGET=all
build: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target build-all

configure: ensure-etl ensure-sim-venv
configure: ensure-etl ensure-sim-venv
	$(SOURCE_TCE)
	$(CMAKE) -S "$(ROOT_DIR)" -B "$(BUILD_DIR)" \
		-DETL_ROOT="$(ETL_ROOT)" \
		-DETL_INSTALL_DIR="$(ETL_INSTALL_DIR)" \
		-DTCE_ENV="$(TCE_ENV)" \
		-DSIMULATOR="$(SIMULATOR)" \
		-DOPENASIP_TARGET="$(OPENASIP_TARGET)" \
		-DPython3_EXECUTABLE="$(SIM_PYTHON)"

# Clone and build/install ETL locally.  CMake configuration of open_baseband
# depends on this target so `make`, `make all`, and every build-* target are
# self-contained.
ensure-etl:
	$(SOURCE_TCE)
	if [[ -z "$(ETL_REPO)" ]]; then
		echo "ERROR: ETL_REPO is not set." >&2
		echo "Set it to your ETL fork, for example:" >&2
		echo "  make ETL_REPO=https://github.com/victorgervasio/etl.git" >&2
		exit 1
	fi
	if [[ ! -d "$(ETL_ROOT)/.git" ]]; then
		echo "==> Cloning ETL ($(ETL_BRANCH)) into $(ETL_ROOT)"
		mkdir -p "$$(dirname "$(ETL_ROOT)")"
		$(GIT) clone --branch "$(ETL_BRANCH)" "$(ETL_REPO)" "$(ETL_ROOT)"
	else
		echo "==> ETL already cloned: $(ETL_ROOT)"
	fi
	if [[ ! -f "$(ETL_ROOT)/CMakeLists.txt" ]]; then
		echo "ERROR: ETL source at $(ETL_ROOT) does not contain CMakeLists.txt" >&2
		exit 1
	fi
	if [[ ! -f "$(ETL_INSTALL_DIR)/.open_baseband_etl_installed" ]]; then
		echo "==> Configuring ETL"
		$(CMAKE) -S "$(ETL_ROOT)" -B "$(ETL_BUILD_DIR)" \
			-DCMAKE_INSTALL_PREFIX="$(ETL_INSTALL_DIR)"
		echo "==> Building/installing ETL"
		$(CMAKE) --build "$(ETL_BUILD_DIR)" --parallel --target install
		mkdir -p "$(ETL_INSTALL_DIR)"
		touch "$(ETL_INSTALL_DIR)/.open_baseband_etl_installed"
	else
		echo "==> ETL already built/installed: $(ETL_INSTALL_DIR)"
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

build-tta: OPENASIP_TARGET=tta
build-tta: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target tta-build

build-x86_64: OPENASIP_TARGET=x86_64
build-x86_64: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target x86_64-build

build-almaif: OPENASIP_TARGET=almaif
build-almaif: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target almaif-build

tta: OPENASIP_TARGET=tta
tta: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target tta

x86_64: OPENASIP_TARGET=x86_64
x86_64: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target x86_64

almaif: OPENASIP_TARGET=almaif
almaif: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target almaif

simulate: OPENASIP_TARGET=all
simulate: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target simulate

simulate-tta: OPENASIP_TARGET=tta
simulate-tta: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target simulate-tta

simulate-x86_64: OPENASIP_TARGET=x86_64
simulate-x86_64: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target simulate-x86_64

simulate-almaif: OPENASIP_TARGET=almaif
simulate-almaif: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target simulate-almaif

ttasim: OPENASIP_TARGET=tta
ttasim: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target ttasim

proxim: OPENASIP_TARGET=tta
proxim: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target proxim

rtl: OPENASIP_TARGET=tta
rtl: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target rtl

dataset: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target dataset

analyze: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target analyze

test: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target test

test-tta: OPENASIP_TARGET=tta
test-tta: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target test-tta

test-x86_64: OPENASIP_TARGET=x86_64
test-x86_64: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target test-x86_64

test-almaif: OPENASIP_TARGET=almaif
test-almaif: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --parallel --target test-almaif

format: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target format

lint: configure
	$(SOURCE_TCE)
	$(CMAKE) --build "$(BUILD_DIR)" --target lint

clean:
	$(SOURCE_TCE)
	for dir in "$(BUILD_ROOT)"/*; do
		if [[ -d "$$dir" ]]; then
			$(CMAKE) --build "$$dir" --target clean
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
	@echo "  make build-x86_64        Build x86_64 target"
	@echo "  make build-almaif        Build ALMAIF target"
	@echo "  make tta                 Build TTA and run it"
	@echo "  make x86_64              Build x86_64 and run it"
	@echo "  make almaif              Build ALMAIF and run it"
	@echo "  make simulate[-<target>] Run an already configured simulation target"
	@echo "  make ttasim              Run TTA with ttasim"
	@echo "  make proxim              Run TTA with Proxim"
	@echo "  make rtl                Generate processor RTL"
	@echo "  make dataset             Generate timing CSVs"
	@echo "  make analyze             Run analysis"
	@echo "  make test[-<target>]     Run correctness tests"
	@echo "  make clean               Clean CMake build artifacts"
	@echo "  make distclean           Remove the CMake build tree"
	@echo ""
	@echo "Configuration variables:"
	@echo "  TCE_ENV=$(TCE_ENV)"
	@echo "  ETL_REPO=$(ETL_REPO)"
	@echo "  ETL_BRANCH=$(ETL_BRANCH)"
	@echo "  ETL_ROOT=$(ETL_ROOT)"
	@echo "  SIMULATOR=$(SIMULATOR)"
	@echo "  BUILD_DIR=$(BUILD_DIR)"
	@echo "  SIM_VENV=$(SIM_VENV)"
	@echo "  SIM_PYTHON=$(SIM_PYTHON)"
