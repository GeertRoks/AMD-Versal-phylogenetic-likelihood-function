#! /bin/bash

echo "Have you commented the lines in the xrt.ini file?"
read -p "Enter 'y' for yes or 'n' for no: " choice

case "$choice" in
    [Yy]* ) echo "Good we can continue then";;
    [Nn]* ) echo "Please do so, if there is configuration in the xrt.ini, then the gen-pl test will not work"; exit 1;;
    * ) echo "Invalid input. Please enter 'y' or 'n'."; exit 1;
esac

calls=200

mkdir data_hw_pl_generator_runs

make host_gen TARGET=hw NO_PRERUN_CHECK=1 PL_FREQ=250 -B

make run_hw_gen_tests TARGET=hw PL=genstream2in AIE=128x4PLFstream2in PLF_CALLS=$calls INSTANCES_USED=1
make run_hw_gen_tests TARGET=hw PL=genstream2in AIE=128x4PLFstream2in PLF_CALLS=$calls INSTANCES_USED=2
make run_hw_gen_tests TARGET=hw PL=genstream2in AIE=128x4PLFstream2in PLF_CALLS=$calls INSTANCES_USED=4

make run_hw_gen_tests TARGET=hw PL=genstream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=1
make run_hw_gen_tests TARGET=hw PL=genstream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=2
make run_hw_gen_tests TARGET=hw PL=genstream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=4
make run_hw_gen_tests TARGET=hw PL=genstream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=8
make run_hw_gen_tests TARGET=hw PL=genstream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=9

make run_hw_gen_tests TARGET=hw PL=genwindow2in AIE=128x4PLFwindow2in PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=1024
make run_hw_gen_tests TARGET=hw PL=genwindow2in AIE=128x4PLFwindow2in PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=1024
make run_hw_gen_tests TARGET=hw PL=genwindow2in AIE=128x4PLFwindow2in PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=1024

make run_hw_gen_tests TARGET=hw PL=genwindow1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=1024
make run_hw_gen_tests TARGET=hw PL=genwindow1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=1024
make run_hw_gen_tests TARGET=hw PL=genwindow1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=1024
make run_hw_gen_tests TARGET=hw PL=genwindow1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=8 WINDOW_SIZE=1024
make run_hw_gen_tests TARGET=hw PL=genwindow1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=9 WINDOW_SIZE=1024

make run_hw_gen_tests TARGET=hw PL=genwindow2in AIE=128x4PLFwindow2in16384 PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=16384
make run_hw_gen_tests TARGET=hw PL=genwindow2in AIE=128x4PLFwindow2in16384 PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=16384
make run_hw_gen_tests TARGET=hw PL=genwindow2in AIE=128x4PLFwindow2in16384 PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=16384

make run_hw_gen_tests TARGET=hw PL=genwindow1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=8192
make run_hw_gen_tests TARGET=hw PL=genwindow1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=8192
make run_hw_gen_tests TARGET=hw PL=genwindow1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=8192
make run_hw_gen_tests TARGET=hw PL=genwindow1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=8 WINDOW_SIZE=8192
make run_hw_gen_tests TARGET=hw PL=genwindow1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=9 WINDOW_SIZE=8192

make run_hw_gen_tests TARGET=hw PL=genwindow1inEV AIE=128x4PLFwindow1inEV16288 PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=16288
make run_hw_gen_tests TARGET=hw PL=genwindow1inEV AIE=128x4PLFwindow1inEV16288 PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=16288
make run_hw_gen_tests TARGET=hw PL=genwindow1inEV AIE=128x4PLFwindow1inEV16288 PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=16288

mv data_hw_pl_generator_runs data_gen_v4-300

echo "remember to uncomment the xrt.ini file again afterwards"
