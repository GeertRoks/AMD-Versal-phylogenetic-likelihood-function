#####################################################################################################
#
# References:
# - v++ options: https://docs.xilinx.com/r/2022.2-English/ug1393-vitis-application-acceleration/v-Command
#
#####################################################################################################

## Parameters
# TARGET:      <sw_emu, hw_emu, hw> (default: sw_emu)
# PL:          <uint32, uint128, uint512, qdma32, qdma128, qdma512>
# AIE:         <streams: (bitwidth)stream(vector operations)[for], windows: (bitwidth)window(windowsize)(vector operations)[for]>
# BLOCKS:      <natural number> (default: 1)


PLATFORM := /opt/xilinx/platforms/xilinx_vck5000_gen4x8_qdma_2_202220_1/xilinx_vck5000_gen4x8_qdma_2_202220_1.xpfm
WORKSPACE := PLF

AIE ?= 128x9PLFwindow1inEV16288
PL ?= uint128x4window1inEV

PL_TYPE := $(patsubst uint128x4%,%,$(PL))

PL_FREQ := 400

# Targets: sw_emu, hw_emu, hw
TARGET := sw_emu

ifeq ($(TARGET),sw_emu)
    AIE_TARGET := x86sim
else
    AIE_TARGET := hw
endif

# Project source directories
DIR_HOST := app_$(WORKSPACE)
DIR_AIE := aie_$(WORKSPACE)
DIR_HLS := hls_$(WORKSPACE)_datamovers
DIR_BUILD := build

AIE_SRCS_MAIN := $(DIR_AIE)/src/$(AIE)/project.cpp
AIE_SRCS_OTHER := $(filter-out $(AIE_SRCS_MAIN), $(wildcard $(DIR_AIE)/src/$(AIE)/*))

# artefacts
AIE_LIBADF := $(DIR_BUILD)/$(AIE_TARGET)/aie/libadf_$(AIE).a
HLS_XO := $(DIR_BUILD)/$(TARGET)/hls/mm2sleft_$(PL).xo $(DIR_BUILD)/$(TARGET)/hls/mm2sright_$(PL).xo $(DIR_BUILD)/$(TARGET)/hls/s2mm_$(PL).xo
XSA := $(DIR_BUILD)/$(TARGET)/sys/$(WORKSPACE)_$(PL)_$(AIE).xsa
XCLBIN := $(DIR_BUILD)/$(TARGET)/$(WORKSPACE)_$(PL)_$(AIE).xclbin

VPP_LINK_DEPS := $(AIE_LIBADF) $(HLS_XO)
VPP_PACKAGE_DEPS := $(XSA) $(AIE_LIBADF)

NUM_AIE_IO := $(shell echo $(AIE) | sed -n 's/.*x\([0-9]*\).*/\1/p')


define VPP_CONNECTION_FLAGS_1_INPUT
--connectivity.nk mm2sleft:$(1):$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n "mm2sleft_$$i,"; done | sed 's/,$$//') \
--connectivity.nk mm2sright:$(1):$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n "mm2sright_$$i,"; done | sed 's/,$$//') \
--connectivity.nk s2mm:$(1):$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n "s2mm_$$i,"; done | sed 's/,$$//') \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc mm2sleft_$$i.s$$j:ai_engine_0.plio_in_$${i}_0_$${j}"; done; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc mm2sright_$$i.s$$j:ai_engine_0.plio_in_$${i}_1_$${j}"; done; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc ai_engine_0.plio_out_$${i}_$${j}:s2mm_$$i.s$$j"; done; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n " --connectivity.sc mm2sleft_$$i.sEV:ai_engine_0.plio_in_EV_$$i"; done)
endef

