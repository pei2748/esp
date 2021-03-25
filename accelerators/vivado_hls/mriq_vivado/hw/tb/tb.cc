// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <fstream> /* file stream */
#include "helper.h" /* two functions used by tb.cc */



int main(int argc, char **argv) {

    printf("****start*****\n");

    /* <<--params-->> */
	 const unsigned numX = 8;
	 const unsigned numK = 16;

	 const unsigned num_batch_x = 4;
	 const unsigned batch_size_x = 2;

    uint32_t inX_words_adj;
    uint32_t inK_words_adj;

    uint32_t out_words_adj;

    uint32_t inX_size;
    uint32_t inK_size;
    uint32_t out_size;

    uint32_t dma_inX_size;
    uint32_t dma_inK_size;

    uint32_t dma_in_size;

    uint32_t dma_out_size;
    uint32_t dma_size;



    inX_words_adj = round_up(3*numX, VALUES_PER_WORD);
    inX_size = inX_words_adj * (1);
    dma_inX_size = inX_size / VALUES_PER_WORD;

    inK_words_adj = round_up(5*numK, VALUES_PER_WORD);
    inK_size = inK_words_adj * (1);
    dma_inK_size = inK_size / VALUES_PER_WORD;


    out_words_adj = round_up(2*numX, VALUES_PER_WORD);
    out_size = out_words_adj * (1);
    dma_out_size = out_size / VALUES_PER_WORD;


    dma_size = dma_inX_size + dma_inK_size + dma_out_size;

    dma_word_t *mem=(dma_word_t*) malloc(dma_size * sizeof(dma_word_t));

    word_t *inbuff_x=(word_t*) malloc(inX_size * sizeof(word_t));
    word_t *inbuff_k=(word_t*) malloc(inK_size * sizeof(word_t));

    word_t *outbuff=(word_t*) malloc(out_size * sizeof(word_t));
    word_t *outbuff_gold= (word_t*) malloc(out_size * sizeof(word_t));

    // suppose we have 8 transaction, kx~phiI, x~z
    //    dma_info_t *load  = (dma_info_t*) malloc(8 * sizeof(dma_info_t));
    //dma_info_t *store = (dma_info_t*) malloc(2 * sizeof(dma_info_t));


    dma_info_t load;
    dma_info_t store;




    // Prepare input data
#if(0)
    std::string inputName =  "/home/esp2020/pl2748/new-esp/esp/accelerators/vivado_hls/mriq_vivado/hw/tb/test_small.bin";
    std::string goldName =   "/home/esp2020/pl2748/new-esp/esp/accelerators/vivado_hls/mriq_vivado/hw/tb/test_small.out";
    std::string data4barec = "/home/esp2020/pl2748/new-esp/esp/accelerators/vivado_hls/mriq_vivado/hw/tb/test_barec.h";
#endif

    std::string inputName =  "/home/esp2020/pl2748/new-esp/esp/accelerators/vivado_hls/mriq_vivado/hw/tb/test_32_x8_k16.bin";
    std::string goldName =   "/home/esp2020/pl2748/new-esp/esp/accelerators/vivado_hls/mriq_vivado/hw/tb/test_32_x8_k16.out";
    std::string data4barec = "/home/esp2020/pl2748/new-esp/esp/accelerators/vivado_hls/mriq_vivado/hw/tb/test_barec.h";

    char inputName_c[inputName.size() + 1];
    std::copy(inputName.begin(), inputName.end(), inputName_c);
    inputName_c[inputName.size()] = '\0';

    char goldName_c[goldName.size() + 1];
    std::copy(goldName.begin(), goldName.end(), goldName_c);
    goldName_c[goldName.size()] = '\0';

    std::ifstream fin;
    fin.open(inputName);
    std::ifstream fpr;
    fpr.open(goldName);
    std::ofstream fbc;
    fbc.open(data4barec);

    if(!fin.is_open() || !fpr.is_open() || !fbc.is_open()){
      fprintf(stderr, "cannot open file... FAIL!\n");
      exit(1);
    }


    float *kx, *ky, *kz;          /* K trajectory (3D vectors) */
    float *x, *y, *z;             /* X coordinates (3D vectors) */
    float *phiR, *phiI;           /* Phi values (complex) */
    float *goldQr, *goldQi;
    int numX_bm, numK_bm; /* should be the same value as numX and numK */

    inputData(inputName_c, &numK_bm, &numX_bm,
              &kx, &ky, &kz,
              &x,  &y,  &z,
              &phiR, &phiI);


    goldQr = (float *) malloc(sizeof(float) * numX);
    goldQi = (float *) malloc(sizeof(float) * numX);
    outputData(goldName_c, &goldQr, &goldQi, &numX_bm);
    unsigned i;

    for(i = 0; i < numK; i++) {
      inbuff_k[i] = (word_t) kx[i];
      inbuff_k[i + numK] = (word_t) ky[i];
      inbuff_k[i + 2 * numK] = (word_t) kz[i];
      inbuff_k[i + 3 * numK] = (word_t) phiR[i];
      inbuff_k[i + 4 * numK] = (word_t) phiI[i];
    }



    int idx_inbuff_x = 0;
    
    for (i = 0; i < num_batch_x; i++) {

      for (unsigned j = 0; j < batch_size_x; j++) {
	int idx = i * batch_size_x + j;
        inbuff_x[idx_inbuff_x++] = (word_t) x[idx];
      }

      for (unsigned j = 0; j < batch_size_x; j++) {
	int idx = i * batch_size_x + j;
        inbuff_x[idx_inbuff_x++] = (word_t) y[idx];
      }

      for (unsigned j = 0; j < batch_size_x; j++) {
	int idx = i * batch_size_x + j;
        inbuff_x[idx_inbuff_x++] = (word_t) z[idx];
      }

    }



    for(unsigned i = 0; i < dma_inK_size; i++)
      for(unsigned k = 0; k < VALUES_PER_WORD; k++){
	mem[i].word[k] = inbuff_k[i * VALUES_PER_WORD + k];
      	fbc << "in[" << i * VALUES_PER_WORD + k << "] = " <<mem[i].word[k] << ";\n";
      }
    dma_in_size = dma_inK_size + dma_inX_size;
    // send x, y, z only once
    for(unsigned i = dma_inK_size; i < dma_in_size; i++)
      for(unsigned k = 0; k < VALUES_PER_WORD; k++){
	mem[i].word[k] = inbuff_x[(i - dma_inK_size) * VALUES_PER_WORD + k];
       	fbc << "in[" << i * VALUES_PER_WORD + k << "] = " <<mem[i].word[k] << ";\n";
      }




    // Set golden output                                                                                                                   
    unsigned offset = 0;
    for (unsigned i = 0; i < num_batch_x; i++) {
    	for(unsigned j = 0; j < batch_size_x; j++){
		outbuff_gold[offset + j] = (word_t) goldQr[i * batch_size_x + j];
		fbc << "gold[" << offset + j << "] = " << outbuff_gold[offset + j] << ";\n";
	}
	offset += batch_size_x;

	for(unsigned j=0; j < batch_size_x; j++){
		outbuff_gold[offset + j] = (word_t) goldQi[i * batch_size_x + j];
		fbc << "gold[" << offset + j << "] = " << outbuff_gold[offset + j] << ";\n";
	}
	offset += batch_size_x;
    }



    // Call the TOP function
    top(mem, mem,
        /* <<--args-->> */
	 	 numX,
	 	 numK,
	num_batch_x,
	batch_size_x,
        load, store); // load, store are pointers

    // Validate
    uint32_t out_offset = dma_in_size;

    for(unsigned i = 0; i < dma_out_size; i++)
	for(unsigned k = 0; k < VALUES_PER_WORD; k++)
	  {
	    outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];
	    //	    std::cout << "out[" << i * VALUES_PER_WORD + k << "] = " << outbuff[i * VALUES_PER_WORD + k] << std::endl;
	  }
    int errors = 0;
    float error_th = 0.05;
    float error_rate_total;
    float error_rate_th = 0.01;
    float error_rate;

 
    for(unsigned j = 0; j < 2*numX; j++) {
	  error_rate = abs(((float)(outbuff[j]) - (float)(outbuff_gold[j]))/(float)(outbuff_gold[j]));
	  if (error_rate > error_th)
	      errors++;

    }


    error_rate_total = errors/(2 * numX);

    if (error_rate_total > error_rate_th){
      std::cout << "Test FAILED with " << errors << " errors.\n" << std::endl;
      std::cout << "error rate is " << error_rate_total << "\n" << std::endl;
    } else {
      std::cout << "Test PASSED." << std::endl;
    }

    // Free memory

    free(mem);

    free(inbuff_x);
    free(inbuff_k);
    free(outbuff);
    free(outbuff_gold);

    free(kx);
    free(ky);
    free(kz);
    free(x);
    free(y);
    free(z);
    free(phiR);
    free(phiI);
    free(goldQr);
    free(goldQi);

    fin.close();
    fpr.close();
    fbc.close();
    return 0;
}
