# Phylogenetic Likelihood Function (PLF) Accelerator for Versal Adaptive SoCs

Tested for VC1902 on VCK5000 datacenter card
Build using v++ 2022.2 and XRT library 2023.2

### Project Structure

- aie\_PLF/
    - contains AI Engine kernels.
    - Has two versions PLFv1 and PLFv2. Always use PLFv2. (TODO: remove versions and only use PLFv2)
    - Each version has multiple setups where,
        - __128x1__ refers to 128-bit PLIO in-out streams
        - __xDxBxE__, where x is either s or w and refers to whether a stream (s) or window (w) is used for the Alignment Data (D), Branch matrices (B) and Eigen Value Matrix (E)
        - __pipelined__ refers to whether the kernel has chess\_prepare\_for\_pipelining in the for loop

- hls\_PLF\_datamovers/
    - contains HLS kernels for the movement of data between memory and the AIE array
    - has multiple setups where,
        - __mm2sleft__ is the data mover from memory (mm) to (2) stream of the AIE array (s) for the left side of the PLF calculation
        - __mm2sright__ is the data mover from memory (mm) to (2) stream of the AIE array (s) for the right side of the PLF calculation
        - __s2mm__ is the datamover from stream of the AIE array (s) to (2) memory (mm) for the result of the PLF calculation
        - __uint128x4__ means that the kernel reads 512-bit from memory and splits this over 4 128-bit streams
        - __window__ means that HLS kernel is meant for use with an AIE array that uses windows, because this kernel pads the alignment data with zeroes if the data does not fit neatly in a window.

- app\_PLF/
    - contains the code for the program that calls the PLF accelerator
    - In the code one can change the amount of alignment sites and how many times the PLF calculation is repeated, using the `tb.alignment_sites` and `tb.plf_calls` variables respectively. The `tb.window_size` variable should match to whatever the AIE graph has for its data window size.


### Usage
The accelerator can be compiled using the Makefile. There are some flags to modify the compilation

`TARGET`: select which compilation target is used: `sw_emu` or `hw`
`VERSION`: select the version (choose PLFv2) (TODO: remove the versions)
`AIE`: select the AIE setup. Use the same name as the directory that you want to use
`PL`: select the Programable Logic data movers setup. The datamovers come in sets and use the overarching name, so the name behind the kernel type. For example: if you want to use `mm2sleft\_uint128x4\_window.cpp` then write `uint128x4\_window` for the `PL` flag and all hls kernels of this set are chosen (mm2sleft, mm2sright and s2mm)
`NO\_PRERUN\_CHECKS`: if set to `1`, then no confirmation is asked before the execution of the accelerator
`PL_FREQ`: set the frequency of the PL in MHz

**NOTE: make sure the PLATFORM variable in the Makefile points to the correct .xpfm platform file and that the XILINX_XRT environment variable is set.**

#### Building

There are multiple targets that can be build
- `aie`: only builds the needed AIE graph
- `hls`: only builds the needed HLS kernels
- `xsa`: links the AIE graph and HLS kernels, if either of these still need to be build, then it will build them as well
- `xclbin`: packages the AIE graph and HLS kernels for use on VCK5000 board, if either stage (building or linking) still needs to be done, then it will do those first
- `host`:  build the host program that controls the accelerator

All build artifacts are stored in the `build/` directory

##### Software emulation
Build the XCLBIN for the accelerator
```
make xclbin TARGET=sw_emu VERSION=PLFv2 AIE=128x1PLF_wDwBwE PL=uint128x4_window
```

Build the program that controlls the accelerator
```
make host TARGET=sw_emu VERSION=PLFv2 AIE=128x1PLF_wDwBwE PL=uint128x4_window
```

##### Hardware
Build the XCLBIN for the accelerator
```
make xclbin TARGET=hw VERSION=PLFv2 AIE=128x1PLF_wDwBwE PL=uint128x4_window
```

Build the program that controlls the accelerator
```
make host TARGET=hw VERSION=PLFv2 AIE=128x1PLF_wDwBwE PL=uint128x4_window
```

#### Simulation
The AIE graph can be tested individually

##### AIE functional simulation
```
make aie_x86sim VERSION=PLFv2 AIE=128x1PLF_wDwBwE
```

##### AIE performance simulation
```
make aie_sim VERSION=PLFv2 AIE=128x1PLF_wDwBwE
```

#### Running

##### Software emulation
```
make run_sw_emu TARGET=sw_emu VERSION=PLFv2 AIE=128x1PLF_wDwBwE PL=uint128x4_window
```

##### Hardware
```
make run_hw TARGET=hw VERSION=PLFv2 AIE=128x1PLF_wDwBwE PL=uint128x4_window
```


Currently is `AIE=128x1PLF_wDwBwE` with `PL=uint128x4_window` the best performing setup with about 200MB/s for 512 alignments