define VPP_CONNECTION_FLAGS_1_INPUT_EV
--connectivity.nk mm2sleft:$(1):$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n "mm2sleft_$$i,"; done | sed 's/,$$//') \
--connectivity.nk mm2sright:$(1):$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n "mm2sright_$$i,"; done | sed 's/,$$//') \
--connectivity.nk s2mm:$(1):$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n "s2mm_$$i,"; done | sed 's/,$$//') \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc mm2sleft_$$i.s$$j:ai_engine_0.plio_in_$${i}_0_$${j}"; done; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc mm2sright_$$i.s$$j:ai_engine_0.plio_in_$${i}_1_$${j}"; done; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc ai_engine_0.plio_out_$${i}_$${j}:s2mm_$$i.s$$j"; done; done)
endef

define VPP_CONNECTION_FLAGS_2_INPUTS
--connectivity.nk mm2sleft:$(1):$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n "mm2sleft_$$i,"; done | sed 's/,$$//') \
--connectivity.nk mm2sright:$(1):$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n "mm2sright_$$i,"; done | sed 's/,$$//') \
--connectivity.nk s2mm:$(1):$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n "s2mm_$$i,"; done | sed 's/,$$//') \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc mm2sleft_$$i.s$$j:ai_engine_0.plio_in_$${i}_0_$${j}"; done; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc mm2sright_$$i.s$$j:ai_engine_0.plio_in_$${i}_1_$${j}"; done; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc ai_engine_0.plio_out_$${i}_$${j}:s2mm_$$i.s$$j"; done; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc mm2sleft_$$i.sBranch$${j}:ai_engine_0.plio_in_branch_$${i}_0_$${j}"; done; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc mm2sright_$$i.sBranch$${j}:ai_engine_0.plio_in_branch_$${i}_1_$${j}"; done; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n " --connectivity.sc mm2sleft_$$i.sEV:ai_engine_0.plio_in_EV_$$i"; done)
endef

ifeq ($(PL_TYPE),window2in)
	VPP_CONNECTION_FLAGS := $(call VPP_CONNECTION_FLAGS_2_INPUTS,$(NUM_AIE_IO))
else ifeq ($(PL_TYPE),window1in)
	VPP_CONNECTION_FLAGS := $(call VPP_CONNECTION_FLAGS_1_INPUT,$(NUM_AIE_IO))
else ifeq ($(PL_TYPE),window1inEV)
	VPP_CONNECTION_FLAGS := $(call VPP_CONNECTION_FLAGS_1_INPUT_EV,$(NUM_AIE_IO))
else ifeq ($(PL_TYPE),stream2in)
	VPP_CONNECTION_FLAGS := $(call VPP_CONNECTION_FLAGS_2_INPUTS,$(NUM_AIE_IO))
else ifeq ($(PL_TYPE),stream1in)
	VPP_CONNECTION_FLAGS := $(call VPP_CONNECTION_FLAGS_1_INPUT,$(NUM_AIE_IO))
else ifeq ($(PL_TYPE),stream1inEV)
	VPP_CONNECTION_FLAGS := $(call VPP_CONNECTION_FLAGS_1_INPUT_EV,$(NUM_AIE_IO))
else ifeq ($(PL),genwindow1inEV)
	VPP_CONNECTION_FLAGS := $(call VPP_CONNECTION_FLAGS_1_INPUT_EV,$(NUM_AIE_IO))
else ifeq ($(PL),genstream1inEV)
	VPP_CONNECTION_FLAGS := $(call VPP_CONNECTION_FLAGS_1_INPUT_EV,$(NUM_AIE_IO))
else ifeq ($(PL),genwindow2in)
	VPP_CONNECTION_FLAGS := $(call VPP_CONNECTION_FLAGS_2_INPUTS,$(NUM_AIE_IO))
else ifeq ($(PL),genstream2in)
	VPP_CONNECTION_FLAGS := $(call VPP_CONNECTION_FLAGS_2_INPUTS,$(NUM_AIE_IO))
endif

