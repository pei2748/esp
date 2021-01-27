// Copyright (c) 2011-2019 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include <iostream>
#include <string>

#include "mriq_conf_info.hpp"
#include "mriq_debug_info.hpp"
#include "mriq.hpp"
#include "mriq_directives.hpp"
#include "esp_templates.hpp"

#include "fpdata.hpp"
const size_t MEM_SIZE = (266144 * 3 + 2048 * 5)/(DMA_WIDTH/8);

#include "core/systems/esp_system.hpp"

#ifdef CADENCE
#include "mriq_wrap.h"
#endif

class system_t : public esp_system<DMA_WIDTH, MEM_SIZE>
{
public:

    // ACC instance
#ifdef CADENCE
    mriq_wrapper *acc;
#else
    mriq *acc;
#endif

    // Constructor
    SC_HAS_PROCESS(system_t);
    system_t(sc_module_name name/*,
	   std::string inputData_path,
	   std::string outputData_path,
           std::string outint_path*/)
        : esp_system<DMA_WIDTH, MEM_SIZE>(name)
	  /*	, inputData_path(inputData_path)
	, outputData_path(outputData_path)
	, outint_path(outint_path)*/
    {
        // ACC
#ifdef CADENCE
        acc = new mriq_wrapper("mriq_wrapper");
#else
        acc = new mriq("mriq_wrapper");
#endif
        // Binding ACC
        acc->clk(clk);
        acc->rst(acc_rst);
        acc->dma_read_ctrl(dma_read_ctrl);
        acc->dma_write_ctrl(dma_write_ctrl);
        acc->dma_read_chnl(dma_read_chnl);
        acc->dma_write_chnl(dma_write_chnl);
        acc->conf_info(conf_info);
        acc->conf_done(conf_done);
        acc->acc_done(acc_done);
        acc->debug(debug);

        /* <<--params-default-->> */
	//      	numX = 32768;
       	numX = 262144;
        batch_size_x = 1024;
        num_batch_x = 256; // numX / batch_size_x


	//     	numK = 3072;
	numK = 2048;
#if(ARCH==0)
	//        batch_size_k = 3072;
	batch_size_k = 2048;

        num_batch_k = 1; // numK / batch_size_k
#else
	//        batch_size_k = 1024;
        batch_size_k = 2048;
        num_batch_k =  numK / batch_size_k;

#endif	





   }

    // Processes

    // Configure accelerator
    void config_proc();

    // Load internal memory
    void load_memory();

    // Dump internal memory
    void dump_memory();

    // Validate accelerator results
    int validate();

    // Accelerator-specific data
    /* <<--params-->> */
    int32_t numX;
    int32_t numK;
    int32_t batch_size_x;
    int32_t batch_size_k;
    int32_t num_batch_x;
    int32_t num_batch_k;


    int32_t in_words_adj;
    int32_t out_words_adj;
    int32_t in_size;
    int32_t in_k_size;
    int32_t in_k_words_adj;
    int32_t out_size;
    FPDATA_S *x;
    FPDATA_S *y;
    FPDATA_S *z;

    FPDATA_S *kx;
    FPDATA_S *ky;
    FPDATA_S *kz;

    FPDATA_S *phiR;
    FPDATA_S *phiI;
    
    float *out_Qr;
    float *out_Qi;
    float *gold_Qr;
    float *gold_Qi;
    char *input_name;
    char *output_name;


    // Path for input:
    std::string inputData_path;
  
    // Path for golden output
    std::string outputData_path;

    // Path for integer type data used for barec
    std::string outint_path;

    // Other Functions

    void load_one_var(float *in, int offset, int32_t base_index, int32_t size);


    void inputData(const char *file, int* _numK, int* _numX,
               float** kx, float** ky, float** kz,
               float** x, float** y, float** z,
               float** phiR, float** phiI);

    
    void outputDataTest(const char *file, float outR[], float outI[], int numX);
    void outputData(const char *file, float** outR, float** outI, int* _numX);



};

#endif // __SYSTEM_HPP__
