// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], dma_word_t *in1, unsigned chunk,
	  dma_info_t &load_ctrl, int base_index)
{


    unsigned base = SIZE_IN_CHUNK * chunk;

    load_ctrl.index = base;
    load_ctrl.length = SIZE_IN_CHUNK;
    load_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < SIZE_IN_CHUNK; i++) {
    	for(unsigned j = 0; j < VALUES_PER_WORD; j++) {

            #pragma HLS unroll complete

	    _inbuff[i * VALUES_PER_WORD + j] = in1[base + i].word[j];
    	}
    }
}

void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out, unsigned chunk,
	   dma_info_t &store_ctrl, int base_index)
{


    unsigned base = SIZE_OUT_CHUNK * chunk;
    
    store_ctrl.index = base + base_index;
    store_ctrl.length = SIZE_OUT_CHUNK;
    store_ctrl.size = SIZE_WORD_T;

    
    for (unsigned i = 0; i < SIZE_OUT_CHUNK; i++) {
	for(unsigned j = 0; j < VALUES_PER_WORD; j++) {

	  #pragma HLS unroll complete

	  out[base + i].word[j] = _outbuff[i * VALUES_PER_WORD + j];
	}
    }
}

void compute(word_t _inbuff[SIZE_IN_CHUNK_DATA], word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{
#pragma HLS inline off

    for (int i = 0; i < SIZE_OUT_CHUNK_DATA; i++)
        _outbuff[i] = _inbuff[i*2] + _inbuff[i*2+1];
}

void top(dma_word_t *out, dma_word_t *in1, const unsigned conf_info_nbursts,
	 dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

go:
    for (unsigned i = 0; i < conf_info_nbursts; i++)
    {
	  //      #pragma HLS pipeline II=32
      #pragma HLS dataflow

	word_t _inbuff[SIZE_IN_CHUNK_DATA];
	word_t _outbuff[SIZE_OUT_CHUNK_DATA];
	int base_index_store = conf_info_nbursts * SIZE_IN_CHUNK;

        load(_inbuff, in1, i, load_ctrl, 0);
        compute(_inbuff, _outbuff);
        store( _outbuff, out, i, store_ctrl, base_index_store);
    }
}
