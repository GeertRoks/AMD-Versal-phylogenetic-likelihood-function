# Phylogenetic Likelihood Function Accelerator for Versal Adaptive SoCs

Implementation of the Phylogenetic Likelihood Function (PLF) for the AMD Versal Adaptive SoC architecture. The Versal Adaptive SoCs have both Programmable Logic (PL) and an array of vector processors called AI Engines (AIE). This repository contains the code for the system described in my thesis for the completion of my master Embedded Systems at the University of Twente.

The PLF is a common calculation in the field of phylogenetics that collapses two child nodes of a binary tree into their parent node, where each node represents a conditional likelihood vector (CLV) with the statistical information of an organism reprsented by that node. The result of the PLF is the CLV of the parent node. The main computations of the PLF are matrix multiplications and scaling of the results to prevent underflow.

Tested for the AMD Versal VC1902 chip on a VCK5000 datacenter card using _v++ 2022.2_ and _XRT library 2023.2_.

## Usage
To start using the accelerator, it first needs to be compiled. We compile the accelerator source code into an `.xclbin` file that can be loaded onto the Versal platform, as well as the host program that controls the accelerator. Assuming that Vitis 2022.2 and XRT 2022.2 are installed, the accelerator is compiled using the following command. Specify the platform file (`.xpfm` file, part of [installation](https://xilinx.github.io/Alveo-Versal-Platforms/alveoversalplatforms/build/html/docs/getting-started/install_development.html)) if it is different than the default location at `/opt/xilinx/platforms/xilinx_vck5000_gen4x8_qdma_2_202220_1/xilinx_vck5000_gen4x8_qdma_2_202220_1.xpfm`.
~~~
make xclbin host PLATFORM=<path/to/vck5000/platform/file.xpfm>
~~~

When the compilation is done, we run the accelerator with the following command:
~~~
make run PLATFORM=<path/to/vck5000/platform/file.xpfm>
~~~

The accelerator can be build for various targets and by default it builds and runs it for the *Software Emulation* target. This means that only a functional implementation of the accelerator runs on a Versal emulator on the CPU. Refer to the [Makefile options](#makefile-options) section to read more about the different build targets. Get the full performance using the *Hardware* target. Additionally, there are multiple configurations of the accelerator available which are described in the [Accelerator configurations](#accelerator-configurations) section.




## Project info
The system  consists of four main parts: 1) an array of AI Engines, 2) programmable logic, 3) device memory, 4) host CPU. The host CPU runs a host program that controls the accelerator that is implemented on the AI Engines and programmable logic, as well as move input data over PCIe to the device memroy. On the programmable logic PL kernels are implemented that read from the device memory and send it to the AI Engines, where AIE kernels run and execute the PLF calculation. The output is received again by an output PL kernel on the programmable logic, which also scales the values when necessary. Eventually the output PL kernel writes the results back to the device memory, where it can be read back to the host program over PCIe.

The matrix multiplication of the PLF are executed on the AI Engines. The calculations for one PLF accelerator are divided over many AIE kernels which are organized in a AIE graph. A top-level AIE graph can hold multiple accelerator graphs and also routes the communication between the programmable logic kernel and the accelerator graphs.

The following sections provide information about the details of this repository.

#### Accelerator configurations
The PLF can be implemented for various data and in various ways. The following shows the current configuration parameters.

###### Type of data
 Currently the PLF is only implemented for DNA data (4 states), but it could be implemented for any other type of date with more or fewer states.

###### PLIO input layout
TODO: add info (separate or combined)


###### AI Engine inter-kernel communication method
TODO: add info (stream or window)


###### Data input source
TODO: add info (mem or gen)

#### Makefile options
The following makefile variables can be used to control details of the accelerators

***NOTE: Make sure that the `XILINX_XRT` bash environment variable is set.***  
Check using `echo $XILINX_XRT`. If it is not set then the makefile will not be able to compile the host program.So to load the XRT library use the setup script in the XRT install location `source /opt/xilinx/xrt/setup.sh`.

