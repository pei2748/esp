// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "libesp.h"
#include "cfg.h"
#include <math.h> // for fabs function 
#include "../../../hw/tb/helper.h" // for two reading file functions
#include <fixed_point.h> // for fx2float
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
      diff = fabs((fx2float(gold[i * out_words_adj + j], FX_IL) - fx2float(out[i * out_words_adj + j], FX_IL))
                  /fx2float(gold[i * out_words_adj + j], FX_IL));

      if (diff > error_th)
        errors++;
    }

  return errors;

}


/* User-defined code */
static void init_buffer(token_t *inbuff, token_t *outbuff_gold)
{

  // Prepare input data

  // these two files should be put into sysroot.

  char* inputName =  "test_small.bin";
  char* goldName =   "test_small.out";


  float *kx, *ky, *kz;          /* K trajectory (3D vectors) */
  float *x, *y, *z;             /* X coordinates (3D vectors) */
  float *phiR, *phiI;           /* Phi values (complex) */
  float *goldQr, *goldQi;
  int numX_bm, numK_bm; /* should be the same value as numX and numK */

  inputData(inputName, &numK_bm, &numX_bm,
	    &kx, &ky, &kz,
	    &x,  &y,  &z,
	    &phiR, &phiI);


  goldQr = (float *) malloc(sizeof(float) * numX);
  goldQi = (float *) malloc(sizeof(float) * numX);
  outputData(goldName, &goldQr, &goldQi, &numX_bm);
  unsigned i;
  for(i = 0; i < numK; i++) {
    inbuff[i] = float2fx(kx[i], FX_IL);
    inbuff[i + numK] = float2fx(ky[i], FX_IL);
    inbuff[i + 2 * numK] = float2fx(kz[i], FX_IL);
    inbuff[i + 3 * numK] = float2fx(phiR[i], FX_IL);
    inbuff[i + 4 * numK] = float2fx(phiI[i], FX_IL);
  }

  for(i = 0; i < numX; i++){
    inbuff[i + 5 * numK] = float2fx(x[i], FX_IL);
    inbuff[i + 5 * numK + numX] = float2fx(y[i], FX_IL);
    inbuff[i + 5 * numK + 2 * numX] = float2fx(z[i], FX_IL);
  }


  // Set golden output

  for(unsigned j = 0; j < numX; j++)
    outbuff_gold[j] = float2fx(goldQr[j], FX_IL);


  for(unsigned j=0; j < numX; j++)
    outbuff_gold[j + numX] = float2fx(goldQi[j], FX_IL);

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
}


/* User-defined code */
static void init_parameters()
{
	if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
		in_words_adj = 3*numX + 5*numK;
		out_words_adj = 2*numX;
	} else {
		in_words_adj = round_up(3*numX + 5*numK, DMA_WORD_PER_BEAT(sizeof(token_t)));
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

	init_parameters();

	buf = (token_t *) esp_alloc(size);
	cfg_000[0].hw_buf = buf;
    
	gold = malloc(out_size);

	init_buffer(buf, gold);

	printf("\n====== %s ======\n\n", cfg_000[0].devname);
	/* <<--print-params-->> */
	printf("  .numX = %d\n", numX);
	printf("  .numK = %d\n", numK);
	printf("\n  ** START **\n");

	esp_run(cfg_000, NACC);

	printf("\n  ** DONE **\n");

	errors = validate_buffer(&buf[out_offset], gold);

	free(gold);
	esp_free(buf);

	if (!errors)
		printf("+ Test PASSED\n");
	else
		printf("+ Test FAILED\n");

	printf("\n====== %s ======\n\n", cfg_000[0].devname);

	return errors;
}
