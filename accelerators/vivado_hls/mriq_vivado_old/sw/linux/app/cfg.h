// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "mriq_vivado.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define NUMX 4
#define NUMK 16

/* <<--params-->> */
const int32_t numX = NUMX;
const int32_t numK = NUMK;

#define NACC 1

struct mriq_vivado_access mriq_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.numX = NUMX,
		.numK = NUMK,
		.src_offset = 0,
		.dst_offset = 0,
		.esp.coherence = ACC_COH_NONE,
		.esp.p2p_store = 0,
		.esp.p2p_nsrcs = 0,
		.esp.p2p_srcs = {"", "", "", ""},
	}
};

esp_thread_info_t cfg_000[] = {
	{
		.run = true,
		.devname = "mriq_vivado.0",
		.ioctl_req = MRIQ_VIVADO_IOC_ACCESS,
		.esp_desc = &(mriq_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */
