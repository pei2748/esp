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


void load(word_t _inbuff_x[NUMX],
	  word_t _inbuff_y[NUMX],
	  word_t _inbuff_z[NUMX],
	  word_t _inbuff_kx[NUMK],
	  word_t _inbuff_ky[NUMK],
	  word_t _inbuff_kz[NUMK],
	  word_t _inbuff_phiR[NUMK],
	  word_t _inbuff_phiI[NUMK],
	  dma_word_t *in1,
          /* <<--compute-params-->> */
	 const unsigned numX,
	 const unsigned numK,
	  dma_info_t &load_ctrl, int chunk, int batch,
	  bool *load_k)
{
load_data:
	unsigned dma_length;
	unsigned dma_length_x, dma_length_k;
	unsigned dma_index;

	// numX and numK are always even numbers
	// round_up would always gets the same number of numX, no matter the value_per_word.

	dma_length = round_up(3*numX+5*numK, VALUES_PER_WORD) >> (VALUES_PER_WORD - 1);
	dma_length_x = round_up(numX, VALUES_PER_WORD) >> (VALUES_PER_WORD - 1);// must be an even number
	dma_length_k = round_up(numK, VALUES_PER_WORD) >> (VALUES_PER_WORD - 1);// must be an even number


	// ********************** load kx ****************************//

	//	for(unsigned oo=0; oo< VALUES_PER_WORD; oo++) {

	dma_index = 0;

	load_ctrl.index = dma_index;
	load_ctrl.length = dma_length;
	load_ctrl.size = SIZE_WORD_T;

	for (unsigned i = 0; i < dma_length_k; i++) {		  
		for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
#pragma HLS unroll
		  	_inbuff_kx[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
		}		  
	}

	// ********************** load ky ****************************//

	dma_index += dma_length_k;

	for (unsigned i = 0; i < dma_length_k; i++) {		  
		for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
#pragma HLS unroll
		  	_inbuff_ky[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
		}		  
	}

	// ********************** load kz ****************************//
	dma_index += dma_length_k;
	for (unsigned i = 0; i < dma_length_k; i++) {		  
		for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
#pragma HLS unroll
		  	_inbuff_kz[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
		}		  
	}

	// ********************** load phiR ****************************//
	dma_index += dma_length_k;
	for (unsigned i = 0; i < dma_length_k; i++) {		  
		for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
		  #pragma HLS unroll
		  _inbuff_phiR[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
		}		  
	}

	// ********************** load phiI ****************************//
	dma_index += dma_length_k;
	for (unsigned i = 0; i < dma_length_k; i++) {		  
		for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
#pragma HLS unroll
		  	_inbuff_phiI[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
		}		  
	}

	// ********************** load x ****************************//
	dma_index +=  dma_length_k;

//
	for (unsigned i = 0; i < dma_length_x; i++) {		  
		for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
#pragma HLS unroll
		  	_inbuff_x[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
			//	std::cout << "x[" << i * VALUES_PER_WORD + j << "] = " << _inbuff_x[i * VALUES_PER_WORD + j] << std::endl;
		}		  
	}

                                                                       
	// ********************** load y ****************************//
	dma_index += dma_length_x; 
	//length and size are the same.

	for (unsigned i = 0; i < dma_length_x; i++) {		  
		for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
#pragma HLS unroll
		  	_inbuff_y[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
		}		  
	}
	// ********************** load z ****************************//
	dma_index += dma_length_x; 
	//length and size are the same.

	for (unsigned i = 0; i < dma_length_x; i++) {		  
		for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
#pragma HLS unroll
		  	_inbuff_z[i * VALUES_PER_WORD + j] = in1[dma_index + i].word[j];
		}		  
	}


}


