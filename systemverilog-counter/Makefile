REPO_ROOT := ..
ARCILATOR_UTILS_ROOT ?= $(dir $(shell which arcilator))

ARCRUNTIME ?= 0
ARCRUNTIME_INCLUDE_DIR   ?= $(ARCILATOR_UTILS_ROOT)/../../include
ARCRUNTIME_CIRCT_LIB_DIR ?= $(ARCILATOR_UTILS_ROOT)/../lib
ARCRUNTIME_LLVM_LIB_DIR  ?= $(dir $(shell which llc))/../lib
ARCRUNTIME_LINK_LIBS     := -lCIRCTArcRuntime -lLLVMSupport -lLLVMDemangle

CXXFLAGS = -O3 -Wall -std=c++17
LDFLAGS =
ifeq ($(shell uname), Linux)
	CXXFLAGS += -no-pie
	LDFLAGS += -latomic
endif

BUILD_DIR ?= build/$(CONFIG)
$(shell mkdir -p $(BUILD_DIR))

SOURCE_MODEL ?= counter
BUILD_MODEL ?= $(BUILD_DIR)/counter

ARCILATOR_ARGS ?= --mlir-timing --print-debug-info --mlir-pass-statistics --async-resets-as-sync

ifeq ($(TRACE),1)
	ARCILATOR_ARGS += --observe-wires --observe-ports --observe-named-values --observe-registers --observe-memories
	VERILATOR_ARGS += --trace --trace-underscore
	CXXFLAGS += -DTRACE
else
	ARCILATOR_ARGS += --observe-wires=0 --observe-ports=0 --observe-named-values=0 --observe-registers=0 --observe-memories=0
endif

ifeq ($(ARCRUNTIME),1)
    ARCRUNTIME_CXXFLAGS  = -DARC_USE_COMPILED_RUNTIME_LIB
    ARCRUNTIME_CXXFLAGS += -I$(ARCRUNTIME_INCLUDE_DIR)
    ARCRUNTIME_LD_FLAGS  = -L$(ARCRUNTIME_CIRCT_LIB_DIR)
    ARCRUNTIME_LD_FLAGS += -L$(ARCRUNTIME_LLVM_LIB_DIR)
    ARCRUNTIME_LD_FLAGS += $(ARCRUNTIME_LINK_LIBS)
endif

#===------------------------------------------------------------------------===
# SystemVerilog to core dialect IR
#===------------------------------------------------------------------------===

$(BUILD_DIR)/$(SOURCE_MODEL).mlir: $(SOURCE_MODEL).sv | $(BUILD_DIR)
	circt-verilog --ir-hw --top $(TOP) $< -o $@

#===-------------------------------------------------------------------------===
# Arcilator
#===-------------------------------------------------------------------------===

$(BUILD_MODEL)-arc.o $(BUILD_MODEL).json &: $(BUILD_MODEL).mlir
	arcilator $< --state-file=$(BUILD_MODEL).json -o $(BUILD_MODEL)-arc.ll $(ARCILATOR_ARGS)
	opt -O3 -S $(BUILD_MODEL)-arc.ll | llc -O3 --filetype=obj -o $(BUILD_MODEL)-arc.o
	objdump -d $(BUILD_MODEL)-arc.o > $(BUILD_MODEL)-arc.s

$(BUILD_MODEL)-arc.h: $(BUILD_MODEL).json
	python3 $(ARCILATOR_UTILS_ROOT)/arcilator-header-cpp.py $< --view-depth 1 > $@

#===-------------------------------------------------------------------------===
# Convenience
#===-------------------------------------------------------------------------===

run: $(BUILD_MODEL)-main
	$(BUILD_MODEL)-main $(BINARY) $(RUN_ARGS)

run-arcs: RUN_ARGS += --arcs
run-arcs: run
run-trace: RUN_ARGS += --trace $(BUILD_MODEL).vcd
run-trace: run

.PHONY: all run run-arcs run-trace clean
