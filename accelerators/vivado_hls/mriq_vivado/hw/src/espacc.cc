// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>
#include "espacc_utils.h" // dma_read and dma_write

void init_parameters(dma_length_load_t &dma_length_load,
		     dma_length_store_t &dma_length_store,
		     unsigned &store_offset,
		     const unsigned num_batch_x)

{
    // compute store_offset and dma_length for each loading transaction
    
    const unsigned dma_length_x = round_up(BATCH_SIZE_X, VALUES_PER_WORD) >> (VALUES_PER_WORD - 1);
    const unsigned dma_length_k = round_up(NUMK, VALUES_PER_WORD) >> (VALUES_PER_WORD - 1);
    store_offset = round_up(3*BATCH_SIZE_X*num_batch_x + 5*NUMK, VALUES_PER_WORD) >> (VALUES_PER_WORD - 1);  


    // configure dma length for load process                                                                                                             
    dma_length_load.x = dma_length_x;
    dma_length_load.k = dma_length_k;

    // configure dma length for store process                                                                                                            
    dma_length_store.Qr = dma_length_x;
    dma_length_store.Qi = dma_length_x;
}



void load(word_t _inbuff_x[BATCH_SIZE_X],
	  word_t _inbuff_y[BATCH_SIZE_X],
	  word_t _inbuff_z[BATCH_SIZE_X],
	  word_t _inbuff_kx[NUMK],
	  word_t _inbuff_ky[NUMK],
	  word_t _inbuff_kz[NUMK],
	  word_t _inbuff_phiR[NUMK],
	  word_t _inbuff_phiI[NUMK],
	  dma_word_t *in1,
          /* <<--compute-params-->> */
	  dma_length_load_t dma_length_load,
	  dma_info_t &load_ctrl, 
	  int chunk)
{
    // configure dma index for load process

    unsigned dma_length;
    unsigned dma_index;

    if (chunk == 0) {
      dma_index = 0;
      dma_length = dma_length_load.k * 5 + dma_length_load.x * 3;
    } else if (chunk == 1) {
      dma_index = 0;
      dma_length = dma_length_load.k * 5 + dma_length_load.x * 6;
    } else {
      dma_index = dma_length_load.k * 5 + 3 * dma_length_load.x * chunk;
      dma_length = dma_length_load.x * 3;
    }

    load_ctrl.index = dma_index;
    load_ctrl.length = dma_length;
    load_ctrl.size = SIZE_WORD_T;

    // only to 5 k-variables only in the 1st and 2nd iteration

    if (chunk == 0 || chunk == 1) {

	dma_read(_inbuff_kx, dma_index, dma_length_load.k, in1);
	dma_index += dma_length_load.k;
  
	dma_read(_inbuff_ky, dma_index, dma_length_load.k, in1);
	dma_index += dma_length_load.k;

	dma_read(_inbuff_kz, dma_index, dma_length_load.k, in1);
	dma_index += dma_length_load.k;

	dma_read(_inbuff_phiR, dma_index, dma_length_load.k, in1);
	dma_index += dma_length_load.k;

	dma_read(_inbuff_phiI, dma_index, dma_length_load.k, in1);
	dma_index += dma_length_load.k;
    }

    // load into inbuff_x, inbuff_y, inbuff_z for every chunk

    dma_read(_inbuff_x, dma_index, dma_length_load.x, in1);
    dma_index += dma_length_load.x;

    dma_read(_inbuff_y, dma_index, dma_length_load.x, in1);
    dma_index += dma_length_load.x;

    dma_read(_inbuff_z, dma_index, dma_length_load.x, in1);

    if (chunk == 1) {
      // overwrite inbuff_x, y, z with new data
      dma_index += dma_length_load.x;

      dma_read(_inbuff_x, dma_index, dma_length_load.x, in1);
      dma_index += dma_length_load.x;

      dma_read(_inbuff_y, dma_index, dma_length_load.x, in1);
      dma_index += dma_length_load.x;

      dma_read(_inbuff_z, dma_index, dma_length_load.x, in1);

    }
}