#v++ flags
#VPP_PROFILE_FLAGS := --profile.stall=all:all:all --profile.data=all:all:all
#VPP_PROFILE_FLAGS := --profile.aie=all --profile.stall=all:all:all --profile.data=all:all:all --profile.exec=all:all
VPP_VIVADO_FLAGS := --vivado.impl.jobs 8 --vivado.synth.jobs 8
VPP_PACKAGE_FLAGS := --package.boot_mode ospi --package.out_dir $(DIR_BUILD)/$(TARGET)/package
#VPP_PACKAGE_FLAGS += --package.defer_aie_run
VPP_INTERMEDIATE_FILE_DIRS := --save-temps --temp_dir $(DIR_BUILD)/$(TARGET)/_x_$(PL)_$(AIE)/temp --report_dir $(DIR_BUILD)/$(TARGET)/_x_$(PL)_$(AIE)/reports --log_dir $(DIR_BUILD)/$(TARGET)/_x_$(PL)_$(AIE)/logs
VPP_HLS_FLAGS := --hls.jobs 8

GCC_HOST_FLAGS := -g -Wall -std=c++17
GCC_HOST_INCLUDES := -I$(DIR_HOST)/src -I${XILINX_XRT}/include -L${XILINX_XRT}/lib
GCC_HOST_LIBS := -lxrt_coreutil -luuid -pthread

ifdef PL_FREQ
	VPP_LINK_CLOCK_FLAGS := --clock.freqHz $(PL_FREQ)000000:$(shell for i in $$(seq 0 $$(($(NUM_AIE_IO)-1))); do echo -n "mm2sleft_$$i,mm2sright_$$i,s2mm_$$i,"; done | sed 's/,$$//')
	#VPP_LINK_CLOCK_FLAGS := --kernel_frequency 0:$(PL_FREQ)
	VPP_AIE_FLAGS := --pl-freq=$(PL_FREQ)
	GCC_HOST_FLAGS += -DPL_FREQ=$(PL_FREQ)
endif

ifdef NO_PRERUN_CHECK
	GCC_HOST_FLAGS += -DNO_PRERUN_CHECK=$(NO_PRERUN_CHECK)
endif
ifdef NO_CORRECTNESS_CHECK
	GCC_HOST_FLAGS += -DNO_CORRECTNESS_CHECK=$(NO_CORRECTNESS_CHECK)
endif
ifdef NO_INTERMEDIATE_RESULTS
	GCC_HOST_FLAGS += -DNO_INTERMEDIATE_RESULTS=$(NO_INTERMEDIATE_RESULTS)
endif

ifdef BLOCKS
	GCC_HOST_FLAGS += -DBLOCKS=$(BLOCKS)
endif



ifeq ($(strip ${XILINX_XRT}),)
	@echo "ERROR: XRT is not defined. First run: 'source /opt/xilinx/xrt/setup.sh' or for UT 'module load xilinx/xrt' and then run this make recipe again."
endif

dir_guard = @mkdir -p $(@D)
log_output := 2>&1 | tee screen_output.txt

#####################################################################################################

all: xclbin host

host: $(DIR_BUILD)/$(TARGET)/host.exe

host_gen: $(DIR_BUILD)/$(TARGET)/host_gen.exe

host_pcie: $(DIR_BUILD)/$(TARGET)/host_pcie.exe

xclbin: $(XCLBIN)

xsa: $(XSA)

aie: $(AIE_LIBADF)

hls: $(HLS_XO)

#####################################################################################################
CURRENT_DATE_TIME := $(shell date +%Y%m%d-%H%M%S)
PROJECT_ROOT := $(shell pwd)
DIR_EMU_LOGS := emulation

run_hw_pcie:
	$(DIR_BUILD)/hw/host_pcie.exe $(XCLBIN)

ALIGNMENTS ?= 100
#ALIGNMENT_SITES ?= 100 500 1000 5000 10000 50000 100000 500000 1000000 5000000 10000000
ALIGNMENT_SITES ?= 10000 50000 100000 500000 1000000 5000000 10000000 50000000 100000000 500000000 1000000000
PLF_CALLS ?= 1
INSTANCES_USED ?= 1
#TODO: read window size from xclbin instead of separate input parameter
WINDOW_SIZE ?= 1024

run_hw_gen:
	$(DIR_BUILD)/hw/host_gen.exe $(XCLBIN) $(ALIGNMENTS) $(PLF_CALLS) $(INSTANCES_USED) $(WINDOW_SIZE)

