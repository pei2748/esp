// Copyright (c) 2011-2019 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <stdlib.h>
#include <sstream>
#include <malloc.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <fstream>
#include <iostream>
#include "system.hpp"
#include "fpdata.hpp"

std::ofstream ofs;

// Process
///*


// In load_one_var function, we print out each data in int type and storing them in a file, whose name is related to the testbench.

void system_t::load_one_var(float *in, int offset, int32_t base_index, int32_t size)
{
  FPDATA_S data_fp;
  sc_dt::sc_bv<FPDATA_S_WL> data_bv;
  int16_t mem_i = 0;
  // base_index is the base for mem[].
  // offset is the offset for one variable.
  ESP_REPORT_INFO("loading one variable");

  ofstream outfile;
  outfile.open(esc_argv()[3], ofstream::app);

  FPDATA_S_WORD data_int;

  int mem_idx = base_index*2  + mem_i;

  for(int16_t i=0; i < size; i+=DMA_WORD_PER_BEAT)
    {
      for (int k = 0; k < DMA_WORD_PER_BEAT; k++)
	{
	  data_fp = FPDATA_S(in[offset + i + k]);
	  fp2bv<FPDATA_S, FPDATA_S_WL>(data_fp, data_bv);
	  mem[base_index + mem_i].range((k+1) * FPDATA_S_WL - 1, k * FPDATA_S_WL) = data_bv;
	  data_int = data_bv.to_int64();
	  outfile << "mem["<< mem_idx  <<"] = "<< data_int <<";\n";
	  mem_idx += 1;
	}  
      mem_i++;
    }

    outfile.close();
}



//*/
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
        config.batch_size_x = batch_size_x;
        config.batch_size_k = batch_size_k;
        config.num_batch_x = num_batch_x;
        config.num_batch_k = num_batch_k;

        wait();
	conf_info.write(config);
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
        if (validate() > 0.001)
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


    in_words_adj = round_up(numX, DMA_WORD_PER_BEAT);
    in_k_words_adj = round_up(numK, DMA_WORD_PER_BEAT);

    out_words_adj = round_up(numX, DMA_WORD_PER_BEAT);


    in_size = in_words_adj * (1);
    in_k_size = in_k_words_adj * (1);
    out_size = out_words_adj * (1);
    int numX_bm, numK_bm;
    float *kx, *ky, *kz;          /* K trajectory (3D vectors) */
    float *x, *y, *z;             /* X coordinates (3D vectors) */
    float *phiR, *phiI;           /* Phi values (complex) */

    // Load all the data in the file. But only transfer a part to accelerator.

    ESP_REPORT_INFO("esc_argv %s", esc_argv()[1]);

    inputData(esc_argv()[1], &numK_bm, &numX_bm,
	      &kx, &ky, &kz,
	      &x,  &y,  &z,
	      &phiR, &phiI);

    ESP_REPORT_INFO("Finish reading input data");
    // Memory initialization:
    //  int32_t addr_offset = 0;

    ofstream outfile;
    outfile.open(esc_argv()[3]);
      
    int16_t mem_base = 0;

    //in_size is the same as numX;
    //in_k_size is the same as numK;
    
    //    int batch_size_x = 128;

    int x_offset = 0;
    int k_offset = 0;





    ESP_REPORT_INFO("num_batch_k = %d", num_batch_k);



    // load all the K variables.
    if(numX < batch_size_x)
	batch_size_x = numX;
    if(numK < batch_size_k)
	batch_size_k = numK;
      


    for(int i = 0; i < num_batch_k; i++)
      {
	ESP_REPORT_INFO("loading kx");
	load_one_var(kx, k_offset, mem_base, batch_size_k);
	mem_base += batch_size_k/DMA_WORD_PER_BEAT;

	ESP_REPORT_INFO("loading ky");

	load_one_var(ky, k_offset, mem_base, batch_size_k);
	mem_base += batch_size_k/DMA_WORD_PER_BEAT;

	ESP_REPORT_INFO("loading kz");

	load_one_var(kz,  k_offset, mem_base, batch_size_k);
	mem_base += batch_size_k/DMA_WORD_PER_BEAT;

	ESP_REPORT_INFO("loading phiR");

	load_one_var(phiR, k_offset, mem_base, batch_size_k);
	mem_base += batch_size_k/DMA_WORD_PER_BEAT;

	ESP_REPORT_INFO("loading phiI");

	load_one_var(phiI, k_offset, mem_base, batch_size_k);
	mem_base += batch_size_k/DMA_WORD_PER_BEAT;

	k_offset += batch_size_k;
      }





    // load the rest x variables.

    for(int j = 0; j < num_batch_x; j++)
      {
	ESP_REPORT_INFO("loading x");

	load_one_var(x, x_offset, mem_base, batch_size_x);
	mem_base += batch_size_x/DMA_WORD_PER_BEAT;

	ESP_REPORT_INFO("loading y");

	load_one_var(y, x_offset, mem_base, batch_size_x);
	mem_base += batch_size_x/DMA_WORD_PER_BEAT;

	ESP_REPORT_INFO("loading z");

	load_one_var(z, x_offset, mem_base, batch_size_x);
	mem_base += batch_size_x/DMA_WORD_PER_BEAT;
	
	x_offset += batch_size_x;
       
      }


}


