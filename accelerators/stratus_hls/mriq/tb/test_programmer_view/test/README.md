#### test_128_data.c:
  used to verify that if newly generated 128x128x128.bin file has non-zero phiR and phiI, and the newly generated 128x128x128.out has sound Qr-Qi pairs. This program reads out some number of phiR and phiI, which is printed to console. It also writes some Qr-Qi pairs to 128.txt file.
* input files:
   * 128x128x128.bin
   * 128x128x128.out
* output file:
    * 128.txt
#### Makefile
run "make" to generate an executable.