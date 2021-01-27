#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"

typedef int32_t token_t;

// The following define is for the WL = 32.
#define fx2float fixed32_to_float
#define float2fx float_to_fixed32
#define FX_IL 12

/* <<--params-def-->> */
#define NUMX /*32768*/ 4
#define NUMK /*3072*/ 16

/* <<--params-->> */
const int32_t numX = NUMX;
const int32_t numK = NUMK;

#define NACC 1

esp_thread_info_t cfg_000[] = {
	{
		.run = true,
		.devname = "mriq_v2.0",
		.type = mriq_v2,
		/* <<--descriptor-->> */
		.desc.mriq_v2_desc.numX = NUMX,
		.desc.mriq_v2_desc.numK = NUMK,
		.desc.mriq_v2_desc.src_offset = 0,
		.desc.mriq_v2_desc.dst_offset = 0,
		.desc.mriq_v2_desc.esp.coherence = ACC_COH_NONE,
		.desc.mriq_v2_desc.esp.p2p_store = 0,
		.desc.mriq_v2_desc.esp.p2p_nsrcs = 0,
		.desc.mriq_v2_desc.esp.p2p_srcs = {"", "", "", ""},
	}
};

#endif /* __ESP_CFG_000_H__ */
