// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], dma_word_t *in1,
          /* <<--compute-params-->> */
	 const unsigned numX,
	 const unsigned numK,
	  dma_info_t &load_ctrl, int chunk, int batch)
{
load_data:

    const unsigned length = round_up(5*numK+3*numX, VALUES_PER_WORD) / 1;
    const unsigned index = length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    load_ctrl.index = dma_index;
    load_ctrl.length = dma_length;
    load_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
    load_label0:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
	    _inbuff[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
    	}
    }
}

void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out,
          /* <<--compute-params-->> */
	 const unsigned numX,
	 const unsigned numK,
	   dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(2*numX, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(5*numK+3*numX, VALUES_PER_WORD) * 1;
    const unsigned out_offset = store_offset;
    const unsigned index = out_offset + length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;

    store_ctrl.index = dma_index;
    store_ctrl.length = dma_length;
    store_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length; i++) {
    store_label1:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
	    out[dma_index + i].word[j] = _outbuff[i * VALUES_PER_WORD + j];
	}
    }
}


void compute(word_t _inbuff[SIZE_IN_CHUNK_DATA],
             /* <<--compute-params-->> */
	 const unsigned numX,
	 const unsigned numK,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{

    // TODO implement compute functionality
 compute_data:
  const word_t PI2 = 6.2831853071796;
  const word_t HALF_PI = ((word_t) (0.25)) * PI2;
  word_t x, y, z;
  word_t kx, ky, kz, phiR, phiI;
  word_t expArg, phiMag;
  word_t cosArg, sinArg;
  word_t Qracc, Qiacc;
  unsigned indexX;
  const unsigned NUMK_BATCH = 8;
  const unsigned ITER_K = 2; // numK/8

 compute_label_x:for(indexX = 0; indexX < numX; indexX++)
    {
      x = _inbuff[5*numK + indexX];
      y = _inbuff[5*numK + numX + indexX];
      z = _inbuff[5*numK + 2*numX + indexX];
      Qracc = 0;
      Qiacc = 0;
    compute_label_k:for(unsigned t = 0; t < ITER_K; t++)
        {
	compute_label_inK:for(unsigned indexK = 0; indexK < NUMK_BATCH; indexK++)
	    {
#pragma HLS UNROLL factor=4

	      int idx = t * NUMK_BATCH + indexK;
	      kx = _inbuff[idx];
	      ky = _inbuff[numK + idx];
	      kz = _inbuff[numK * 2 + idx];
	      phiR = _inbuff[numK * 3 + idx];
	      phiI = _inbuff[numK * 4 + idx];

	      phiMag = phiR * phiR + phiI * phiI;
	      expArg = PI2 * (kx * x + ky * y + kz * z);
	      cosArg = hls::cos(expArg);
	      sinArg = hls::sin(expArg);
	      Qracc += phiMag * cosArg;
	      Qiacc += phiMag * sinArg;
	    }       // end of inner k
        } // end of outer k
      _outbuff[indexX ] = Qracc;
      _outbuff[indexX  + numX] = Qiacc;
      //   std::cout <<  indexX <<"-th finished!\n" << std::endl;
    } // end of x
}


void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_numX,
	 const unsigned conf_info_numK,
	 dma_info_t &load_ctrl, dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
	 const unsigned numX = conf_info_numX;
	 const unsigned numK = conf_info_numK;

    // Batching
batching:
    for (unsigned b = 0; b < 1; b++)
    {
        // Chunking
    go:
        for (int c = 0; c < 1; c++)
        {
            word_t _inbuff[SIZE_IN_CHUNK_DATA];
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];

            load(_inbuff, in1,
                 /* <<--args-->> */
	 	 numX,
	 	 numK,
                 load_ctrl, c, b);
            compute(_inbuff,
                    /* <<--args-->> */
	 	 numX,
	 	 numK,
                    _outbuff);
            store(_outbuff, out,
                  /* <<--args-->> */
	 	 numX,
	 	 numK,
                  store_ctrl, c, b);
        }
    }
}
