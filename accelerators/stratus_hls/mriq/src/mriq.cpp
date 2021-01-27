
// Copyright (c) 2011-2019 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "mriq.hpp"
#include "mriq_directives.hpp"

// Functions

#include "mriq_functions.hpp"
#include "fpdata.hpp"

// Processes



void mriq::load_one_data(FPDATA_S_WORD array[],  uint32_t dma_addr, uint32_t len)
{
  dma_info_t dma_info(dma_addr, len/DMA_WORD_PER_BEAT, DMA_SIZE);
  this -> dma_read_ctrl.put(dma_info);

  for(uint16_t i = 0; i < len; i += DMA_WORD_PER_BEAT)
    {
      HLS_PROTO("load one data");

      HLS_BREAK_DEP(array);

      sc_dt::sc_bv<DMA_WIDTH> data = this->dma_read_chnl.get();
      wait();

      for(int k = 0; k < DMA_WORD_PER_BEAT; k++){
	HLS_UNROLL_SIMPLE;
	array[i + k] = data.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH).to_int64();
	//	wait();
      }
    }
}


void mriq::store_one_data(FPDATA_L_WORD array[], uint32_t dma_addr, uint32_t len)
{
  dma_info_t dma_info(dma_addr, len/DMA_WORD_PER_BEAT, DMA_SIZE);
  this -> dma_write_ctrl.put(dma_info);

  for(uint16_t i = 0; i < len; i += DMA_WORD_PER_BEAT)
    {
      HLS_BREAK_DEP(array);
      sc_dt::sc_bv<DMA_WIDTH> data;
      wait();      

      for(int k = 0; k < DMA_WORD_PER_BEAT; k++){
	HLS_UNROLL_SIMPLE;
	data.range((k+1) * DATA_WIDTH - 1, k * DATA_WIDTH) = array[i + k];
      }
      this->dma_write_chnl.put(data);
    }

}