run_hw_gen_tests:
	for alignments in $(ALIGNMENT_SITES); do\
		$(DIR_BUILD)/hw/host_gen.exe $(XCLBIN) $$alignments $(PLF_CALLS) $(INSTANCES_USED) $(WINDOW_SIZE); \
	done

run_hw:
	$(DIR_BUILD)/hw/host.exe $(XCLBIN) $(ALIGNMENTS) $(PLF_CALLS) $(INSTANCES_USED) $(WINDOW_SIZE)

run_hw_tests:
	for alignments in $(ALIGNMENT_SITES); do\
		$(DIR_BUILD)/hw/host.exe $(XCLBIN) $$alignments $(PLF_CALLS) $(INSTANCES_USED) $(WINDOW_SIZE); \
	done

run_hw_emu: $(DIR_BUILD)/hw_emu/emconfig.json
	@echo "Running hw_emu @ $(CURRENT_DATE_TIME)"
	@echo "Project root $(PROJECT_ROOT)"
	@mkdir -p $(DIR_EMU_LOGS)/hw_emu
	export XCL_EMULATION_MODE=hw_emu; \
	export XRT_INI_PATH=$(shell pwd)/xrt.ini; \
	cd $(DIR_EMU_LOGS)/hw_emu; \
	$(PROJECT_ROOT)/$(DIR_BUILD)/hw_emu/host.exe $(PROJECT_ROOT)/$(XCLBIN); \
	cd -

run_sw_emu_pcie: $(DIR_BUILD)/sw_emu/emconfig.json
	@echo "Running sw_emu @ $(CURRENT_DATE_TIME)"
	@echo "Project root $(PROJECT_ROOT)"
	@mkdir -p $(DIR_EMU_LOGS)/sw_emu
	export XCL_EMULATION_MODE=sw_emu; \
	export XRT_INI_PATH=$(shell pwd)/xrt.ini; \
	cd $(DIR_EMU_LOGS)/sw_emu; \
	$(PROJECT_ROOT)/$(DIR_BUILD)/sw_emu/host_pcie.exe $(PROJECT_ROOT)/$(XCLBIN); \
	cd -

run_sw_emu_gen: $(DIR_BUILD)/sw_emu/emconfig.json
	@echo "Running sw_emu @ $(CURRENT_DATE_TIME)"
	@echo "Project root $(PROJECT_ROOT)"
	@mkdir -p $(DIR_EMU_LOGS)/sw_emu
	export XCL_EMULATION_MODE=sw_emu; \
	export XRT_INI_PATH=$(shell pwd)/xrt.ini; \
	cd $(DIR_EMU_LOGS)/sw_emu; \
	$(PROJECT_ROOT)/$(DIR_BUILD)/sw_emu/host_gen.exe $(PROJECT_ROOT)/$(XCLBIN) 100; \
	cd -

run_sw_emu: $(DIR_BUILD)/sw_emu/emconfig.json
	@echo "Running sw_emu @ $(CURRENT_DATE_TIME)"
	@echo "Project root $(PROJECT_ROOT)"
	@mkdir -p $(DIR_EMU_LOGS)/sw_emu
	export XCL_EMULATION_MODE=sw_emu; \
	export XRT_INI_PATH=$(shell pwd)/xrt.ini; \
	cd $(DIR_EMU_LOGS)/sw_emu; \
	$(PROJECT_ROOT)/$(DIR_BUILD)/sw_emu/host.exe $(PROJECT_ROOT)/$(XCLBIN) 203 $(PLF_CALLS) $(INSTANCES_USED); \
	cd -


aie_sim: $(DIR_BUILD)/hw/aie/libadf_$(AIE).a
	rm -r $(DIR_BUILD)/aiesimulator_output
	aiesimulator --pkg-dir=$(<D)/Work_$(AIE) --input-dir=$(DIR_AIE) --output-dir=$(DIR_BUILD)/aiesimulator_output --profile --dump-vcd=foo --output-time-stamp=no

