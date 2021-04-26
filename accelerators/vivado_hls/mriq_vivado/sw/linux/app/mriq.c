// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "libesp.h"
#include "cfg.h"
#include <fixed_point.h> // for fx2float

#include "../../../common/utils.h"     // init_parameters() and validate_buffer()
#include "../../../common/sw_exec.h"   // sw_exec()
#include "../../../common/init_buff.h" // init_buffer()

#define fx2float fixed32_to_float
#define float2fx float_to_fixed32
#define FX_IL 12

#define NUMK 1024
#define BATCH_SIZE_X 8

static unsigned in_words_adj;
static unsigned out_words_adj;
static unsigned in_len;
static unsigned out_len;
static unsigned in_size;
static unsigned out_size;
static unsigned out_offset;
static unsigned mem_size;

/* User-defined code */

static int validate_buf(token_t *out, float *gold)
{
  float *out_fp;
  int ret;
  out_fp = malloc(out_len * sizeof(float));

  for (int i = 0; i < out_len; i++) {
    out_fp[i] = fx2float(out[i], FX_IL);
  }

  ret = validate_buffer(out_fp, gold, out_len);

  free(out_fp);
  return ret;
}


/* User-defined code */
static void init_buf(token_t *inbuff, float *inbuff_fp, float *outbuff_gold,
		     const char* inputFile, 
		     const char* goldFile)
{

  // Prepare input data

#ifdef __sparc

  const char* be_inputFile = "be_inputFile.bin";
  const char* be_goldFile = "be_goldFile.bin";

  file2be(inputFile, be_inputFile);
  file2be(goldFile, be_goldFile);

  init_buffer(inbuff_fp, outbuff_gold, be_inputFile, be_goldFile, 
	      BATCH_SIZE_X, num_batch_x, NUMK);

#else

  init_buffer(inbuff_fp, outbuff_gold, inputFile, goldFile, 
	      BATCH_SIZE_X, num_batch_x, NUMK);
#endif



  for(int i=0; i < in_len; i++) {
    inbuff[i] = float2fx(inbuff_fp[i], FX_IL);
  }

}




int main(int argc, char **argv)
{
	int errors;

	float *gold;
	float *in_fp;
	token_t *buf;

	const char* inputFile = argv[1];
	const char* goldFile  = argv[2];
	int run_sw = atoi(argv[3]); 


	init_parameters(BATCH_SIZE_X, num_batch_x, 
			NUMK,
			&in_words_adj, &out_words_adj,
			&in_len, &out_len,
			&in_size, &out_size,
			&out_offset,
			&mem_size,
			DMA_WORD_PER_BEAT(sizeof(token_t)));

	buf = (token_t *) esp_alloc(mem_size);

	cfg_000[0].hw_buf = buf;
    
	gold = malloc(out_len * sizeof(float));
	in_fp = malloc(in_len * sizeof(float));


	init_buf(buf, in_fp, gold, inputFile, goldFile);

	printf("\n====== %s ======\n\n", cfg_000[0].devname);
	/* <<--print-params-->> */
	printf("  .num_batch_x = %d\n", num_batch_x);

	printf("\n  ** START HW TESTING**\n");

	esp_run(cfg_000, NACC);

	printf("\n  ** DONE **\n");

	errors = validate_buf(&buf[out_offset], gold);


	if (!errors)
		printf("+ Test PASSED\n");
	else
		printf("+ Test FAILED\n");


	if (run_sw) {	    
	  float *out_sw = malloc(out_len * sizeof(float));

	  sw_exec(out_sw, in_fp, 
		  BATCH_SIZE_X, num_batch_x,
		  NUMK);

	  int ret;
	  ret = validate_buffer(out_sw, gold, out_len);

	  if (ret)
	    printf("+ SW Test FAILED!\n");
	  else
	    printf("+ SW Test PASSED!\n");

	  free(out_sw);
	}


	printf("\n====== %s ======\n\n", cfg_000[0].devname);


	free(gold);
	free(in_fp);
	esp_free(buf);

	return errors;
}
