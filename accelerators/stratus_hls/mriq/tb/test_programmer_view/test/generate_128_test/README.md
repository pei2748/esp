#### gen128.c:
  used to generate test data for 128*128*128 dataset. The original numX is 2048*1024 and numK is also 2048*1024. In order to test on fpga for A1, we need to make numK as multiple of 1024 since batch_size_k is fixed to 1024. So numK could be 4K, 8K, 16K. Then numX could be any number, here I set is as 4.
  
* input files:
   * ../128x128x128.bin
* output file:
    * test_128_x4_k**K.bin (** could be 4, 8, 16, 32, this should be manually modifed in gen128.c code)
    * test_128_x4_k**K.out (** could be same as the above)
#### Makefile
run "make" to generate an executable.

#### Other files:
* file0.cc
    contains some functions used to read data from files and do computation.
* file0.h 
    header file of file0.cc, which is included in the Makefile.