// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include <sstream>
#include "system.hpp"
#include "helper.h" // for two functions to read data from files



// helper functions

void system_t::load_one_var(float *in, int offset, int32_t base_index, int32_t size)
{
  FPDATA_S data_fp;
  sc_dt::sc_bv<FPDATA_S_WL> data_bv;

  int16_t mem_i = 0;

  // base_index is the base for mem[].
  // offset is the offset for one variable.
  // use this file to store baremetal application.

  int mem_idx = base_index * 2  + mem_i;

  for(int16_t i=0; i < size; i+=DMA_WORD_PER_BEAT) {
       for (int k = 0; k < DMA_WORD_PER_BEAT; k++) {
	  // convert floating to fx 
          data_fp = FPDATA_S(in[offset + i + k]);

	  // convert fx to bv
	  fp2bv<FPDATA_S, FPDATA_S_WL>(data_fp, data_bv);


	  mem[base_index + mem_i].range((k+1) * FPDATA_S_WL - 1, k * FPDATA_S_WL) = data_bv;

	  outfile << "in["<< mem_idx  <<"] = "<< data_fp <<";\n";
	  mem_idx += 1;
       }  
       mem_i++;
  }

}



// Process
void system_t::config_proc()
{

    // Reset
    {
        conf_done.write(false);
        conf_info.write(conf_info_t());
        wait();
    }

    ESP_REPORT_INFO("reset done");

    // Config
    load_memory();
    {
        conf_info_t config;
        // Custom configuration
        /* <<--params-->> */
        config.numX = numX;
        config.numK = numK;
        config.num_batch_k = num_batch_k;
        config.batch_size_k = batch_size_k;
        config.num_batch_x = num_batch_x;
        config.batch_size_x = batch_size_x;

        wait(); conf_info.write(config);
        conf_done.write(true);
    }

    ESP_REPORT_INFO("config done");

    // Compute
    {
        // Print information about begin time
        sc_time begin_time = sc_time_stamp();
        ESP_REPORT_TIME(begin_time, "BEGIN - mriq");

        // Wait the termination of the accelerator
        do { wait(); } while (!acc_done.read());
        debug_info_t debug_code = debug.read();

        // Print information about end time
        sc_time end_time = sc_time_stamp();
        ESP_REPORT_TIME(end_time, "END - mriq");

        esc_log_latency(sc_object::basename(), clock_cycle(end_time - begin_time));
        wait(); conf_done.write(false);
    }

    // Validate
    {
        dump_memory(); // store the output in more suitable data structure if needed
        // check the results with the golden model
        if (validate())
        {
            ESP_REPORT_ERROR("validation failed!");
        } else
        {
            ESP_REPORT_INFO("validation passed!");
        }
    }

    // Conclude
    {
        sc_stop();
    }
}

// Functions
void system_t::load_memory()
{
    // Optional usage check
#ifdef CADENCE
    if (esc_argc() != 4)
    {
        ESP_REPORT_INFO("usage: %s\n", esc_argv()[0]);
        sc_stop();
    }
#endif

    // Input data and golden output (aligned to DMA_WIDTH makes your life easier)


#if (DMA_WORD_PER_BEAT == 0)
    in_words_adj = 3*numX+5*numK;
    out_half_words_adj = numX;
#else
    in_words_adj = round_up(3*numX+5*numK, DMA_WORD_PER_BEAT);
    out_half_words_adj = round_up(numX, DMA_WORD_PER_BEAT);
#endif
    in_size = in_words_adj * (1);
    out_half_size = out_half_words_adj * (1);

    // input and memory initialization
    int numX_bm, numK_bm;         // store the numX and numK read from files.
    float *kx, *ky, *kz;         
    float *x, *y, *z;            
    float *phiR, *phiI;          

    // Load all the data in the file. But only transfer a part to accelerator.
    ESP_REPORT_INFO("esc_argv %s", esc_argv()[1]);

    inputData(esc_argv()[1], &numK_bm, &numX_bm,
	      &kx, &ky, &kz,
	      &x,  &y,  &z,
	      &phiR, &phiI);

    int16_t mem_base = 0;
    int x_offset = 0;
    int k_offset = 0;

    if(numX < batch_size_x)
      batch_size_x = numX;
    if(numK < batch_size_k)
      batch_size_k = numK;

    // outfile is declared in system.hpp and opened here.
    outfile.open(esc_argv()[3], ofstream::ate);

    for(int i = 0; i < num_batch_k; i++) {
	//loading kx;
	load_one_var(kx, k_offset, mem_base, batch_size_k);
	mem_base += batch_size_k/DMA_WORD_PER_BEAT;

	//loading ky;
	load_one_var(ky, k_offset, mem_base, batch_size_k);
	mem_base += batch_size_k/DMA_WORD_PER_BEAT;

	//loading kz;
	load_one_var(kz,  k_offset, mem_base, batch_size_k);
	mem_base += batch_size_k/DMA_WORD_PER_BEAT;

	//loading phiR;
	load_one_var(phiR, k_offset, mem_base, batch_size_k);
	mem_base += batch_size_k/DMA_WORD_PER_BEAT;

	//loading phiI;
	load_one_var(phiI, k_offset, mem_base, batch_size_k);
	mem_base += batch_size_k/DMA_WORD_PER_BEAT;

	k_offset += batch_size_k;
    }

    // load the rest x variables.
    for(int j = 0; j < num_batch_x; j++) {
	//loading x
	load_one_var(x, x_offset, mem_base, batch_size_x);
	mem_base += batch_size_x/DMA_WORD_PER_BEAT;

	//loading y
	load_one_var(y, x_offset, mem_base, batch_size_x);
	mem_base += batch_size_x/DMA_WORD_PER_BEAT;

	//loading z
	load_one_var(z, x_offset, mem_base, batch_size_x);
	mem_base += batch_size_x/DMA_WORD_PER_BEAT;
	
	x_offset += batch_size_x;       
    }

    // Compute golden output

    gold_Qr = new float [numX];
    gold_Qi = new float [numX];

    outputData(esc_argv()[2], &gold_Qr, &gold_Qi, &numX_bm);

    free(x);
    free(y);
    free(z);
    free(kx);
    free(ky);
    free(kz);
    free(phiR);
    free(phiI);


    ESP_REPORT_INFO("load memory completed");
}


