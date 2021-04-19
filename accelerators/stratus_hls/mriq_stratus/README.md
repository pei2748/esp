### Directory introduction:
hw/

sw/

common/


|── helper.h 
>Functions reading data from input file and golden output file. It is included by init_buff.h file

|── init_buff.h 
>Generate input data and golden output data. It is used in hw/tb/system.cpp and sw/linux/app/mriq.c. 

|── sw_exec.h
>It is only used in sw/linux/app/mriq.c. sw_exec() measures executing time of software computation. 

|── utils.h 
>Has two functions: init_parameters() and validate_buffer(). They are used by hw/tb/system.cpp, sw/baremetal/mriq.c, sw/linux/app/mriq.c

###  Testing Instructions:

We can test the correctness of this accelerator through the testbench.  The following two datafiles are used to test and debug RTL, which work with the testbench. The file path and name are specified in hls/project.tcl. The configuration parameters related to the testing datafiles is [batch_size_x, num_batch_x, batch_size_k, num_batch_k] = [4, 1, 16, 1]. If you want to use other paramters, you can specify the them in tb/system.hpp file and then generate the corresponding testing data with hw/data4bm/genData program. 

>hw/input/test_small.bin
>hw/output/test_small.out

We can also test through baremetal application and linux application. One sample test data for baremetal application is the following. We can generate our test data with the two programs under hw/data4bm/ folder. The instructions of how to generate test data are in the README file of hw/data4bm/. We need to specify the corresponding configuration parameters in sw/baremetal/mriq.c file first and then use these parameters to generate the corresponding testing data.

>hw/data4bm/test_32_x4_k16_bm.h

There are three arguments of linux app: name-of-input-file, name-of-golden-outputfile, and the answer to "do you want to run software program of this accelerator?". For example:
>  ./mriq_stratus.exe test_small.bin test_small.out 0

The 3rd argment tells the linux app whether you want to run the software code of computation and measure its execution time on FPGA. "0" means "No" and "1" is "Yes".

	