aie_x86sim: $(DIR_BUILD)/x86sim/aie/libadf_$(AIE).a
	x86simulator --pkg-dir=$(<D)/Work_$(AIE) --input-dir=$(DIR_AIE) --output-dir=$(DIR_BUILD)/x86simulator_output

#####################################################################################################
# HOST

#$(DIR_BUILD)/$(TARGET)/app/%.o: $(DIR_HOST)/src/%.cpp
#	$(dir_guard)
#	$(CXX) $(GCC_HOST_FLAGS) $(GCC_HOST_INCLUDES) -c $@ $<

$(DIR_BUILD)/$(TARGET)/host.exe: $(DIR_HOST)/src/host.cpp $(DIR_HOST)/src/plf.cpp $(DIR_HOST)/src/utils.cpp
	$(dir_guard)
	$(CXX) $(GCC_HOST_FLAGS) $(GCC_HOST_INCLUDES) -o $@ $^ $(GCC_HOST_LIBS)

$(DIR_BUILD)/$(TARGET)/host_gen.exe: $(DIR_HOST)/src/host_gen.cpp $(DIR_HOST)/src/plf.cpp $(DIR_HOST)/src/utils.cpp
	$(dir_guard)
	$(CXX) $(GCC_HOST_FLAGS) $(GCC_HOST_INCLUDES) -o $@ $^ $(GCC_HOST_LIBS)

$(DIR_BUILD)/$(TARGET)/host_pcie.exe: $(DIR_HOST)/src/host_pcie.cpp $(DIR_HOST)/src/plf.cpp $(DIR_HOST)/src/utils.cpp
	$(dir_guard)
	$(CXX) $(GCC_HOST_FLAGS) $(GCC_HOST_INCLUDES) -o $@ $^ $(GCC_HOST_LIBS)


#####################################################################################################
# XCLBIN

$(XCLBIN): $(VPP_PACKAGE_DEPS)
	$(dir_guard)
	v++ -p --target $(TARGET) --platform $(PLATFORM) $(VPP_PACKAGE_FLAGS) $(VPP_INTERMEDIATE_FILE_DIRS) $^ -o $@

#####################################################################################################
# AIE GRAPH

$(DIR_BUILD)/%/aie/libadf_$(AIE).a: $(AIE_SRCS_MAIN) $(AIE_SRCS_OTHER)
	$(dir_guard)
	# For v++ version 2022.2:
	# Preferering to use 'v++ -c --mode aie' instead of aiecompiler, because it seems that aiecompiler is getting phased out. Also, aiecompiler has no option to place log files in a specific directory, which will clutter the workspace
	# Using the --aie_legacy flag to enable the use of aiecompiler flags using v++. Needed because the -o flag of v++ has no effect on libadf.a, so --ouput-archive of aiecompiler is used.
	# If this is fixed in a later version of v++, then remove the --aie_legacy flag and replace the --ouput-archive flag with -o
	#v++ -c --mode aie --target $* --platform $(PLATFORM) $(VPP_AIE_FLAGS) -I "${XILINX_VITIS}/aietools/include" -I "$(DIR_AIE)/src/$(AIE)" -I "$(DIR_AIE)/data" -I "$(DIR_AIE)/src/$(AIE)/kernels" -I "$(DIR_AIE)" --log_dir $(@D)/logs_$(AIE) --work_dir=$(@D)/Work_$(AIE) $< --aie_legacy --output-archive $@
	aiecompiler --target $* --platform $(PLATFORM) -I "${XILINX_VITIS}/aietools/include" -I "$(DIR_AIE)/src/$(AIE)" -I "$(DIR_AIE)/data" -I "$(DIR_AIE)/src/$(AIE)/kernels" -I "$(DIR_AIE)" --workdir=$(@D)/Work_$(AIE) $< --output-archive $@

#####################################################################################################
# XSA

