// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "../../common/utils.h" /* init_parameters, validate_buffer */
#include "../../common/init_buff.h" /* init_buffer */



int main(int argc, char **argv) {

    printf("****start*****\n");

    /* <<--params-->> */

	 const unsigned num_batch_x = 8;


    uint32_t in_words_adj;
    uint32_t out_words_adj;
    uint32_t in_size; //in_size as in_len
    uint32_t out_size;

    // the following four parameters will be re-assigned later.
    uint32_t dma_in_size;
    uint32_t dma_out_size;
    uint32_t dma_size;
    uint32_t out_offset;


    init_parameters(BATCH_SIZE_X, num_batch_x, 
		    NUMK,
		    &in_words_adj, &out_words_adj,
		    &in_size, &out_size,
		    &dma_in_size, &dma_out_size,
		    &out_offset,
		    &dma_size,
		    VALUES_PER_WORD);


    dma_in_size = in_size / VALUES_PER_WORD;
    dma_out_size = out_size / VALUES_PER_WORD;
    dma_size = dma_in_size + dma_out_size;
    

    dma_word_t *mem=(dma_word_t*) malloc(dma_size * sizeof(dma_word_t));

    float *inbuff=(float*) malloc(in_size * sizeof(float));
    float *outbuff=(float*) malloc(out_size * sizeof(float));
    float *outbuff_gold= (float*) malloc(out_size * sizeof(float));

    dma_info_t load;
    dma_info_t store;


    // the running folder of csim is mriq_vivado/hw/hls-work-<board>/mriq_vivado_dma32_w32/mriq_vivado_acc/csim/build

    //    const char *inputFile = "../../../../../data/test_small.bin";
    //    const char *goldFile = "../../../../../data/test_small.out";

    const char *inputFile = "../../../../../data/test_32_x64_k1024.bin";
    const char *goldFile = "../../../../../data/test_32_x64_k1024.out";



    init_buffer(inbuff, outbuff_gold, inputFile, goldFile, 
		BATCH_SIZE_X, num_batch_x, NUMK);

    

    for(unsigned i = 0; i < dma_in_size; i++)
      for(unsigned k = 0; k < VALUES_PER_WORD; k++){
	mem[i].word[k] = (word_t) inbuff[i * VALUES_PER_WORD + k];
      }


    // Call the TOP function
    top(mem, mem,
        /* <<--args-->> */
	num_batch_x,
        load, store); // load, store are pointers


    // Validate
    out_offset = dma_in_size;

    for(unsigned i = 0; i < dma_out_size; i++)
	for(unsigned k = 0; k < VALUES_PER_WORD; k++)
	  {
	    outbuff[i * VALUES_PER_WORD + k] = (float) mem[out_offset + i].word[k];
	  }

    int ER_tooBig;

    ER_tooBig = validate_buffer(outbuff, outbuff_gold, out_size);



    if (ER_tooBig){
      std::cout << "Test FAILED." << std::endl;
    } else {
      std::cout << "Test PASSED." << std::endl;
    }

    // Free memory

    free(mem);
    free(inbuff);
    free(outbuff);
    free(outbuff_gold);

    return 0;
}