void system_t::dump_memory()
{
    // Get results from memory
    out_Qr = new float[out_half_size];
    out_Qi = new float[out_half_size];

    uint32_t offset = in_size;
    offset = offset / DMA_WORD_PER_BEAT;

    if(numX < batch_size_x)
	batch_size_x = numX;

    sc_dt::sc_bv<FPDATA_L_WL> data_bv;   
      
    for(int r = 0; r < num_batch_x; r++) {

	for (int i = 0; i < batch_size_x / DMA_WORD_PER_BEAT; i++) {    
	    for(int j = 0; j < DMA_WORD_PER_BEAT; j++) {
		int32_t index = r * batch_size_x + i * DMA_WORD_PER_BEAT + j;
		data_bv = mem[offset + i].range((j + 1) * FPDATA_L_WL - 1, j * FPDATA_L_WL);
		FPDATA_L data_fp = bv2fp<FPDATA_L, FPDATA_L_WL>(data_bv);
		out_Qr[index] = (float) data_fp;  
	    }
	}
	offset += batch_size_x / DMA_WORD_PER_BEAT;
	//
	for (int i = 0; i < batch_size_x / DMA_WORD_PER_BEAT; i++) {    
	    for (int j = 0; j < DMA_WORD_PER_BEAT; j++) {
		int32_t index = r * batch_size_x + i * DMA_WORD_PER_BEAT + j;
		data_bv = mem[offset + i].range((j + 1) * FPDATA_L_WL - 1, j * FPDATA_L_WL);
		FPDATA_L data_fp = bv2fp<FPDATA_L, FPDATA_L_WL>(data_bv);
		out_Qi[index] = (float) data_fp;
		
	    }
	}
	offset += batch_size_x / DMA_WORD_PER_BEAT;
    }
    ESP_REPORT_INFO("dump memory completed");
}

int system_t::validate()
{
    // Check for mismatches
    uint32_t errors = 0;
    float error_threshold = 0.05;
    float diff, error_rate;

    FPDATA_L data_fp;
    sc_dt::sc_bv<FPDATA_L_WL> data_bv;  


    int count = 0;

    for (int j = 0; j < numX; j++) {
        if(!gold_Qr[j] && !out_Qr[j]) {
	   diff = 0;
	} else if(!gold_Qr[j]) {// gold_Qr = 0, can't do division 
	   diff = fabs((gold_Qr[j] - out_Qr[j])/out_Qr[j]);
	} else { // gold_Qr != 0
	   diff = fabs((gold_Qr[j] - out_Qr[j])/gold_Qr[j]);
	}

	if (diff > error_threshold)
	    errors++;

	// The following 3 lines is for input of barec testing.
	data_fp = FPDATA_L(gold_Qr[j]);
	outfile << "gold["<< count <<"] = "<< data_fp  <<";\n";
	count += 1;  
    }

    for (int j = 0; j < numX; j++) {
        if(!gold_Qi[j] && !out_Qi[j]) {
	   diff = 0;
	} else if(!gold_Qi[j]) {// gold_Qi = 0, can't do division 
	   diff = fabs((gold_Qi[j] - out_Qi[j])/out_Qi[j]);
	} else { // gold_Qi != 0
	   diff = fabs((gold_Qi[j] - out_Qi[j])/gold_Qi[j]);
	}

	if (diff > error_threshold)
	    errors++;

	// The following 3 lines is for input of barec testing.
	data_fp = FPDATA_L(gold_Qi[j]);
	outfile << "gold["<< count <<"] = "<< data_fp  <<";\n";
	count += 1;  
    }


    delete [] out_Qr;
    delete [] out_Qi;

    delete [] gold_Qr;
    delete [] gold_Qi;
    outfile.close(); //close the outfile here. only open once and close once

    return errors;
}