void system_t::dump_memory()
{

    // Get results from memory
    out_Qr = new float[out_size];
    out_Qi = new float[out_size];
    
    sc_dt::sc_bv<FPDATA_L_WL> data_bv;	   

    int32_t offset_Qr = 3*in_size + 5* in_k_size;
    int32_t offset = offset_Qr / DMA_WORD_PER_BEAT;

    if(numX < batch_size_x)
      {
	batch_size_x = numX;
      }


    for(int r = 0; r < num_batch_x; r++)
      {
	for (int i = 0; i < batch_size_x / DMA_WORD_PER_BEAT; i++)
	  {    
	    for(int j = 0; j < DMA_WORD_PER_BEAT; j++)
	      {
		int32_t index = r * batch_size_x + DMA_WORD_PER_BEAT * i + j;
		data_bv = mem[offset + i].range((j + 1) * FPDATA_L_WL - 1, j * FPDATA_L_WL);
		FPDATA_L data_fp = bv2fp<FPDATA_L, FPDATA_L_WL>(data_bv);
		out_Qr[index] = (float) data_fp;  
      		cout << "out_Qr = " << data_fp; 
	      }
	  }
	offset += batch_size_x / DMA_WORD_PER_BEAT;
	//
	for (int i = 0; i < batch_size_x / DMA_WORD_PER_BEAT; i++)
	  {    
	    for (int j = 0; j < DMA_WORD_PER_BEAT; j++)
	      {
		int32_t index = r * batch_size_x + DMA_WORD_PER_BEAT * i + j;
		data_bv = mem[offset + i].range((j + 1) * FPDATA_L_WL - 1, j * FPDATA_L_WL);
		FPDATA_L data_fp = bv2fp<FPDATA_L, FPDATA_L_WL>(data_bv);
		out_Qi[index] = (float) data_fp;
		cout << "out_Qi = " << data_fp; 

	      }
	  }
	offset += batch_size_x / DMA_WORD_PER_BEAT;
      }

    ESP_REPORT_INFO("dump memory completed");
}




int system_t::validate()
{
  // Check for mismatches
  int32_t errors = 0;
  float error_threshold = 0.05;

  float diff_r;

  float diff_i;

  float *gold_Qr, *gold_Qi;               /* Q signal (complex) */

  gold_Qr = new float [numX];
  gold_Qi = new float [numX];


  int numX_bm;
  outputData(esc_argv()[2], &gold_Qr, &gold_Qi, &numX_bm);




  ofstream outfile;
  outfile.open(esc_argv()[3], ofstream::app);


  FPDATA_L data_fp;
  sc_dt::sc_bv<FPDATA_L_WL> data_bv;  
  FPDATA_L_WORD data_int;
  int count = 0;


  // Compute golden output


  ESP_REPORT_INFO("Did I ever enter validate?");
  ESP_REPORT_INFO("numX = %d, numK = %d \n", numX, numK);
  for(int i = 0; i < 2; i++)
    {
	switch(i){
	case 0: 

	  for (int j = 0; j < numX; j++)
	    {
                  diff_r = fabs((gold_Qr[j] - out_Qr[j])/gold_Qr[j]);
                  if (diff_r > error_threshold)
                    {
		      errors++;
		      ESP_REPORT_INFO("An error occurs to Qr, i = %d", j);
		      ESP_REPORT_INFO("gold = %f, out = %f ",gold_Qr[j], out_Qr[j]);
		    }
		  // The following 5 lines is for input of barec testing.
		  data_fp = FPDATA_L(gold_Qr[j]);
		  fp2bv<FPDATA_L, FPDATA_L_WL>(data_fp, data_bv);	    
		  data_int = data_bv.to_int64();
		  outfile << "gold["<< count <<"] = "<< data_int <<";\n";
		  count += 1;		  

		  //		  ESP_REPORT_INFO("Qr = %d ;\n", data_bv);


	    }
	  break;
	case 1:

	  for (int j = 0; j < numX; j++)
	    {
                  diff_i = fabs((gold_Qi[j] - out_Qi[j])/gold_Qi[j]);

                  if (diff_i > error_threshold)
                    {
		      errors++;
		      ESP_REPORT_INFO("An error occurs to Qi, i = %d", j);
		      ESP_REPORT_INFO("gold = %f, out = %f ",gold_Qi[j], out_Qi[j]);

		    }

		  data_fp = FPDATA_L(gold_Qi[j]);
		  fp2bv<FPDATA_L, FPDATA_L_WL>(data_fp, data_bv);	    
		  data_int = data_bv.to_int64();
		  outfile << "gold["<< count <<"] = "<< data_int <<";\n";
		  count += 1;

		  //		  ESP_REPORT_INFO("Qi = %d ;\n", data_bv);


	    }
	  break;
	default:
	  break;
	}
	
    }

    outfile.close();

    delete [] x;
    delete [] y;
    delete [] z;
    delete [] kx;
    delete [] ky;
    delete [] kz;
    delete [] phiR;
    delete [] phiI;
    delete [] gold_Qr;
    delete [] gold_Qi;
    delete [] out_Qr;
    delete [] out_Qi;
    ESP_REPORT_INFO("\n Error number is %d", errors);

    float error_rate;

    error_rate = (float) errors/(2*numX);

    ESP_REPORT_INFO("\n Error rate is %f", error_rate);

    //////////// to convert some data into int type /////////////
    /*
    float some[8];
    some[0] = -0.238735;
    some[1] = -0.240356;
    some[2] = -0.253621;
    some[3] = -0.250953;
    some[4] = -0.109612;
    some[5] = -0.106009;
    some[6] = -0.0684509;
    some[7] = 0.0776644;



    FPDATA_S data_1_fp;
    FPDATA_S_WORD data_1_int;
    sc_dt::sc_bv<FPDATA_S_WL> data_1_bv;

    for(int16_t i = 0; i < 8; i++)
      {
	data_1_fp = FPDATA_S(some[i]);
	fp2bv<FPDATA_S, FPDATA_S_WL>(data_1_fp, data_1_bv);
	data_1_int = data_1_bv.to_int64();
	cout << "int["<< i << "] = " << data_1_int << ";\n";

      }

*/
    ////////////////////////////////////////////////////////////


    return error_rate;
}