void store(word_t _outbuff_Qr[BATCH_SIZE_X], word_t _outbuff_Qi[BATCH_SIZE_X],
	   dma_word_t *out,
	   /* <<--compute-params-->> */
	  dma_length_store_t dma_length_store,
	   unsigned store_offset,
	   dma_info_t &store_ctrl, 
	   int chunk)
{

    // configure dma index for store process
    dma_index_store_t dma_index_store;

    unsigned dma_length;
    dma_length = dma_length_store.Qr + dma_length_store.Qi;

    dma_index_store.Qr = store_offset + chunk * dma_length;
    dma_index_store.Qi = dma_index_store.Qr + dma_length_store.Qr;


 store_data:
    // configure store_ctrl
    store_ctrl.index = dma_index_store.Qr;
    store_ctrl.length = dma_length;
    store_ctrl.size = SIZE_WORD_T;

    dma_write(_outbuff_Qr, dma_index_store.Qr, dma_length_store.Qr, out);
    dma_write(_outbuff_Qi, dma_index_store.Qi, dma_length_store.Qi, out);

}



void compute(word_t _inbuff_x[BATCH_SIZE_X],
	     word_t _inbuff_y[BATCH_SIZE_X],
	     word_t _inbuff_z[BATCH_SIZE_X],
	     word_t _inbuff_kx[NUMK],
	     word_t _inbuff_ky[NUMK],
	     word_t _inbuff_kz[NUMK],
	     word_t _inbuff_phiR[NUMK],
	     word_t _inbuff_phiI[NUMK],
             /* <<--compute-params-->> */
             word_t _outbuff_Qr[BATCH_SIZE_X],
             word_t _outbuff_Qi[BATCH_SIZE_X])
{
  // TODO implement compute functionality
  #include "espacc_func.h"
}




void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_num_batch_x,
	 dma_info_t &load_ctrl, 
	 dma_info_t &store_ctrl)
{

  /* <<--local-params-->> */

  const unsigned num_batch_x = conf_info_num_batch_x;


  dma_length_load_t dma_length_load;
  dma_length_store_t dma_length_store;

#pragma HLS data_pack variable=dma_length_load
#pragma HLS data_pack variable=dma_length_store

  unsigned store_offset;

  init_parameters(dma_length_load, dma_length_store, store_offset, 
		  num_batch_x);
  // Chunking

  word_t _inbuff_kx[NUMK];
  word_t _inbuff_ky[NUMK];
  word_t _inbuff_kz[NUMK];
  word_t _inbuff_phiR[NUMK];
  word_t _inbuff_phiI[NUMK];

#pragma HLS array_partition variable=_inbuff_kx block factor=4
#pragma HLS array_partition variable=_inbuff_ky block factor=4
#pragma HLS array_partition variable=_inbuff_kz block factor=4
#pragma HLS array_partition variable=_inbuff_phiR block factor=4
#pragma HLS array_partition variable=_inbuff_phiI block factor=4

 go:
  for (int c = 0; c < num_batch_x; c++)
    {

	  word_t _inbuff_x[BATCH_SIZE_X];
	  word_t _inbuff_y[BATCH_SIZE_X];
	  word_t _inbuff_z[BATCH_SIZE_X];	   
	  word_t _outbuff_Qr[BATCH_SIZE_X];
	  word_t _outbuff_Qi[BATCH_SIZE_X];


	  load(_inbuff_x,_inbuff_y,_inbuff_z,
	       _inbuff_kx,_inbuff_ky, _inbuff_kz, _inbuff_phiR, _inbuff_phiI, 
	       in1,
	       /* <<--args-->> */
	       dma_length_load,
	       load_ctrl, c);

	  compute(_inbuff_x,_inbuff_y,_inbuff_z,
		  _inbuff_kx,_inbuff_ky, _inbuff_kz, _inbuff_phiR, _inbuff_phiI, 
		  /* <<--args-->> */
		  _outbuff_Qr, _outbuff_Qi);

	  store(_outbuff_Qr, _outbuff_Qi, out,
		/* <<--args-->> */
		dma_length_store,
		store_offset,
		store_ctrl, c);

    }

}


