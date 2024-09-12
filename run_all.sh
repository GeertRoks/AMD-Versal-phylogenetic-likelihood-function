#! /bin/bash

mkdir data_hw_runs
mkdir -p data_mem_v4-300

calls=100

make host TARGET=hw PL_FREQ=300 NO_PRERUN_CHECK=1 NO_CORRECTNESS_CHECK=1 NO_INTERMEDIATE_RESULTS=0 -B

make run_hw_tests TARGET=hw PL=uint128x4stream2in AIE=128x4PLFstream2in PLF_CALLS=$calls INSTANCES_USED=1
make run_hw_tests TARGET=hw PL=uint128x4stream2in AIE=128x4PLFstream2in PLF_CALLS=$calls INSTANCES_USED=2
make run_hw_tests TARGET=hw PL=uint128x4stream2in AIE=128x4PLFstream2in PLF_CALLS=$calls INSTANCES_USED=4

make run_hw_tests TARGET=hw PL=uint128x4stream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=1
make run_hw_tests TARGET=hw PL=uint128x4stream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=2
make run_hw_tests TARGET=hw PL=uint128x4stream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=4
make run_hw_tests TARGET=hw PL=uint128x4stream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=8
make run_hw_tests TARGET=hw PL=uint128x4stream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=9

make run_hw_tests TARGET=hw PL=uint128x4window2in AIE=128x4PLFwindow2in PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=1024
make run_hw_tests TARGET=hw PL=uint128x4window2in AIE=128x4PLFwindow2in PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=1024
make run_hw_tests TARGET=hw PL=uint128x4window2in AIE=128x4PLFwindow2in PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=1024

make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=1024
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=1024
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=1024
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=8 WINDOW_SIZE=1024
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=9 WINDOW_SIZE=1024

make run_hw_tests TARGET=hw PL=uint128x4window2in AIE=128x4PLFwindow2in16384 PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=16384
make run_hw_tests TARGET=hw PL=uint128x4window2in AIE=128x4PLFwindow2in16384 PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=16384
make run_hw_tests TARGET=hw PL=uint128x4window2in AIE=128x4PLFwindow2in16384 PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=16384

make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=8192
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=8192
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=8192
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=8 WINDOW_SIZE=8192
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=9 WINDOW_SIZE=8192

make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x4PLFwindow1inEV16288 PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=16288
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x4PLFwindow1inEV16288 PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=16288
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x4PLFwindow1inEV16288 PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=16288

mv data_hw_runs data_mem_v4-300/intermediate

mkdir data_hw_runs

make host TARGET=hw PL_FREQ=300 NO_PRERUN_CHECK=1 NO_CORRECTNESS_CHECK=1 NO_INTERMEDIATE_RESULTS=1 -B

make run_hw_tests TARGET=hw PL=uint128x4stream2in AIE=128x4PLFstream2in PLF_CALLS=$calls INSTANCES_USED=1
make run_hw_tests TARGET=hw PL=uint128x4stream2in AIE=128x4PLFstream2in PLF_CALLS=$calls INSTANCES_USED=2
make run_hw_tests TARGET=hw PL=uint128x4stream2in AIE=128x4PLFstream2in PLF_CALLS=$calls INSTANCES_USED=4

make run_hw_tests TARGET=hw PL=uint128x4stream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=1
make run_hw_tests TARGET=hw PL=uint128x4stream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=2
make run_hw_tests TARGET=hw PL=uint128x4stream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=4
make run_hw_tests TARGET=hw PL=uint128x4stream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=8
make run_hw_tests TARGET=hw PL=uint128x4stream1inEV AIE=128x9PLFstream1inEV PLF_CALLS=$calls INSTANCES_USED=9

make run_hw_tests TARGET=hw PL=uint128x4window2in AIE=128x4PLFwindow2in PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=1024
make run_hw_tests TARGET=hw PL=uint128x4window2in AIE=128x4PLFwindow2in PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=1024
make run_hw_tests TARGET=hw PL=uint128x4window2in AIE=128x4PLFwindow2in PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=1024

make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=1024
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=1024
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=1024
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=8 WINDOW_SIZE=1024
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV PLF_CALLS=$calls INSTANCES_USED=9 WINDOW_SIZE=1024

make run_hw_tests TARGET=hw PL=uint128x4window2in AIE=128x4PLFwindow2in16384 PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=16384
make run_hw_tests TARGET=hw PL=uint128x4window2in AIE=128x4PLFwindow2in16384 PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=16384
make run_hw_tests TARGET=hw PL=uint128x4window2in AIE=128x4PLFwindow2in16384 PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=16384

make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=8192
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=8192
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=8192
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=8 WINDOW_SIZE=8192
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x9PLFwindow1inEV8192 PLF_CALLS=$calls INSTANCES_USED=9 WINDOW_SIZE=8192

make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x4PLFwindow1inEV16288 PLF_CALLS=$calls INSTANCES_USED=1 WINDOW_SIZE=16288
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x4PLFwindow1inEV16288 PLF_CALLS=$calls INSTANCES_USED=2 WINDOW_SIZE=16288
make run_hw_tests TARGET=hw PL=uint128x4window1inEV AIE=128x4PLFwindow1inEV16288 PLF_CALLS=$calls INSTANCES_USED=4 WINDOW_SIZE=16288

mv data_hw_runs data_mem_v4-300/roundtrip
