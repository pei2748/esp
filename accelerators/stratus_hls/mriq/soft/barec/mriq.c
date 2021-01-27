/* Copyright (c) 2011-2019 Columbia University, System Level Design Group */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __riscv
#include <stdio.h>
#include <stdlib.h>
#endif
#include "math.h"
#include <esp_accelerator.h>
#include <esp_probe.h>

typedef int32_t token_t;

static unsigned DMA_WORD_PER_BEAT(unsigned _st)
{
        return (sizeof(void *) / _st);
}


#define SLD_MRIQ 0x001
#define DEV_NAME "sld,mriq"

/* <<--params-->> */
//const int32_t numX = 32768;
const int32_t numX = 4;
// add 4 more parameters.
const int32_t numK = 16;
const int32_t batch_size_x = 128;
const int32_t batch_size_k = 16;
const int32_t num_batch_x = 1;
const int32_t num_batch_k = 1;




// 11 variables for length.

static unsigned in_words_adj;
static unsigned in_k_words_adj;
static unsigned out_words_adj;

static unsigned in_len;
static unsigned in_k_len;
static unsigned out_len;

static unsigned in_size;
static unsigned in_k_size;	
static unsigned out_size;

static unsigned out_offset;
static unsigned mem_size;


/* Size of the contiguous chunks for scatter/gather */
#define CHUNK_SHIFT 20
#define CHUNK_SIZE BIT(CHUNK_SHIFT)
#define NCHUNK(_sz) ((_sz % CHUNK_SIZE == 0) ?		\
			(_sz / CHUNK_SIZE) :		\
			(_sz / CHUNK_SIZE) + 1)

/* User defined registers */
/* <<--regs-->> */

//#define MRIQ_NUMX_REG  0x54
//#define MRIQ_NUMK_REG  0x50
//#define MRIQ_BATCH_SIZE_X_REG  0x4C
//#define MRIQ_BATCH_SIZE_K_REG  0x48
//#define MRIQ_NUM_BATCH_X_REG   0x44
//#define MRIQ_NUM_BATCH_K_REG   0x40
//
//
#define MRIQ_NUMX_REG  0x40
#define MRIQ_NUMK_REG  0x44
#define MRIQ_BATCH_SIZE_X_REG  0x48
#define MRIQ_BATCH_SIZE_K_REG  0x4C
#define MRIQ_NUM_BATCH_X_REG   0x50
#define MRIQ_NUM_BATCH_K_REG   0x54



int main(int argc, char * argv[])
{
	int i;
	int n;
	int ndev;
	struct esp_device *espdevs;
	struct esp_device *dev;
	unsigned done;
	unsigned **ptable;
	token_t *mem;
	token_t gold[2*numX];
	unsigned errors = 0;
	
	float diff;
	float error_thresh = 0.1;

	if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
		in_words_adj      = numX;
		in_k_words_adj    = numK;
		out_words_adj     = numX;
	      
	} else {
		in_words_adj   = round_up(numX, DMA_WORD_PER_BEAT(sizeof(token_t)));
		in_k_words_adj = round_up(numK, DMA_WORD_PER_BEAT(sizeof(token_t)));
		out_words_adj  = round_up(numX, DMA_WORD_PER_BEAT(sizeof(token_t)));

	}

	in_len      = in_words_adj      * (1);
	in_k_len    = in_k_words_adj    * (1);
	out_len     = out_words_adj     * (1);

	in_size   = in_len   * sizeof(token_t);
	in_k_size = in_k_len * sizeof(token_t);
	out_size  = out_len  * sizeof(token_t);

	out_offset = 3 * in_len + 5 * in_k_len;
	mem_size = 3 * in_size + 5 * in_k_size + 2 * out_size;


	// Search for the device
#ifndef __riscv
	printf("Scanning device tree... \n");
#else
	print_uart("Scanning device tree... \n");