$(XSA): $(VPP_LINK_DEPS)
	$(dir_guard)
	v++ -l -t $(TARGET) -g --platform $(PLATFORM) $(VPP_LINK_CLOCK_FLAGS) $(VPP_PROFILE_FLAGS) $(VPP_VIVADO_FLAGS) $(VPP_CONNECTION_FLAGS) $(VPP_INTERMEDIATE_FILE_DIRS) $^ -o $(XSA)

#####################################################################################################
# HLS

# hw
$(DIR_BUILD)/hw/hls/%.xo: $(DIR_HLS)/src/%.cpp $(DIR_HLS)/src/transpose.cpp
	$(dir_guard)
	v++ -c --target hw --platform $(PLATFORM) $(VPP_HLS_FLAGS) $(VPP_INTERMEDIATE_FILE_DIRS) -k $(firstword $(subst _, ,$*)) $^ -I$(DIR_HLS)/src -o $@

# hw emu
$(DIR_BUILD)/hw_emu/hls/%.xo: $(DIR_HLS)/src/%.cpp $(DIR_HLS)/src/transpose.cpp
	$(dir_guard)
	v++ -c --target hw_emu --platform $(PLATFORM) $(VPP_HLS_FLAGS) $(VPP_INTERMEDIATE_FILE_DIRS) -k $(firstword $(subst _, ,$*)) $^ -I$(DIR_HLS)/src -o $@

# sw_emu
$(DIR_BUILD)/sw_emu/hls/mm2sleft_%.xo: $(DIR_HLS)/src/mm2sleft_%.cpp $(DIR_HLS)/src/transpose.cpp
	$(dir_guard)
	#v++ -c --target $(TARGET) --platform $(PLATFORM) $(VPP_HLS_FLAGS) --hls.clock $(PL_FREQ)000000:$(firstword $(subst _, ,$*)) $(VPP_INTERMEDIATE_FILE_DIRS) -k $(firstword $(subst _, ,$*)) $^ -o $@
	v++ -c --target sw_emu --platform $(PLATFORM) $(VPP_HLS_FLAGS) $(VPP_INTERMEDIATE_FILE_DIRS) -k mm2sleft $^ -I$(DIR_HLS)/src -o $@

$(DIR_BUILD)/sw_emu/hls/mm2sright_%.xo: $(DIR_HLS)/src/mm2sright_%.cpp
	$(dir_guard)
	v++ -c --target sw_emu --platform $(PLATFORM) $(VPP_HLS_FLAGS) $(VPP_INTERMEDIATE_FILE_DIRS) -k mm2sright $^ -I$(DIR_HLS)/src -o $@

$(DIR_BUILD)/sw_emu/hls/s2mm_%.xo: $(DIR_HLS)/src/s2mm_%.cpp
	$(dir_guard)
	v++ -c --target sw_emu --platform $(PLATFORM) $(VPP_HLS_FLAGS) $(VPP_INTERMEDIATE_FILE_DIRS) -k s2mm $^ -I$(DIR_HLS)/src -o $@

#####################################################################################################
# emu config

%emconfig.json:
	emconfigutil --platform $(PLATFORM) --nd 1 --od $(@D)

#####################################################################################################
# clean

clean_x86sim:
	rm -r $(DIR_BUILD)/x86sim/
clean_sw_emu:
	rm -r $(DIR_BUILD)/sw_emu/
clean_hw_emu:
	rm -r $(DIR_BUILD)/hw_emu/
clean_hw:
	rm -r $(DIR_BUILD)/hw/

clean_build:
	rm -r $(DIR_BUILD)

clean:
	rm -f AIECompiler.log AIESimulator.log
	rm -f vitis_analyzer*.{log,jou}
	rm -f v++_$(WORKSPACE).log
	rm -f xcd.log
	rm -f device_trace_0.csv diag_report.log Map_Report.csv native_trace.csv sol.db summary.csv user_events.csv
	rm -f system_flat.wcfg system.wcfg system.wdb tmp.vcd.vcd vcdanalyze.log
	rm -f xrt.run_summary xsc_report.log
	rm -rf trdata.aiesim/


help:
	@echo "PL: $(PL)"
	@echo "AIE: $(AIE)"

