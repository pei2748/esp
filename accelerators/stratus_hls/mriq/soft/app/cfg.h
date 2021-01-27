#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"

typedef int32_t token_t;
typedef float native_t;

// The following define is for the WL = 32.
#define fx2float fixed32_to_float
#define float2fx float_to_fixed32
#define FX_IL 12

/* <<--params-def-->> */
/* For 64 dataset */
/*
#define NUMX 262144 
#define NUMK 2048
#define BATCH_SIZE_X 1024
#define BATCH_SIZE_K 2048 
#define NUM_BATCH_X 256 
#define NUM_BATCH_K 1
*/
/* For 128 dataset  */

#define NUMX 256
#define NUMK 262144
#define BATCH_SIZE_X 256
#define BATCH_SIZE_K 1024
#define NUM_BATCH_X  1
#define NUM_BATCH_K 256




/* For 32 dataset  */
/*
#define NUMX 32768 
#define NUMK 3072 
#define BATCH_SIZE_X 1024
#define BATCH_SIZE_K 3072 
#define NUM_BATCH_X  32 
#define NUM_BATCH_K 1
*/

/* For 64 dataset for A1 */
/*
#define NUMX 262144
#define NUMK 2048
#define BATCH_SIZE_X 1024
#define BATCH_SIZE_K 1 
#define NUM_BATCH_X 256
#define NUM_BATCH_K 2048
*/

/* <<--params-->> */
const int32_t numX = NUMX;
const int32_t numK = NUMK;
const int32_t batch_size_x = BATCH_SIZE_X;
const int32_t batch_size_k = BATCH_SIZE_K;
const int32_t num_batch_x = NUM_BATCH_X;
const int32_t num_batch_k = NUM_BATCH_K;



#define NACC 1

esp_thread_info_t cfg_000[] = {
	{
		.run = true,
		.devname = "mriq.0",
		.type = mriq,
		/* <<--descriptor-->> */
		.desc.mriq_desc.numX = NUMX,
		.desc.mriq_desc.numK = NUMK,
 		.desc.mriq_desc.batch_size_x = BATCH_SIZE_X,
		.desc.mriq_desc.batch_size_k = BATCH_SIZE_K,
		.desc.mriq_desc.num_batch_x = NUM_BATCH_X,
		.desc.mriq_desc.num_batch_k = NUM_BATCH_K,
		.desc.mriq_desc.src_offset = 0,
		.desc.mriq_desc.dst_offset = 0,
		.desc.mriq_desc.esp.coherence = ACC_COH_NONE,
		.desc.mriq_desc.esp.p2p_store = 0,
		.desc.mriq_desc.esp.p2p_nsrcs = 0,
		.desc.mriq_desc.esp.p2p_srcs = {"", "", "", ""},
	}
};

#endif /* __ESP_CFG_000_H__ */