void store(word_t _outbuff_Qr[NUMX], word_t _outbuff_Qi[NUMX],
	   dma_word_t *out,
          /* <<--compute-params-->> */
	 const unsigned numX,
	 const unsigned numK,
	   dma_info_t &store_ctrl, int chunk, int batch)
{
store_data:

    const unsigned length = round_up(2 * numX, VALUES_PER_WORD);
    const unsigned store_offset = round_up(3*numX + 5*numK, VALUES_PER_WORD) * 1;
    const unsigned out_offset = store_offset;
    const unsigned index = out_offset + length * (batch * 1 + chunk);

    unsigned dma_length = length / VALUES_PER_WORD;
    unsigned dma_index = index / VALUES_PER_WORD;
    unsigned dma_length_x = round_up(numX, VALUES_PER_WORD) >> (VALUES_PER_WORD - 1);

    store_ctrl.index = dma_index;
    store_ctrl.length = dma_length;
    store_ctrl.size = SIZE_WORD_T;

    for (unsigned i = 0; i < dma_length_x; i++) {
		  for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
		  #pragma HLS unroll
		       out[dma_index + i].word[j] = _outbuff_Qr[i * VALUES_PER_WORD + j];
		}
    }

    dma_index += dma_length_x;
//    store_ctrl.index = dma_index;
//    store_ctrl.length = dma_length;
//    store_ctrl.size = SIZE_WORD_T;


    for (unsigned i = 0; i < dma_length_x; i++) {
		  for(unsigned j = 0; j < VALUES_PER_WORD; j++) {
		  #pragma HLS unroll
		       out[dma_index + i].word[j] = _outbuff_Qi[i * VALUES_PER_WORD + j];
		}
    }

    //#endif

}



void compute(word_t _inbuff_x[NUMX],
	     word_t _inbuff_y[NUMX],
	     word_t _inbuff_z[NUMX],
	     word_t _inbuff_kx[NUMK],
	     word_t _inbuff_ky[NUMK],
	     word_t _inbuff_kz[NUMK],
	     word_t _inbuff_phiR[NUMK],
	     word_t _inbuff_phiI[NUMK],
             /* <<--compute-params-->> */
	     const unsigned numX,
	     const unsigned numK,
             word_t _outbuff_Qr[NUMX],
             word_t _outbuff_Qi[NUMX])
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
      x = _inbuff_x[indexX];
      y = _inbuff_y[indexX];
      z = _inbuff_z[indexX];
      Qracc = 0;
      Qiacc = 0;

    c_label_k:for(unsigned indexK = 0; indexK < NUMK; indexK += PRL)
        {
#pragma HLS pipeline


        c_label_k0:for(unsigned i=0; i<PRL; i++) {
#pragma HLS unroll
	    unsigned idx = indexK + i;

	    kx[i] =   _inbuff_kx[idx];
	    ky[i] =   _inbuff_ky[idx];
	    kz[i] =   _inbuff_kz[idx];
	    phiR[i] = _inbuff_phiR[idx];
	    phiI[i] = _inbuff_phiI[idx];
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
//      _outbuff_Qr[indexX] = Qracc;
//      _outbuff_Qi[indexX] = Qiacc;

      _outbuff_Qr[indexX] = Qracc;
      _outbuff_Qi[indexX] = Qiacc;


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
	  word_t _inbuff_x[NUMX];
	  word_t _inbuff_y[NUMX];
	  word_t _inbuff_z[NUMX];


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
	  
	  //	  word_t _outbuff_Qr[NUMX];
	  word_t _outbuff_Qr[NUMX];
	  word_t _outbuff_Qi[NUMX];


	  //#pragma HLS array_partition variable=_outbuff block factor=2

            load(_inbuff_x,_inbuff_y,_inbuff_z,
		 _inbuff_kx,_inbuff_ky, _inbuff_kz, _inbuff_phiR, _inbuff_phiI, 
		 in1,
                 /* <<--args-->> */
	 	 numX,
	 	 numK,
                 load_ctrl, c, b, &load_k);

            compute(_inbuff_x,_inbuff_y,_inbuff_z,
		 _inbuff_kx,_inbuff_ky, _inbuff_kz, _inbuff_phiR, _inbuff_phiI, 
                    /* <<--args-->> */
	 	 numX,
	 	 numK,
		    _outbuff_Qr, _outbuff_Qi);
            store(_outbuff_Qr, _outbuff_Qi, out,
                  /* <<--args-->> */
	 	 numX,
	 	 numK,
                  store_ctrl, c, b);
        }
    }
}