void mriq::load_input()
{

    // Reset
    {
        HLS_PROTO("load-reset");

        this->reset_load_input();

        // explicit PLM ports reset if any

        // User-defined reset code

        wait();
    }

    // Config
    /* <<--params-->> */
    int32_t numX;
    int32_t numK;

    int32_t batch_size_k;
    int32_t batch_size_x;

    int32_t num_batch_x;
    int32_t num_batch_k;

    {
        HLS_PROTO("load-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        /* <<--local-params-->> */
        numX = config.numX;
        numK = config.numK;
        batch_size_x = config.batch_size_x;
        batch_size_k = config.batch_size_k;
        num_batch_x = config.num_batch_x;
        num_batch_k = config.num_batch_k;

    }

    // Load
    {
        HLS_PROTO("load-dma");
        wait();
	

        bool pingpong_x = true;
        int32_t dma_addr = 0;

	if(numX < batch_size_x)
	  {
	    batch_size_x = numX;
	  }
	if(numK < batch_size_k)
	  {
	    batch_size_k = numK;
	  }





#if(ARCH == 0)
	//	ESP_REPORT_INFO("load: come into if-SK part");

	for(int n = 0; n < num_batch_k; n++)
	  {
	    load_one_data(plm_kx, dma_addr, batch_size_k);    dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
	    load_one_data(plm_ky, dma_addr, batch_size_k);    dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
	    load_one_data(plm_kz, dma_addr, batch_size_k);    dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
	    load_one_data(plm_phiR, dma_addr, batch_size_k);    dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
	    load_one_data(plm_phiI, dma_addr, batch_size_k);    dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
	  }

        
	for(int l=0; l < num_batch_x; l++)
	  {
	    if(pingpong_x)
	      {
                load_one_data(plm_x_ping, dma_addr, batch_size_x);    dma_addr += batch_size_x/DMA_WORD_PER_BEAT;
                load_one_data(plm_y_ping, dma_addr, batch_size_x);    dma_addr += batch_size_x/DMA_WORD_PER_BEAT;
                load_one_data(plm_z_ping, dma_addr, batch_size_x);    dma_addr += batch_size_x/DMA_WORD_PER_BEAT;
	      }
	    else
	      {
                load_one_data(plm_x_pong, dma_addr, batch_size_x);    dma_addr += batch_size_x/DMA_WORD_PER_BEAT;
                load_one_data(plm_y_pong, dma_addr, batch_size_x);    dma_addr += batch_size_x/DMA_WORD_PER_BEAT;
                load_one_data(plm_z_pong, dma_addr, batch_size_x);    dma_addr += batch_size_x/DMA_WORD_PER_BEAT;
	      }

	    this->load_compute_handshake();
    	    pingpong_x = !pingpong_x;
	  }
#else
	//ESP_REPORT_INFO("load: come into else part");

        bool pingpong_k = true;
        int32_t total_loading =  num_batch_k * numX;    // total_loading = (numK/batch_size_k ) * numX
        int32_t dma_addr_x = ( 5 * numK )/DMA_WORD_PER_BEAT;
        int counter_k = num_batch_k;    // counter_k indicates when to reverse pingpong_k, = numK / batch_size_k;
        int counter_x_ini = num_batch_k * batch_size_x; //counter_x indicates when to reverse pingpong_x
        int counter_x = counter_x_ini;

	if(numX < batch_size_x)
	  {
	    batch_size_x = numX;
	  }

        for(int l=0; l < total_loading; l++)
          {  // in if statement, counter_x == XXX, this number is the initialized value.
            if(counter_x == counter_x_ini  && pingpong_x == true)
              {
                load_one_data(plm_x_ping, dma_addr_x, batch_size_x);    dma_addr_x += batch_size_x/DMA_WORD_PER_BEAT;
                load_one_data(plm_y_ping, dma_addr_x, batch_size_x);    dma_addr_x += batch_size_x/DMA_WORD_PER_BEAT;
                load_one_data(plm_z_ping, dma_addr_x, batch_size_x);    dma_addr_x += batch_size_x/DMA_WORD_PER_BEAT;
                pingpong_x = !pingpong_x;
              }
            else if(counter_x == 0 && pingpong_x == false)
              {
                load_one_data(plm_x_pong, dma_addr_x, batch_size_x);    dma_addr_x += batch_size_x/DMA_WORD_PER_BEAT;
                load_one_data(plm_y_pong, dma_addr_x, batch_size_x);    dma_addr_x += batch_size_x/DMA_WORD_PER_BEAT;
                load_one_data(plm_z_pong, dma_addr_x, batch_size_x);    dma_addr_x += batch_size_x/DMA_WORD_PER_BEAT;
                counter_x = counter_x_ini << 1; // counter_x = initial value * 2;
                pingpong_x = !pingpong_x;
              }
            if(pingpong_k)
              {
	      	//HLS_PROTO("load ping");
                load_one_data(plm_kx_ping,  dma_addr, batch_size_k);   dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
                load_one_data(plm_ky_ping,  dma_addr, batch_size_k);   dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
                load_one_data(plm_kz_ping,  dma_addr, batch_size_k);   dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
                load_one_data(plm_phiR_ping,  dma_addr, batch_size_k); dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
                load_one_data(plm_phiI_ping,  dma_addr, batch_size_k); dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
                counter_x -= 1;  counter_k -= 1;
		//      		wait();
              }else{
	      //      	      HLS_PROTO("load pong");// wait();
	      load_one_data(plm_kx_pong,  dma_addr, batch_size_k);   dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
	      load_one_data(plm_ky_pong,  dma_addr, batch_size_k);   dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
	      load_one_data(plm_kz_pong,  dma_addr, batch_size_k);   dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
	      load_one_data(plm_phiR_pong,  dma_addr, batch_size_k); dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
	      load_one_data(plm_phiI_pong,  dma_addr, batch_size_k); dma_addr += batch_size_k/DMA_WORD_PER_BEAT;
	      counter_x -= 1;  counter_k -= 1;
	      //       	      wait();
	    }
            if(counter_k == 0)
              { // Time to reverse pingpong_k
		//                pingpong_k = !pingpong_k;
                dma_addr = 0;
	        counter_k = num_batch_k;
              }
    	    pingpong_k = !pingpong_k;

	    this->load_compute_handshake();
	    //	    ESP_REPORT_INFO("Is loading finished?!");
	  } // end of for-total_loading
	
#endif
	// Conclude
	{
	   this->process_done();
	}
    }
}


void mriq::store_output()
{
    // Reset
    {
        HLS_PROTO("store-reset");

        this->reset_store_output();

        // explicit PLM ports reset if any

        // User-defined reset code

        wait();
    }

    // Config
    /* <<--params-->> */
    int32_t numX;
    int32_t numK;
    int32_t batch_size_x;
    int32_t num_batch_x;

    {
        HLS_PROTO("store-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        /* <<--local-params-->> */
        numX = config.numX;
        numK = config.numK;
        batch_size_x = config.batch_size_x;
        num_batch_x = config.num_batch_x;


    }

    // Store
    {
        HLS_PROTO("store-dma");
        wait();
        uint32_t offset = (3 * numX + 5 * numK)/DMA_WORD_PER_BEAT;
	uint32_t dma_addr = offset;
	bool pingpong_x = true;


	if(numX < batch_size_x)
	  {
	    batch_size_x = numX;
	  }

	for(int i = 0; i < num_batch_x; i++)
	  {
	    this->store_compute_handshake();
	    if(pingpong_x)
	      {
		store_one_data(plm_Qr_ping, dma_addr, batch_size_x);
		dma_addr += batch_size_x/DMA_WORD_PER_BEAT;
		store_one_data(plm_Qi_ping, dma_addr, batch_size_x);
		dma_addr += batch_size_x/DMA_WORD_PER_BEAT;
	      }
	    else
	      {
		store_one_data(plm_Qr_pong, dma_addr, batch_size_x);
		dma_addr += batch_size_x/DMA_WORD_PER_BEAT;
		store_one_data(plm_Qi_pong, dma_addr, batch_size_x);
		dma_addr += batch_size_x/DMA_WORD_PER_BEAT;
	      }
	    pingpong_x = !pingpong_x;
	  }
    } // store process

    // Conclude
    {
        HLS_PROTO("store-done");
	//	wait();
        this->accelerator_done();
        this->process_done();

    }
}


void mriq::compute_kernel()
{
    // Reset
    {
        HLS_PROTO("compute-reset");

        this->reset_compute_kernel();

        // explicit PLM ports reset if any

        // User-defined reset code

        wait();
    }

    // Config
    /* <<--params-->> */
    int32_t numX;
    int32_t numK;
    int32_t batch_size_k;
    int32_t batch_size_x;

    int32_t num_batch_x;
    int32_t num_batch_k;


    {
        HLS_PROTO("compute-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        /* <<--local-params-->> */
        numX = config.numX;
        numK = config.numK;
        batch_size_k = config.batch_size_k;
        batch_size_x = config.batch_size_x;
        num_batch_k = config.num_batch_k;
        num_batch_x = config.num_batch_x;


    }


    // Compute



    FPDATA_S sin_table[20];
    HLS_FLATTEN_ARRAY(sin_table);

    sin_table[0] = 0.000000000000;
    sin_table[1] = 0.078459098935;
    sin_table[2] = 0.156434476376;
    sin_table[3] = 0.233445376158;
    sin_table[4] = 0.309017002583;
    sin_table[5] = 0.382683455944;
    sin_table[6] = 0.453990519047;
    sin_table[7] = 0.522498548031;
    sin_table[8] = 0.587785243988;
    sin_table[9] = 0.649448037148;
    sin_table[10]= 0.707106769085;
    sin_table[11]= 0.760405957699;
    sin_table[12]= 0.809017002583;
    sin_table[13]= 0.852640211582;
    sin_table[14]= 0.891006529331;
    sin_table[15]= 0.923879504204;
    sin_table[16]= 0.951056540012;
    sin_table[17]= 0.972369909286;
    sin_table[18]= 0.987688362598;
    sin_table[19]= 0.996917307377;

        
    FPDATA_L Qr;
    FPDATA_L Qi;
    FPDATA_S x;
    FPDATA_S y;
    FPDATA_S z;

    bool pingpong_x = true;
    bool pingpong_k = true;

    if(numX < batch_size_x)
	batch_size_x = numX;
    if(numK < batch_size_k)
	batch_size_k = numK;
   


#if(ARCH==0)

    //ESP_REPORT_INFO("compute_kernel: come into if-SK part");

    for(int i = 0; i < num_batch_x; i++)
      {
	this->compute_load_handshake();
	for(int r = 0; r < batch_size_x; r++)
	  {

	    if(pingpong_x)
	      {
		x = int2fp<FPDATA_S, FPDATA_S_WL>(plm_x_ping[r]);
		y = int2fp<FPDATA_S, FPDATA_S_WL>(plm_y_ping[r]);
		z = int2fp<FPDATA_S, FPDATA_S_WL>(plm_z_ping[r]);
	      }  else
	      {
		x = int2fp<FPDATA_S, FPDATA_S_WL>(plm_x_pong[r]);
		y = int2fp<FPDATA_S, FPDATA_S_WL>(plm_y_pong[r]);
		z = int2fp<FPDATA_S, FPDATA_S_WL>(plm_z_pong[r]);
	      }

	    Qr=0; Qi=0;
	    ComputeQ(x, y, z, batch_size_k,  pingpong_k, sin_table, &Qr, &Qi);

	    if(pingpong_x)
	      {	
		plm_Qr_ping[r] = fp2int<FPDATA_L, FPDATA_L_WL>(Qr);
		plm_Qi_ping[r] = fp2int<FPDATA_L, FPDATA_L_WL>(Qi);   
		//		printf("Qr = %X; \n", plm_Qr_ping[r]);
		//		printf("Qi = %X; \n", plm_Qi_ping[r]);


	      }
	    else
	      { 
		plm_Qr_pong[r] = fp2int<FPDATA_L, FPDATA_L_WL>(Qr);
		plm_Qi_pong[r] = fp2int<FPDATA_L, FPDATA_L_WL>(Qi);   
	      }
	  } // end of batch_size_x
	pingpong_x = !pingpong_x;
	this->compute_store_handshake();
    }


#else

    //ESP_REPORT_INFO("compute_kernel: come into else part");
    FPDATA_L Qr_p;
    FPDATA_L Qi_p;
    //    int counter_pingpong = 0;


    for (int i=0; i < num_batch_x; i++)
      {
	for(int r=0; r < batch_size_x; r++)
	  {


	    Qr = 0; Qi = 0;
	  
	    for(int j=0; j< num_batch_k; j++)
	      {	      
		this->compute_load_handshake();	  

		if(pingpong_x)
		  {
		    x = int2fp<FPDATA_S, FPDATA_S_WL>(plm_x_ping[r]);
		    y = int2fp<FPDATA_S, FPDATA_S_WL>(plm_y_ping[r]);
		    z = int2fp<FPDATA_S, FPDATA_S_WL>(plm_z_ping[r]);
		  }  else
		  {
		    x = int2fp<FPDATA_S, FPDATA_S_WL>(plm_x_pong[r]);
		    y = int2fp<FPDATA_S, FPDATA_S_WL>(plm_y_pong[r]);
		    z = int2fp<FPDATA_S, FPDATA_S_WL>(plm_z_pong[r]);
		  }

		ComputeQ(x, y, z, batch_size_k,  pingpong_k, sin_table, &Qr_p, &Qi_p);

		Qr += Qr_p;
		Qi += Qi_p;
		pingpong_k = !pingpong_k;
	      }
	    if(pingpong_x)
	      {	plm_Qr_ping[r] = fp2int<FPDATA_L, FPDATA_L_WL>(Qr);
		plm_Qi_ping[r] = fp2int<FPDATA_L, FPDATA_L_WL>(Qi);   }
	    else
	      { plm_Qr_pong[r] = fp2int<FPDATA_L, FPDATA_L_WL>(Qr);
		plm_Qi_pong[r] = fp2int<FPDATA_L, FPDATA_L_WL>(Qi);   }
	  } // end of num_batch_k

	//	counter_pingpong -= 1;

	pingpong_x = !pingpong_x;
	this->compute_store_handshake();
	//	ESP_REPORT_INFO("compute finished!");
      } // end of num_batch_x


#endif
   // Conclude
        {
            this->process_done();
        }
    
}
