// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>

#define PRL 4

// to get latency info for synthesis
#define NUMX 4
#define NUMK 16 

void load(word_t _inbuff[SIZE_IN_CHUNK_DATA], // store x,y,z
          word_t _inbuff_k[SIZE_IN_K_DATA],
	  dma_word_t *in1,
          /* <<--compute-params-->> */
	 const unsigned numX,
	 const unsigned numK,
	  dma_info_t &load_ctrl, int chunk, int batch,
	  bool *load_k)
{
load_data:

	unsigned dma_length;
	unsigned dma_index;
	unsigned buff_idx_base;
	    
	dma_length = round_up(numK, VALUES_PER_WORD) >> (VALUES_PER_WORD - 1);// must be an even number

	for(unsigned n=0; n<5; n++){
	  // every seperate memory needs to be configured once.
    	        dma_index =  n * dma_length;
		load_ctrl.index = dma_index;
		load_ctrl.length = dma_length;
		load_ctrl.size = SIZE_WORD_T;

		for (unsigned i = 0; i < dma_length; i++) {		  
			buff_idx_base = n * numK + i * VALUES_PER_WORD;
			load_label0:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
		    		_inbuff_k[buff_idx_base + j] = in1[dma_index + i].word[j];
			}		  
		}
	}//end-of-loading-k

	dma_length = round_up(numX, VALUES_PER_WORD) >> (VALUES_PER_WORD - 1);// must be an even number

	for(unsigned m=0; m<3; m++){
	  // every seperate memory needs to be configured once.
		dma_index =  round_up(5 * numK, VALUES_PER_WORD) >> (VALUES_PER_WORD - 1) + m * dma_length;
		load_ctrl.index = dma_index;
		load_ctrl.length = dma_length;
		load_ctrl.size = SIZE_WORD_T;

		for (unsigned i = 0; i < dma_length; i++) {		  
			buff_idx_base = m * numX + i * VALUES_PER_WORD;
			load_label1:for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
		    		_inbuff[buff_idx_base + j] = in1[dma_index + i].word[j];
			}		  
		}
	}//end-of-loading-x
}


void store(word_t _outbuff[SIZE_OUT_CHUNK_DATA], dma_word_t *out,
          /* <<--compute-params-->> */
	 const unsigned numX,
	 const unsigned numK,
	   dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(2*numX, VALUES_PER_WORD) / 1;
    const unsigned store_offset = round_up(3*numX + 5*numK, VALUES_PER_WORD) * 1;
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
             word_t _inbuff_k[SIZE_IN_K_DATA],
             /* <<--compute-params-->> */
	     const unsigned numX,
	     const unsigned numK,
             word_t _outbuff[SIZE_OUT_CHUNK_DATA])
{
  // TODO implement compute functionality
 compute_data:
  //  const unsigned paral = 4;
  const word_t PI2 = 6.2831853071796;
  const word_t HALF_PI = ((word_t) (0.25)) * PI2;


  word_t x, y, z;

  word_t kx[PRL], ky[PRL], kz[PRL], phiR[PRL], phiI[PRL];
  word_t expArg[PRL], phiMag[PRL];
  word_t cosArg[PRL], sinArg[PRL];

  word_t Qracc_p[PRL], Qiacc_p[PRL];
  word_t Qracc, Qiacc;

#pragma HLS array_partition variable=kx complete
#pragma HLS array_partition variable=ky complete
#pragma HLS array_partition variable=kz complete
#pragma HLS array_partition variable=phiR complete
#pragma HLS array_partition variable=phiI complete
#pragma HLS array_partition variable=expArg complete
#pragma HLS array_partition variable=phiMag complete
#pragma HLS array_partition variable=cosArg complete
#pragma HLS array_partition variable=sinArg complete
#pragma HLS array_partition variable=Qracc_p complete
#pragma HLS array_partition variable=Qiacc_p complete

 c_label_x:for(unsigned indexX = 0; indexX < NUMX; indexX++)
    {
      x = _inbuff[indexX];
      y = _inbuff[numX + indexX];
      z = _inbuff[2*numX + indexX];
      Qracc = 0;
      Qiacc = 0;

    c_label_k:for(unsigned indexK = 0; indexK < NUMK; indexK += PRL)
        {
#pragma HLS pipeline
        c_label_k0:for(unsigned i=0; i<PRL; i++) {
#pragma HLS unroll
	    unsigned idx = indexK + i;

	    kx[i] =   _inbuff_k[idx];
	    ky[i] =   _inbuff_k[idx + numK];
	    kz[i] =   _inbuff_k[idx + numK * 2];
	    phiR[i] = _inbuff_k[idx + numK * 3];
	    phiI[i] = _inbuff_k[idx + numK * 4];
          }

        c_label_k1:for(unsigned i=0; i<PRL; i++) {
#pragma HLS unroll
	    phiMag[i] = phiR[i] * phiR[i] + phiI[i] * phiI[i];
	    expArg[i] = PI2 * (kx[i] * x + ky[i] * y + kz[i] * z);
	    cosArg[i] = hls::cos(expArg[i]);
	    sinArg[i] = hls::sin(expArg[i]);
	    Qracc_p[i] = phiMag[i] * cosArg[i];
	    Qiacc_p[i] = phiMag[i] * sinArg[i];
          }
          {
	    Qracc += Qracc_p[0] + Qracc_p[1] + Qracc_p[2] + Qracc_p[3];
	    Qiacc += Qiacc_p[0] + Qiacc_p[1] + Qiacc_p[2] + Qiacc_p[3];
          }

        } // end of outer k                                                                                                                                  

      _outbuff[indexX] = Qracc;
      _outbuff[indexX + numX] = Qiacc;

      //      std::cout << "Qr[" << indexX << "] = " << _outbuff[indexX] << std::endl;                                                                       
      //      std::cout << "Qi[" << indexX << "] = " <<  _outbuff[indexX + numX] << std::endl;                                                               
    } // end of x                         



}




void top(dma_word_t *out, dma_word_t *in1,
         /* <<--params-->> */
	 const unsigned conf_info_numX,
	 const unsigned conf_info_numK,
 	 dma_info_t &load_ctrl, 
	 dma_info_t &store_ctrl)
{

    /* <<--local-params-->> */
	 const unsigned numX = conf_info_numX;
	 const unsigned numK = conf_info_numK;

    // Batching
batching:
    for (unsigned b = 0; b < 1; b++)
    {
        // Chunking
      // first loading all the k-data into PLM
      

      bool load_k = true;

    go:
        for (int c = 0; c < 1; c++)
        {
            word_t _inbuff[SIZE_IN_CHUNK_DATA]; // store x, y, z
            word_t _outbuff[SIZE_OUT_CHUNK_DATA];
	    word_t _inbuff_k[SIZE_IN_K_DATA]; // store kx, ky, kz

#pragma HLS array_partition variable=_inbuff_k block factor=5
#pragma HLS array_partition variable=_inbuff block factor=3
#pragma HLS array_partition variable=_outbuff block factor=2

            load(_inbuff, _inbuff_k, in1,
                 /* <<--args-->> */
	 	 numX,
	 	 numK,
                 load_ctrl, c, b, &load_k);

            compute(_inbuff, _inbuff_k,
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
