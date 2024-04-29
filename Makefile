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
# BUFFER_SIZE: <natural number> (default: 1048576)
# BLOCKS:      <natural number> (default: 1)


#PLATFORM := /opt/xilinx/platforms/xilinx_vck5000_gen4x8_qdma_2_202220_1/xilinx_vck5000_gen4x8_qdma_2_202220_1.xpfm
PLATFORM := /home/s2716879/xilinx_vck5000_gen4x8_qdma_2_202220_1/xilinx_vck5000_gen4x8_qdma_2_202220_1.xpfm
WORKSPACE := PLF
VERSION := PLFv1

PL := uint128
AIE := 128x1PLF

PL_FREQ := 300
AIE_FREQ := 300

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
DIR_BUILD := build/$(VERSION)

AIE_SRCS_MAIN := $(DIR_AIE)/src/$(VERSION)/$(AIE)/project.cpp
AIE_SRCS_OTHER := $(filter-out $(AIE_SRCS_MAIN), $(wildcard $(DIR_AIE)/src/$(VERSION)/$(AIE)/*))

# artefacts
AIE_LIBADF := $(DIR_BUILD)/$(AIE_TARGET)/aie/libadf_$(AIE).a
HLS_XO := $(DIR_BUILD)/$(TARGET)/hls/mm2s_$(PL).xo $(DIR_BUILD)/$(TARGET)/hls/s2mm_$(PL).xo
XSA := $(DIR_BUILD)/$(TARGET)/sys/$(WORKSPACE)_$(PL)_$(AIE).xsa
XCLBIN := $(DIR_BUILD)/$(TARGET)/$(WORKSPACE)_$(PL)_$(AIE).xclbin


define VPP_CONNECTION_FLAGS
--connectivity.nk mm2sleft:$(1):$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n "mm2sleft_$$i,"; done | sed 's/,$$//') \
--connectivity.nk mm2sright:$(1):$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n "mm2sright_$$i,"; done | sed 's/,$$//') \
--connectivity.nk s2mm:$(1):$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n "s2mm_$$i,"; done | sed 's/,$$//') \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc mm2sleft_$$i.s$$j:ai_engine_0.plio_in_$${i}_0_$${j}"; done; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc mm2sright_$$i.s$$j:ai_engine_0.plio_in_$${i}_1_$${j}"; done; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do for j in 0 1 2 3; do echo -n " --connectivity.sc ai_engine_0.plio_out_$${i}_$${j}:s2mm_$$i.s$$j"; done; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n " --connectivity.sc mm2sleft_$$i.sEV:ai_engine_0.plio_in_EV_$$i"; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n " --connectivity.sc mm2sleft_$$i.sBranch:ai_engine_0.plio_in_branch_$${i}_0"; done) \
$(shell for i in $$(seq 0 $$(($(1)-1))); do echo -n " --connectivity.sc mm2sright_$$i.sBranch:ai_engine_0.plio_in_branch_$${i}_1"; done)
endef

HLS_XO := $(DIR_BUILD)/$(TARGET)/hls/mm2sleft_$(PL).xo $(DIR_BUILD)/$(TARGET)/hls/mm2sright_$(PL).xo $(DIR_BUILD)/$(TARGET)/hls/s2mm_$(PL).xo
VPP_LINK_DEPS := $(AIE_LIBADF) $(HLS_XO)
VPP_PACKAGE_DEPS := $(XSA) $(AIE_LIBADF)

#v++ flags
VPP_PROFILE_FLAGS := --profile.aie=all --profile.stall=all:all:all --profile.data=all:all:all --profile.exec=all:all
VPP_VIVADO_FLAGS := --vivado.impl.jobs 8 --vivado.synth.jobs 8
VPP_PACKAGE_FLAGS := --package.boot_mode ospi --package.out_dir $(DIR_BUILD)/$(TARGET)/package
#VPP_PACKAGE_FLAGS += --package.defer_aie_run
VPP_INTERMEDIATE_FILE_DIRS := --save-temps --temp_dir $(DIR_BUILD)/$(TARGET)/_x_$(PL)_$(AIE)/temp --report_dir $(DIR_BUILD)/$(TARGET)/_x_$(PL)_$(AIE)/reports --log_dir $(DIR_BUILD)/$(TARGET)/_x_$(PL)_$(AIE)/logs
#VPP_LINK_CLOCK_FLAGS := --clock.freqHz $(PL_FREQ)000000:mm2s_0,s2mm_0 --clock.freqHz $(AIE_FREQ)000000:ai_engine_0
VPP_LINK_CLOCK_FLAGS := --kernel_frequency 0:$(PL_FREQ)
VPP_HLS_FLAGS := --hls.jobs 8
VPP_AIE_FLAGS := --pl-freq=$(PL_FREQ)

GCC_HOST_FLAGS := -g -Wall -std=c++17 -DAIE=$(AIE) -DPL=$(PL) -DPL_FREQ=$(PL_FREQ)
GCC_HOST_INCLUDES := -I$(DIR_HOST)/src -I${XILINX_XRT}/include -L${XILINX_XRT}/lib
GCC_HOST_LIBS := -lxrt_coreutil -luuid -pthread

ifdef NO_PRERUN_CHECK
	GCC_HOST_FLAGS += -DNO_PRERUN_CHECK=$(NO_PRERUN_CHECK)
endif

ifdef BLOCKS
	GCC_HOST_FLAGS += -DBLOCKS=$(BLOCKS)
endif


NUM_AIE_IO := $(shell echo $(AIE) | sed -n 's/.*x\([0-9]*\).*/\1/p')


ifeq ($(strip ${XILINX_XRT}),)
	@echo "ERROR: XRT is not defined. First run: 'source /opt/xilinx/xrt/setup.sh' or for UT 'module load xilinx/xrt' and then run this make recipe again."
endif

dir_guard = @mkdir -p $(@D)
log_output := 2>&1 | tee screen_output.txt

#####################################################################################################

all: xclbin host

host: $(DIR_BUILD)/$(TARGET)/host.exe

xclbin: $(XCLBIN)

xsa: $(XSA)

aie: $(AIE_LIBADF)

hls: $(HLS_XO)

#####################################################################################################
CURRENT_DATE_TIME := $(shell date +%Y%m%d-%H%M%S)
PROJECT_ROOT := $(shell pwd)
DIR_EMU_LOGS := emulation/$(VERSION)

BUFFER_SIZE ?= 8388608
CHUNK_SIZE ?= 1048576

CHUNK_SIZES ?= 1048576 2097152 4194304
BUFFER_SIZES ?= 33554432

run_hw:
	$(DIR_BUILD)/hw/host.exe $(XCLBIN)

run_hw_tests:
	for buf in $(BUFFER_SIZES); do\
		for chunk in $(CHUNK_SIZES); do\
			$(DIR_BUILD)/hw/host.exe $(XCLBIN) $$buf $$chunk; \
		done; \
	done

run_sw_emu: $(DIR_BUILD)/sw_emu/emconfig.json
	@echo "Running sw_emu @ $(CURRENT_DATE_TIME)"
	@echo "Project root $(PROJECT_ROOT)"
	@mkdir -p $(DIR_EMU_LOGS)/sw_emu
	export XCL_EMULATION_MODE=sw_emu; \
	export XRT_INI_PATH=$(shell pwd)/xrt.ini; \
	cd $(DIR_EMU_LOGS)/sw_emu; \
	$(PROJECT_ROOT)/$(DIR_BUILD)/sw_emu/host.exe $(PROJECT_ROOT)/$(XCLBIN); \
	cd -


aie_sim: $(DIR_BUILD)/hw/aie/libadf_$(AIE).a
	aiesimulator --pkg-dir=$(<D)/Work_$(AIE) --online -wdb -ctf --input-dir=$(DIR_AIE) --output-dir=$(DIR_BUILD)/aiesimulator_output

aie_x86sim: $(DIR_BUILD)/x86sim/aie/libadf_$(AIE).a
	x86simulator --pkg-dir=$(<D)/Work_$(AIE) --input-dir=$(DIR_AIE) --output-dir=$(DIR_BUILD)/x86simulator_output

#####################################################################################################

#$(DIR_BUILD)/$(TARGET)/app/%.o: $(DIR_HOST)/src/%.cpp
#	$(dir_guard)
#	$(CXX) $(GCC_HOST_FLAGS) $(GCC_HOST_INCLUDES) -c $@ $<

$(DIR_BUILD)/$(TARGET)/host.exe: $(DIR_HOST)/src/host_$(VERSION).cpp $(DIR_HOST)/src/plf.cpp $(DIR_HOST)/src/utils.cpp
	$(dir_guard)
	$(CXX) $(GCC_HOST_FLAGS) $(GCC_HOST_INCLUDES) -o $@ $^ $(GCC_HOST_LIBS)

#####################################################################################################


$(XCLBIN): $(VPP_PACKAGE_DEPS)
	$(dir_guard)
	v++ -p --target $(TARGET) --platform $(PLATFORM) $(VPP_PACKAGE_FLAGS) $(VPP_INTERMEDIATE_FILE_DIRS) $^ -o $@


$(DIR_BUILD)/%/aie/libadf_$(AIE).a: $(AIE_SRCS_MAIN) $(AIE_SRCS_OTHER)
	$(dir_guard)
	# For v++ version 2022.2:
	# Preferering to use 'v++ -c --mode aie' instead of aiecompiler, because it seems that aiecompiler is getting phased out. Also, aiecompiler has no option to place log files in a specific directory, which will clutter the workspace
	# Using the --aie_legacy flag to enable the use of aiecompiler flags using v++. Needed because the -o flag of v++ has no effect on libadf.a, so --ouput-archive of aiecompiler is used.
	# If this is fixed in a later version of v++, then remove the --aie_legacy flag and replace the --ouput-archive flag with -o
	v++ -c --mode aie --target $* --platform $(PLATFORM) $(VPP_AIE_FLAGS) -I "${XILINX_VITIS}/aietools/include" -I "$(DIR_AIE)/src/$(VERSION)/$(AIE)" -I "$(DIR_AIE)/data" -I "$(DIR_AIE)/src/$(VERSION)/$(AIE)/kernels" -I "$(DIR_AIE)" --log_dir $(@D)/logs_$(AIE) --work_dir=$(@D)/Work_$(AIE) $< --aie_legacy --output-archive $@
	#aiecompiler --target $* --platform $(PLATFORM) -I "${XILINX_VITIS}/aietools/include" -I "$(DIR_AIE)/src" -I "$(DIR_AIE)/data" -I "$(DIR_AIE)/src/kernels" -I "$(DIR_AIE)" --workdir=$(@D)/Work $< --output-archive $@


$(XSA): $(VPP_LINK_DEPS)
	$(dir_guard)
	v++ -l -t $(TARGET) -g --platform $(PLATFORM) $(VPP_LINK_CLOCK_FLAGS) $(VPP_PROFILE_FLAGS) $(VPP_VIVADO_FLAGS) $(call VPP_CONNECTION_FLAGS,$(NUM_AIE_IO)) $(VPP_INTERMEDIATE_FILE_DIRS) $^ -o $(XSA)

$(DIR_BUILD)/$(TARGET)/hls/%.xo: $(DIR_HLS)/src/%.cpp
	$(dir_guard)
	v++ -c --target $(TARGET) --platform $(PLATFORM) $(VPP_HLS_FLAGS) --hls.clock $(PL_FREQ)000000:$(firstword $(subst _, ,$*)) $(VPP_INTERMEDIATE_FILE_DIRS) -k $(firstword $(subst _, ,$*)) $^ -o $@

%emconfig.json:
	emconfigutil --platform $(PLATFORM) --nd 1 --od $(@D)

#####################################################################################################

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