#endif

	ndev = probe(&espdevs, SLD_MRIQ, DEV_NAME);
	if (ndev == 0) {
#ifndef __riscv
		printf("mriq not found\n");
#else
		print_uart("mriq not found\n");
#endif
		return 0;
	}

	for (n = 0; n < ndev; n++) {

		dev = &espdevs[n];

		// Check DMA capabilities
		if (ioread32(dev, PT_NCHUNK_MAX_REG) == 0) {
#ifndef __riscv
			printf("  -> scatter-gather DMA is disabled. Abort.\n");
#else
			print_uart("  -> scatter-gather DMA is disabled. Abort.\n");
#endif
			return 0;
		}

		if (ioread32(dev, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size)) {
#ifndef __riscv
			printf("  -> Not enough TLB entries available. Abort.\n");
#else
			print_uart("  -> Not enough TLB entries available. Abort.\n");
#endif
			return 0;
		}

		// Allocate memory
		mem = aligned_malloc(mem_size);


#ifndef __riscv
		printf("  memory buffer base-address = %p\n", mem);
#else
		print_uart("  memory buffer base-address = "); print_uart_addr((uintptr_t) mem); print_uart("\n");
#endif

		// Alocate and populate page table
		ptable = aligned_malloc(NCHUNK(mem_size) * sizeof(unsigned *));
		for (i = 0; i < NCHUNK(mem_size); i++)
			ptable[i] = (unsigned *) &mem[i * (CHUNK_SIZE / sizeof(token_t))];
#ifndef __riscv
		printf("  ptable = %p\n", ptable);
		printf("  nchunk = %lu\n", NCHUNK(mem_size));
#else
		print_uart("  ptable = "); print_uart_addr((uintptr_t) ptable); print_uart("\n");
		print_uart("  nchunk = "); print_uart_int(NCHUNK(mem_size)); print_uart("\n");
#endif

#ifndef __riscv
		printf("  Generate input...\n");
#else
		print_uart("  Generate input...\n");
#endif

		//If test for the WL = 24, include test_in_WL24.h, if WL = 32, include test_in_WL32.h

                #include "test_in_WL32.h"
		//                #include "test_in_WL24.h"

		// mem is all the memory space needed for store input data? 


		// Pass common configuration parameters

		iowrite32(dev, SELECT_REG, ioread32(dev, DEVID_REG));
		iowrite32(dev, COHERENCE_REG, ACC_COH_NONE);

#ifndef __sparc
		iowrite32(dev, PT_ADDRESS_REG, (unsigned long long) ptable);
#else
		iowrite32(dev, PT_ADDRESS_REG, (unsigned) ptable);
#endif
		iowrite32(dev, PT_NCHUNK_REG, NCHUNK(mem_size));
		iowrite32(dev, PT_SHIFT_REG, CHUNK_SHIFT);

		// Use the following if input and output data are not allocated at the default offsets
		iowrite32(dev, SRC_OFFSET_REG, 0x0);
		iowrite32(dev, DST_OFFSET_REG, 0x0);



		// Pass accelerator-specific configuration parameters
		/* <<--regs-config-->> */
		iowrite32(dev, MRIQ_NUMX_REG, numX);
		iowrite32(dev, MRIQ_NUMK_REG, numK);
		iowrite32(dev, MRIQ_BATCH_SIZE_X_REG, batch_size_x);
		iowrite32(dev, MRIQ_BATCH_SIZE_K_REG, batch_size_k);
		iowrite32(dev, MRIQ_NUM_BATCH_X_REG, num_batch_x);
		iowrite32(dev, MRIQ_NUM_BATCH_K_REG, num_batch_k);

		// Flush (customize coherence model here)
		esp_flush(ACC_COH_NONE);

		// Start accelerators
#ifndef __riscv
		printf("  Start...\n");
#else
		print_uart("  Start...\n");
#endif
		iowrite32(dev, CMD_REG, CMD_MASK_START);

		// Wait for completion
		done = 0;
		while (!done) {
			done = ioread32(dev, STATUS_REG);
			done &= STATUS_MASK_DONE;
		}
		iowrite32(dev, CMD_REG, 0x0);

#ifndef __riscv
		printf("  Done\n");
		printf("  validating...\n");
#else
		print_uart("  Done\n");
		print_uart("  validating...\n");
#endif

		/* Validation */
		//errors = validate_buf(&mem[out_offset], &gold);
		int j;
		for (j = 0; j < 2*numX; j++)
		  {


		    diff = abs((mem[out_offset + j] - gold[j])/gold[j]);

#ifndef __riscv
                    printf("out = %ld ; ", mem[out_offset + j]);
                    printf("gold = %ld \n", gold[j]);
#else
		    print_uart("out =  ");      print_uart_int(mem[out_offset + j]);
		    print_uart("; gold =  ");   print_uart_int(gold[j]); print_uart("\n"); 
#endif
		    if (diff > error_thresh)
		      errors++;         
		  }



#ifndef __riscv
		if (errors)
			printf("  ... FAIL\n");
		else
			printf("  ... PASS\n");
#else
		if (errors)
			print_uart("  ... FAIL\n");
		else
			print_uart("  ... PASS\n");
#endif

		aligned_free(ptable);
		aligned_free(mem);
		//		aligned_free(gold);
	}

	return 0;
}