###### Accelerator platform options
- `PLATFORM`: point to the platform `.xpfm` file of the VCK5000
    - default: `/opt/xilinx/platforms/xilinx_vck5000_gen4x8_qdma_2_202220_1/xilinx_vck5000_gen4x8_qdma_2_202220_1.xpfm`
- `TARGET`: select which compilation target is used. Also refer to the [AMD documentation UG1393](https://docs.amd.com/r/2022.2-English/ug1393-vitis-application-acceleration/Build-Targets) for more info on the different build targets.
    - `sw_emu` - (default) _CPU emulation of only the accelerator functionality_
    - `hw_emu` - _CPU emulation of the accelerator functionality and timing details_
    - `hw` - _Full implemenation of the accelerator that runs on the Versal adaptive SoC_
- `AIE_TYPE`: the AIE inter-kernel communication method, either `stream` or `window` (default: `window`)
- `WINDOW_SIZE`: the size of the window in bytes, ignored when `AIE_TYPE` is set to `stream` (default: 8192)
- `PLIO_LAYOUT`: the PLIO input layout, either `Sep` for separate input layout or `Comb` for combined input layout (default: `Comb`)
- `NUM_ACCELERATORS`: the number of accelerator instances to map onto the VCK5000 (default: `9`)
- `INPUT_SRC`: source of the input data, either `mem` for the PL reading the input data from the device memory or `gen` for the PL generating random input data (default: `mem`)
- `STATES`: the type of data (default: `DNA`)
- `AIE`: directly select the AIE configuration. Use the same name as the [directory](#file-structure) that you want to use. If not set, then it uses the above options, otherwise it overwrites them
    - default: `128x9DNAwindow8192Comb`
- `PL`: directly select the programable logic HLS kernel configuration for the three kernels (match it with the AIE configuration). Use the same naming convention as described in the hls paragraph of the [file structure](#file-structure) section. If not set, then it uses the above options, otherwise it overwrites them
    - default: `memDNAwindowComb`
- `PL_FREQ`: set the frequency of the programmable logic in MHz (default: 250MHz)


###### Host program options
- `NO_PRERUN_CHECKS`:  defaults to `0`, which will let the host program ask for confirmation of the provided input before accelerator execution. If set to `1` during compilation of host program, then no confirmation is asked before the execution of the accelerator.
- `NO_INTERMEDIATE_RESULTS`: defaults to `0`, which will let the host program measure intermediate steps of the accelerator execution and report execution time of the individual steps. The steps are 1) data movement from host to device over PCIe, 2) plf execution, 3) data mvement back from device to host over PCIe. If this option is set to `1` during host program compilation, then only a total time is measured of the complete execution, which also removes a few synchronisation steps, giving slightly better performance.
- `NO_CORRECTNESS_CHECK`: defaults to `0`, which will let the host program check the output of the accelerator against a known-good CPU implementation and check for corretness of the PLF implementation. If set to `1` during host program compilation, then this check is skipped, which reduces test time and host memory usage.


###### Runtime options
- `DEVICE`: the user BDF of the device you want to use (can be found using `xbutil examine`), (default: `0000:5e:00.1`)
- `ALIGNMENTS`: the length of the conditional likelihood vector (CLV) to be used for the run current (default: 100)
- `INSTANCES_USED`: number of accelerator instances to be used from the total defined by `NUM_ACCELERATORS` (default: 1)
- `PLF_CALLS`: the number of repetitions for the same test to understand the variation (default: 1)
- (`ALIGNMENT_SITES`: list containing all CLV lengths that need to be tested. I feel like this could be set from the command line, but I have not been able to do this, so it has to be edited in the Makefile itself)



