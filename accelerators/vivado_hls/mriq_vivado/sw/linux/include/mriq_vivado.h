// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _MRIQ_VIVADO_H_
#define _MRIQ_VIVADO_H_

#ifdef __KERNEL__
#include <linux/ioctl.h>
#include <linux/types.h>
#else
#include <sys/ioctl.h>
#include <stdint.h>
#ifndef __user
#define __user
#endif
#endif /* __KERNEL__ */

#include <esp.h>
#include <esp_accelerator.h>

struct mriq_vivado_access {
	struct esp_access esp;
	/* <<--regs-->> */
	unsigned numX;
	unsigned numK;
	unsigned src_offset;
	unsigned dst_offset;
};

#define MRIQ_VIVADO_IOC_ACCESS	_IOW ('S', 0, struct mriq_vivado_access)

#endif /* _MRIQ_VIVADO_H_ */
