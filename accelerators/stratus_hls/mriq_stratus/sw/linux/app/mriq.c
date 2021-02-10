// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "libesp.h"
#include "cfg.h"
#include "../../../hw/tb/helper.h"
//#include <malloc.h>
#include <fixed_point.h>

#include <math.h> // for fabs function


#define fx2float fixed32_to_float
#define float2fx float_to_fixed32
#define FX_IL 12



static unsigned in_words_adj;
static unsigned out_words_adj;
static unsigned in_len;
static unsigned out_len;
static unsigned in_size;
static unsigned out_size;
static unsigned out_offset;
static unsigned size;

//float *kx, *ky, *kz, *x, *y, *z, *phiR, *phiI;
//float *sw_out, *Qr, *Qi;

#define PIx2 6.2831853071795864769252867665590058f

static void sw_comp(int numK, int numX,
              float *kx, float *ky, float *kz,
              float *x, float *y, float *z,
              float *phiR, float *phiI,
              float *out)
{
  float expArg;
  float cosArg;
  float sinArg;
  float phiMag;
  int indexK, indexX;
  

  for (indexX = 0; indexX < numX; indexX++) {
    // Sum the contributions to this point over all frequencies
    float Qracc = 0.0f;
    float Qiacc = 0.0f;
    for (indexK = 0; indexK < numK; indexK++) {
      phiMag = phiR[indexK]*phiR[indexK] + phiI[indexK]*phiI[indexK];

      expArg = PIx2 * (kx[indexK] * x[indexX] +
                       ky[indexK] * y[indexX] +
                       kz[indexK] * z[indexX]);
      cosArg = cosf(expArg);
      sinArg = sinf(expArg);

      Qracc += phiMag * cosArg;
      Qiacc += phiMag * sinArg;
    }
    out[indexX] = Qracc;
    out[numX + indexX] = Qiacc;
  }
}


/* User-defined code */
static int validate_buffer(token_t *out, token_t *gold)
{

  int i;
  int j;
  unsigned errors = 0;
  float diff;
  float error_th = 0.01;

  for (i = 0; i < 1; i++)
    for (j = 0; j < 2*numX; j++){
      int idx = i * out_words_adj + j;
      if(!fx2float(gold[idx], FX_IL) && !fx2float(out[idx], FX_IL))
        diff = 0;
      else if(!fx2float(gold[idx], FX_IL))
        diff = fabs((fx2float(gold[idx], FX_IL) - fx2float(out[idx], FX_IL))
                    /fx2float(out[idx], FX_IL));
      else
        diff = fabs((fx2float(gold[idx], FX_IL) - fx2float(out[idx], FX_IL))
                    /fx2float(gold[idx], FX_IL));

      if (diff > error_th)
        errors++;
    }

  return errors;
}


/* User-defined code */