#### File structure
The root folder of the project is called the *workspace*. This workspace contains at least the following directories where the Makefile will look for certain files
- `aie/`
    - `data/`: contains test data for [AIE simulation](#1.-aie-simulation)
    - `src/`: contains source code for AI Engine kernels. Organized in directories, where each directory is a configuration indicated by the name:
    `<plio-width>x<graphs>DNA<aie-type><plio-layout>`
        - __plio-width__ is the bit width of the PLIO channels between the PL and the AIE graphs
        - __graphs__ is the number of accelerator graphs that are defined in the top graph
        - __aie-type__ is the inter-AIE-kernel communication method used, either `stream`, `window1024`, `window8192` or `window16384`. Where the value after the window indicates the window size in bytes
        - __plio-layout__ is the configuration of the input PLIO, either `Sep` or `Comb`

- `hls/`
    - `src/`: contains source code of HLS kernels using the naming scheme:  
    `<kernel>_<mem/gen>DNA<aie-type><plio-layout>`
        - for one accelerator instance we need all of the following three __kernels__:
            - __mm2sleft__ is the data mover from memory (mm) to (2) stream of the AIE array (s) for the left side of the PLF calculation
            - __mm2sright__ is the data mover from memory (mm) to (2) stream of the AIE array (s) for the right side of the PLF calculation
            - __s2mm__ is the datamover from stream of the AIE array (s) to (2) memory (mm) for the result of the PLF calculation, as well as scaling
        - there are also different hls kernel configurations that *need to be matched to the AIE configuration*:
            - **mem/gen** are the two PL versions, where `gen` is a special PL kernel that does not read from memory, but generates test data to measure PL-AIE communication performance. So, generally choose `mem` for normal operation where the PL kernels read the input data from the memory of the VCK5000 card.
            - __aie-type__ the hls kernels need to provide the data differently depending on the inter-AIE-kernel communication method used, either `window` or `stream`. No window size needs to be indicated, the PL kernels manage that in run time
            - __plio-layout__ is the configuration of the input PLIO to the AI Engines, either `Sep` or `Comb`

- `app/`
    - `src/`: contains the code for the program that controls the PLF accelerator
        - the host program expects 5 parameters:
            - path to the xclbin
            - number of alignment patterns
            - parallel accelerator instances used
            - window size (**TODO: set this variable automatically from the xclbin name**)

## Development

#### Building steps

The xclbin consists of many parts which all can be build individually. Also check out the [AMD documentation](https://github.com/Xilinx/Vitis-Tutorials/tree/2022.2/AI_Engine_Development/Feature_Tutorials/05-AI-engine-versal-integration) that describes the various build steps to create an xclbin. The makefile consists of the folowing targets:
- `aie`: only compiles the AIE graph into a `libadf.a` library
- `hls`: only compiles the HLS kernels into `.xo` files
- `xsa`: links the AIE graph and HLS kernels into a `.xsa` file, where their connections are defined by [connectivity flags](https://docs.amd.com/r/2022.2-English/ug1393-vitis-application-acceleration/connectivity-Options)
- `xclbin`: packages the `.xsa` with the `libadf.a` into a `.xclbin` file for use on Versal Adaptive SoC
- `host`:  build the host program that controls the `.xclbin`

All build artifacts are stored in the `build/` directory.

#### Development process
Follow the following development steps to effectively test your implementations:

###### 1. AIE simulation
Test the AIE graph individually with test data defined in the `aie/data/` directory. Use this to check wheter the AIE graph is functionally correct.
```
make aie_x86sim <makefile options>
```

When the AIE graph is functionally correct, use the following to test the performance of the AIE graph individually. This also uses the test data from `aie/data/`, but not only simulates the functionality, it also simulates the timing of the AIE graph as if it were running on the Versal hardware. Use profiling options in the `project.cpp` file to measure various parts of the AIE performance. Remeber that the project.cpp file does not run in real time, so measure clock cycles and not time.
```
make aie_sim <makefile options>
```

###### 2. HLS testbench
*TODO: add HLS testbench to test both functionality and performance of the HLS separate from the rest of the system.*

###### 3a. Software emulation
When you are happy with both the AIE and HLS kernels, then integrate them together and test it using software emulation. This builds quickly, thus allowing quick iterations for troubleshooting the system. Software emulation only concerns system functionality, not system timing.
```
make xclbin TARGET=sw_emu <makefile options>
make host TARGET=sw_emu <makefile options>
```

Run the system in an emulator on your CPU using:
```
make run TARGET=sw_emu <makefile options>
```

###### 3b. Hardware emulation
(optional) Test the system timing using hardware emulation. It takes quite long to build and run, so I prefer skipping this step and going directly to hardware when you are happy with your system functionality. Only use this step to debug timing issues that you encounter in hardware.
```
make xclbin TARGET=hw_emu <makefile options>
make host TARGET=hw_emu <makefile options>
```

Run the system in an emulator on your CPU using:
```
make run TARGET=hw_emu <makefile options>
```

###### 4. Hardware
When you are happy with your system you can run it on the Versal hardware using the following steps. Keep in mind that building the xclbin for hardware can take a long time (up to 3 hours depending on number of accelerators), so it is recommended to do this in a *GNU screen* or *tmux* session.
```
make xclbin TARGET=hw <makefile options>
make host TARGET=hw <makefile options>
```

Run the system on the Versal hardware using:
```
make run TARGET=hw <makefile options>
```

## Resources
- [Standard-RAxML](https://github.com/stamatak/standard-RAxML) by Alexandros Stamatakis  
    *Alexandros Stamatakis, RAxML version 8: a tool for phylogenetic analysis and post-analysis of large phylogenies, Bioinformatics, Volume 30, Issue 9, May 2014, Pages 1312–1313, [https://doi.org/10.1093/bioinformatics/btu033](https://doi.org/10.1093/bioinformatics/btu033)*
- [AMD documentation UG1393](https://docs.amd.com/r/2022.2-English/ug1393-vitis-application-acceleration/Getting-Started-with-Vitis) - Vitis Unified Software Platform Documentation: Application Acceleration Development
- [AMD documentation UG1079](https://docs.amd.com/r/2022.2-English/ug1079-ai-engine-kernel-coding/Overview?tocId=OerrcATBJkz9SuXKjosb1w) - AI Engine Kernel and Graph Programming Guide
- [AMD documentation UG1076](https://docs.amd.com/r/2022.2-English/ug1076-ai-engine-environment/Overview) - AI Engine Tools and Flows User Guide
- [AMD documentation UG1642](https://docs.amd.com/r/en-US/ug1642-aie-sw-driver-ref/Introduction?tocId=SQMeuOjrwqZWNPhXDT5rPQ) - AI Engine System Software Driver Reference Manual
- [AMD documentation AM009](https://docs.amd.com/r/en-US/am009-versal-ai-engine/Overview) - Versal Adaptive SoC AI Engine Architecture Manual
- [AMD documentation UG1388](https://docs.amd.com/r/en-US/ug1388-acap-system-integration-validation-methodology/Introduction) - Versal Adaptive SoC System Integration and Validation Methodology Guide
- [AMD documentation DS957](https://docs.amd.com/r/en-US/ds957-versal-ai-core/Summary) - Versal AI Core Series Data Sheet: DC and AC Switching Characteristics
- [AMD documentation PG347](https://docs.amd.com/r/en-US/pg347-cpm-dma-bridge/Overview?tocId=oTd_ZrdYcOWw7fqmc3hb9g) - Versal Adaptive SoC CPM DMA and Bridge Mode for PCI Express Product Guide

- [Master thesis](https://essay.utwente.nl/103959/) (change to paper if/when published)

## Roadmap
- [ ] Implement protein-based PLF
- [ ] Create a single input PL kernel and a single output PL kernel that divides the workload site-by-site over a variable amount of instances
- [ ] Improve host-device PCIe data movements to always use all 8 lanes, instead of 2 lanes per accelerator.

## License
This software is licensed under the GNU General Public License version 3.0 (GPL-3) as it is based on the PLF implementation of [RAxML](https://github.com/stamatak/standard-RAxML).

