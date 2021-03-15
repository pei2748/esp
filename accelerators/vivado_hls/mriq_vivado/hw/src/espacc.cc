// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "../inc/espacc_config.h"
#include "../inc/espacc.h"
#include "hls_stream.h"
#include "hls_math.h"
#include <cstring>
#include "espacc_utils.h" // dma_read and dma_write

#define PRL 4

// to get latency info for synthesis analysis
#define NUMX 4
#define NUMK 16 


void config_dma(dma_index_load_t &dma_index_load,
		dma_length_load_t &dma_length_load,
		dma_index_store_t &dma_index_store,
		dma_length_store_t &dma_length_store,
		const unsigned numX,
		const unsigned numK,		
		unsigned chunk, unsigned batch)	    

{
  const unsigned store_offset = round_up(3*numX + 5*numK, VALUES_PER_WORD) >> (VALUES_PER_WORD - 1);
  const unsigned dma_length_x = round_up(numX, VALUES_PER_WORD) >> (VALUES_PER_WORD - 1);
  const unsigned dma_length_k = round_up(numK, VALUES_PER_WORD) >> (VALUES_PER_WORD - 1);
	  // configure dma index for load process
	  dma_index_load.kx = 0;
	  dma_index_load.ky = dma_length_k;
	  dma_index_load.kz = dma_length_k * 2;
	  dma_index_load.phiR = dma_length_k * 3;
	  dma_index_load.phiI = dma_length_k * 4;

	  dma_index_load.x = dma_length_k * 5;
	  dma_index_load.y = dma_length_k * 5 + dma_length_x;
	  dma_index_load.z = dma_length_k * 5 + dma_length_x * 2;

	  // configure dma index for store process
	  dma_index_store.Qr = store_offset + dma_length_x * 2 * chunk;
	  dma_index_store.Qi = store_offset + dma_length_x * (2 * chunk + 1);


	  // configure dma length for load process
	  dma_length_load.kx = dma_length_k;
	  dma_length_load.ky = dma_length_k;
	  dma_length_load.kz = dma_length_k;
	  dma_length_load.phiR = dma_length_k;
	  dma_length_load.phiI = dma_length_k;

	  dma_length_load.x = dma_length_x;
	  dma_length_load.y = dma_length_x;
	  dma_length_load.z = dma_length_x;

	  // configure dma length for store process
	  dma_length_store.Qr = dma_length_x;
	  dma_length_store.Qi = dma_length_x;
}

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
	  dma_length_load_t dma_length_load,
	  dma_index_load_t dma_index_load,
	  dma_info_t &load_ctrl, 
	  int chunk, int batch,
	  bool *load_k)
{
 load_data:



  load_ctrl.index = dma_index_load.kx;
  load_ctrl.length = dma_length_load.kx;
  load_ctrl.size = SIZE_WORD_T;  

  ap_wait(); //this one is necessary.
            dma_read(_inbuff_kx, dma_index_load.kx, dma_length_load.kx, in1);
  ap_wait(); // this one is necessary

  load_ctrl.index = dma_index_load.ky;
  load_ctrl.length = dma_length_load.ky;
  load_ctrl.size = SIZE_WORD_T;

  ap_wait();
  
          dma_read(_inbuff_ky, dma_index_load.ky, dma_length_load.ky, in1);

  ap_wait();
  load_ctrl.index = dma_index_load.kz;
  load_ctrl.length = dma_length_load.kz;
  load_ctrl.size = SIZE_WORD_T;

  ap_wait();
            dma_read(_inbuff_kz, dma_index_load.kz, dma_length_load.kz, in1);
  ap_wait();

  load_ctrl.index = dma_index_load.phiR;
  load_ctrl.length = dma_length_load.phiR;
  load_ctrl.size = SIZE_WORD_T;

  ap_wait();
            dma_read(_inbuff_phiR, dma_index_load.phiR, dma_length_load.phiR, in1);
  ap_wait();

  load_ctrl.index = dma_index_load.phiI;
  load_ctrl.length = dma_length_load.phiI;
  load_ctrl.size = SIZE_WORD_T;

  ap_wait();
            dma_read(_inbuff_phiI, dma_index_load.phiI, dma_length_load.phiI, in1);
  ap_wait();

  load_ctrl.index = dma_index_load.x;
  load_ctrl.length = dma_length_load.x;
  load_ctrl.size = SIZE_WORD_T;

  ap_wait();
            dma_read(_inbuff_x, dma_index_load.x, dma_length_load.x, in1);
  ap_wait();

  load_ctrl.index = dma_index_load.y;
  load_ctrl.length = dma_length_load.y;
  load_ctrl.size = SIZE_WORD_T;

  ap_wait();
            dma_read(_inbuff_y, dma_index_load.y, dma_length_load.y, in1);
  ap_wait();

  load_ctrl.index = dma_index_load.z;
  load_ctrl.length = dma_length_load.z;
  load_ctrl.size = SIZE_WORD_T;

  ap_wait();
            dma_read(_inbuff_z, dma_index_load.z, dma_length_load.z, in1);


}


void store(word_t _outbuff_Qr[NUMX], word_t _outbuff_Qi[NUMX],
	   dma_word_t *out,
	   /* <<--compute-params-->> */
	   const unsigned numX,
	   const unsigned numK,
	  dma_length_store_t dma_length_store,
	  dma_index_store_t dma_index_store,
	   dma_info_t &store_ctrl, 
	   int chunk, int batch)
{
 store_data:

  store_ctrl.index = dma_index_store.Qr;
  store_ctrl.length = dma_length_store.Qr;
  store_ctrl.size = SIZE_WORD_T;


  dma_write(_outbuff_Qr, dma_index_store.Qr, dma_length_store.Qr, out);
  ap_wait();

  store_ctrl.index = dma_index_store.Qi;
  store_ctrl.length = dma_length_store.Qi;
  store_ctrl.size = SIZE_WORD_T;

  ap_wait();
  dma_write(_outbuff_Qi, dma_index_store.Qi, dma_length_store.Qi, out);

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
#pragma HLS pipeline II=2


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
	    
	  //  word_t _outbuff_Qr[NUMX];
	  word_t _outbuff_Qr[NUMX];
	  word_t _outbuff_Qi[NUMX];



	  dma_index_load_t dma_index_load;
	  dma_length_load_t dma_length_load;
	  dma_index_store_t dma_index_store;
	  dma_length_store_t dma_length_store;

	  config_dma(dma_index_load,
		     dma_length_load,
		     dma_index_store,
		     dma_length_store,
		     numX,
		     numK,
		     c, b);

	  load(_inbuff_x,_inbuff_y,_inbuff_z,
	       _inbuff_kx,_inbuff_ky, _inbuff_kz, _inbuff_phiR, _inbuff_phiI, 
	       in1,
	       /* <<--args-->> */
	       numX,
	       numK,
	       dma_length_load,
	       dma_index_load,
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
		dma_length_store,
		dma_index_store,
		store_ctrl, c, b);
        }
    }
}