static void init_buffer(token_t *in, token_t *gold, 
			const char* inputFile, 
			const char* goldFile)
{

  int numX_bm, numK_bm;
  float *kx, *ky, *kz, *x, *y, *z, *phiR, *phiI;


  inputData(inputFile, 
	      &numK_bm, 
	      &numX_bm, 
	      &kx, &ky, &kz, 
	      &x, &y, &z, 
	      &phiR, &phiI);

  int i, j;
  int offset_idx = 0;
  int idx_single;


  for(i=0; i < num_batch_k; i++) {
      idx_single = i * num_batch_k;


      for(j=0; j < batch_size_k; j++)  
	in[offset_idx + j] = float2fx(kx[idx_single + j], FX_IL);

      offset_idx += batch_size_k; // 1 

      for(j=0; j < batch_size_k; j++)
	in[offset_idx + j] = float2fx(ky[idx_single + j], FX_IL);
      
      offset_idx += batch_size_k; // 2

      for(j=0; j < batch_size_k; j++)
	in[offset_idx + j] = float2fx(kz[idx_single + j], FX_IL);
            
      offset_idx += batch_size_k; // 3

      for(j=0; j < batch_size_k; j++)
	in[offset_idx + j] = float2fx(phiR[idx_single + j], FX_IL);
      
      offset_idx += batch_size_k; // 4

      for(j=0; j < batch_size_k; j++)
	in[offset_idx + j] = float2fx(phiI[idx_single + j], FX_IL);
      
      offset_idx += batch_size_k; // 5
  }
  

  for(i = 0; i < num_batch_x; i++) {
      idx_single = i * num_batch_x;

      for(j=0; j < batch_size_x; j++)
	in[offset_idx + j] = float2fx(x[idx_single + j], FX_IL);
      offset_idx += batch_size_x; // 1

      for(j=0; j < batch_size_x; j++)
	in[offset_idx + j] = float2fx(y[idx_single + j], FX_IL);
      offset_idx += batch_size_x; // 2

      for(j=0; j < batch_size_x; j++)
	in[offset_idx + j] = float2fx(z[idx_single + j], FX_IL);
      offset_idx += batch_size_x; // 3
  }


  // If golden output is computed from code, then this part should be sw_comp part
  struct timespec ts_start, ts_end;
  unsigned long long sw_ns;

  float *gold_fp;
  gold_fp = malloc(sizeof(float) * 2 * numX);

  printf("\n  ** START SW TESTING **\n");
  gettime(&ts_start);

  sw_comp(numK, numX, \
       kx,\
       ky,\
       kz,\
       x,\
       y,\
       z,\
       phiR,\
       phiI,\
       gold_fp);
  gettime(&ts_end);
  
  sw_ns = ts_subtract(&ts_start, &ts_end);
  printf("\n SW exec time : %llu (ns)\n", sw_ns);
  printf("\n  ** Done! **\n\n");


  // store computed golden_output to gold buf
  for(i = 0; i < 2 * numX; i++) 
    gold[i] = float2fx(gold_fp[i], FX_IL);


  // read golden output from files and store to gold buf 

//  float *Qr, *Qi;
//  outputData(goldFile, &Qr, &Qi, &numX_bm);
//
//  for(i = 0; i < numX; i++) {
//    gold[i] = float2fx(Qr[i], FX_IL);
//    gold[i + numX] = float2fx(Qi[i], FX_IL);
//  }
//  // if read golden output from file
//  free(Qr);
//  free(Qi);
//




  free(x);
  free(y);
  free(z);
  free(kx);
  free(ky);
  free(kz);
  free(phiR);
  free(phiI);
  // if get golden output from sw_comp
  free(gold_fp);
}

/* User-defined code */




/* User-defined code */
static void init_parameters()
{
	if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
		in_words_adj = 3*numX+5*numK;
		out_words_adj = 2*numX;
	} else {
		in_words_adj = round_up(3*numX+5*numK, DMA_WORD_PER_BEAT(sizeof(token_t)));
		out_words_adj = round_up(2*numX, DMA_WORD_PER_BEAT(sizeof(token_t)));
	}
	in_len = in_words_adj * (1);
	out_len =  out_words_adj * (1);
	in_size = in_len * sizeof(token_t);
	out_size = out_len * sizeof(token_t);
	out_offset = in_len;
	size = (out_offset * sizeof(token_t)) + out_size;
}


int main(int argc, char **argv)
{
	int errors;

	token_t *gold;
	token_t *buf;

	const char* inputFile = "test_small.bin";
	const char* goldFile  = "test_small.out";

	//	printf("\n before init parameters \n");
	init_parameters();

	buf = (token_t *) esp_alloc(size);

	cfg_000[0].hw_buf = buf;
    
	gold = malloc(out_size);

	printf("\n before init buffers \n");
	
	printf("\n====== %s ======\n\n", cfg_000[0].devname);
	/* <<--print-params-->> */
	printf("  .numX = %d\n", numX);
	printf("  .numK = %d\n", numK);
	printf("  .num_batch_k = %d\n", num_batch_k);
	printf("  .batch_size_k = %d\n", batch_size_k);
	printf("  .num_batch_x = %d\n", num_batch_x);
	printf("  .batch_size_x = %d\n", batch_size_x);

	// init_buffer probably has sw_comp, so it is in front of HW TESTING

	init_buffer(buf, gold, inputFile, goldFile);
	//	init_buffer(buf, gold);



	printf("\n  ** START HW TESTING **\n");



	esp_run(cfg_000, NACC);

	printf("\n  ** DONE **\n");

	errors = validate_buffer(&buf[out_offset], gold);

	free(gold);
	esp_free(buf);

	if (!errors)
		printf("+ Test PASSED\n");
	else
		printf("+ Test FAILED\n");

	/*
	printf("\n  ** START SW TESTING **\n");

	struct timespec ts_start, ts_end;
	unsigned long long sw_ns;

	gettime(&ts_start);
	sw_comp();
	gettime(&ts_end);

	sw_ns = ts_subtract(&ts_start, &ts_end);
	printf("\n SW exec time : %llu ns\n", sw_ns);

	*/
	printf("\n====== %s ======\n\n", cfg_000[0].devname);

	//	free(sw_out);
	return errors;

}