void system_t::inputData(const char *file, int* _numK, int* _numX,
               float** kx, float** ky, float** kz,
               float** x, float** y, float** z,
               float** phiR, float** phiI)
{
  int numK, numX;
  FILE* fid = NULL;

  if((fid = fopen(file, "r")) == (FILE*)NULL)
    {
      ESP_REPORT_ERROR("[Err] could not open %s", inputData_path.c_str());
      fclose(fid);
    }

  fread (&numK, sizeof (int), 1, fid);
  *_numK = numK;
  fread (&numX, sizeof (int), 1, fid);
  *_numX = numX;
  *kx = (float *) memalign(16, numK * sizeof (float));
  fread (*kx, sizeof (float), numK, fid);
  *ky = (float *) memalign(16, numK * sizeof (float));
  fread (*ky, sizeof (float), numK, fid);
  *kz = (float *) memalign(16, numK * sizeof (float));
  fread (*kz, sizeof (float), numK, fid);
  *x = (float *) memalign(16, numX * sizeof (float));
  fread (*x, sizeof (float), numX, fid);
  *y = (float *) memalign(16, numX * sizeof (float));
  fread (*y, sizeof (float), numX, fid);
  *z = (float *) memalign(16, numX * sizeof (float));
  fread (*z, sizeof (float), numX, fid);
  *phiR = (float *) memalign(16, numK * sizeof (float));
  fread (*phiR, sizeof (float), numK, fid);
  *phiI = (float *) memalign(16, numK * sizeof (float));
  fread (*phiI, sizeof (float), numK, fid);
  fclose (fid); 
}

///*
void system_t::outputData(const char *file, float** outR, float** outI, int* _numX)
{
  int numX;
  FILE* fid = NULL;
  fid = fopen(file, "r");

  if (fid == (FILE*)NULL)
    {
      ESP_REPORT_ERROR("[Err] could not open %s", outputData_path.c_str());
      fclose(fid);
    }


  fread(&numX, sizeof(int), 1, fid);
  *_numX = numX;


  *outR = (float *) memalign(16, numX * sizeof (float));
  fread(*outR, sizeof(float), numX, fid);

  *outI = (float *) memalign(16, numX * sizeof (float));
  fread(*outI, sizeof(float), numX, fid);
  fclose (fid);
}

//*/

void system_t::outputDataTest(const char *file, float outR[], float outI[], int numX)
{
  FILE *fid;
  fid=fopen(file, "r");
  float myvariable;
  
  for (int i = 0; i < numX; i++)
    {
      fscanf(fid, "%f", &myvariable);
      outR[i] = myvariable;
    }
  for (int i = 0; i < numX; i++)
    {
      fscanf(fid, "%f", &myvariable);
      outI[i] = myvariable;
    }
  fclose (fid);
}
