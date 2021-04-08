// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include "libesp.h"
#include "cfg.h"
#include "../../../common/utils.h"     // init_parameters() and validate_buffer()
#include "../../../common/sw_exec.h"   // sw_exec()
#include "../../../common/init_buff.h" // init_buffer()
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

static void init_buf(token_t *in, float *in_fp, float *gold, 
		      const char* inputFile, 
		      const char* goldFile)
{



  init_buffer(in_fp, gold, inputFile, goldFile, 
	      batch_size_x, num_batch_x,
	      batch_size_k, num_batch_k);

  for(int i=0; i < in_len; i++) {
    in[i] = float2fx(in_fp[i], FX_IL);
  }

}

 


int main(int argc, char **argv)
{
        if (argc < 3) {
	  printf("please pass 3 arguments: inputName, goldName, run_sw\n");
	  exit(1);
	}

	int errors;
	float *gold;
	token_t *buf;
	float *in_fp;

	const char* inputFile = argv[1];
	const char* goldFile  = argv[2];
	int run_sw = atoi(argv[3]); 

	//	printf("\n before init parameters \n");

	init_parameters(batch_size_x, num_batch_x, 
			batch_size_k, num_batch_k,
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
	
	printf("\n====== %s ======\n\n", cfg_000[0].devname);
	/* <<--print-params-->> */
	printf("  .num_batch_k = %d\n", num_batch_k);
	printf("  .batch_size_k = %d\n", batch_size_k);
	printf("  .num_batch_x = %d\n", num_batch_x);
	printf("  .batch_size_x = %d\n", batch_size_x);

	init_buf(buf, in_fp, gold, inputFile, goldFile);

	printf("\n  ** START HW TESTING **\n");

	esp_run(cfg_000, NACC);

	printf("\n  ** DONE **\n");

	errors = validate_buf(&buf[out_offset], gold);

	if (!errors)
		printf("+ HW Test PASSED\n");
	else
		printf("+ HW Test FAILED\n");


	if(run_sw) {

	  sw_exec(gold, in_fp, 
		  batch_size_x, num_batch_x,
		  batch_size_k, num_batch_k);
	}

	free(gold);
	free(in_fp);

	esp_free(buf);


	printf("\n====== %s ======\n\n", cfg_000[0].devname);


	return errors;

}
