#include "../inc/espacc_config.h"
#include "../inc/espacc.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fstream>
#include <string.h>
//#include <vector>
#include <iostream>
#include <math.h>
#include <malloc.h>
//void inputData(const char *file, int* _numK, int* _numX, float** kx, float** ky, float** kz, float** x, float** y, float** z,float** phiR, float** phiI);


void inputData(char* fName, int* _numK, int* _numX,
               float** kx, float** ky, float** kz,
               float** x, float** y, float** z,
               float** phiR, float** phiI)
{


  FILE* fid = fopen(fName, "r");
  int numK, numX;
  if (fid == NULL)
    {
      fprintf(stderr, "Cannot open input file\n");
      exit(-1);
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




void outputData(char* fName, float** outR, float** outI, int* _numX)
{
  int numX;
  FILE* fid = fopen(fName, "r");

  if (fid == NULL)
    {
      fprintf(stderr, "Cannot open output file\n");
      exit(-1);
    }


  fread(&numX, sizeof(int), 1, fid);
  *_numX = numX;


  *outR = (float *) memalign(16, numX * sizeof (float));
  fread(*outR, sizeof(float), numX, fid);

  *outI = (float *) memalign(16, numX * sizeof (float));
  fread(*outI, sizeof(float), numX, fid);
  fclose (fid);
}




int main(int argc, char **argv) {

  printf("****start*****\n");

    /* <<--params-->> */
	 //const unsigned numX = 32768;
	 //const unsigned numK = 3072;
	 const unsigned numX = 4;
	 const unsigned numK = 16;
//
//	 const unsigned numX = 3072;
//	 const unsigned numK = 3072;
//
    uint32_t in_words_adj;
    uint32_t out_words_adj;
    uint32_t in_size;
    uint32_t out_size;
    uint32_t dma_in_size;
    uint32_t dma_out_size;
    uint32_t dma_size;


    in_words_adj = round_up(3*numX + 5*numK, VALUES_PER_WORD);
    out_words_adj = round_up(2*numX, VALUES_PER_WORD);

    in_size = in_words_adj * (1);
    out_size = out_words_adj * (1);

    dma_in_size = in_size / VALUES_PER_WORD;
    dma_out_size = out_size / VALUES_PER_WORD;
    dma_size = dma_in_size + dma_out_size;

    dma_word_t *mem=(dma_word_t*) malloc(dma_size * sizeof(dma_word_t));
    word_t *inbuff=(word_t*) malloc(in_size * sizeof(word_t));
    word_t *outbuff=(word_t*) malloc(out_size * sizeof(word_t));
    word_t *outbuff_gold= (word_t*) malloc(out_size * sizeof(word_t));
    dma_info_t *load = (dma_info_t*) malloc(sizeof(dma_info_t));
    dma_info_t *store = (dma_info_t*) malloc(sizeof(dma_info_t));




#ifdef RTL_SIM
    std::string RESULTS_LOG = "/home/esp2020/pl2748/esp/accelerators/vivado_hls/mriq_v2/tb_data/rtl_cosim_results.log";
#else
    std::string RESULTS_LOG = "/home/esp2020/pl2748/esp/accelerators/vivado_hls/mriq_v2/tb_data/csim_results.log";
#endif
    std::cout << "INFO: save inference results to file: " << RESULTS_LOG << std::endl;
    std::ofstream results;
    results.open(RESULTS_LOG);
    if(!results.is_open())
	printf("results can't be opened! \n");



    // set inbuff as input data.

    
    /* fin is the input file
     * fpr is the golden output
     * We can replace "test" with "32_32_32_dataset" or "64_64_64_dataset".
     */
//    std::string inputName = "/home/esp2020/pl2748/esp/accelerators/vivado_hls/mriq_v2/tb_data/32_32_32_dataset.bin";
//    std::string goldName = "/home/esp2020/pl2748/esp/accelerators/vivado_hls/mriq_v2/tb_data/32_32_32_dataset.out";
    std::string inputName = "/home/esp2020/pl2748/esp/accelerators/vivado_hls/mriq_v2/tb_data/test_small.bin";
    std::string goldName = "/home/esp2020/pl2748/esp/accelerators/vivado_hls/mriq_v2/tb_data/test_small.out";
    std::string data4barec = "/home/esp2020/pl2748/esp/accelerators/vivado_hls/mriq_v2/tb_data/test_in.h";

    char inputName_c[inputName.size() + 1];
    std::copy(inputName.begin(), inputName.end(), inputName_c);
    inputName_c[inputName.size()] = '\0';

    char goldName_c[goldName.size() + 1];
    std::copy(goldName.begin(), goldName.end(), goldName_c);
    goldName_c[goldName.size()] = '\0';

    std::ifstream fin;
    fin.open(inputName);
    std::ifstream fpr;
    fpr.open(goldName);

    std::ofstream fbc;
    fbc.open(data4barec);


    if(!fin.is_open() || !fpr.is_open() || !fbc.is_open())
      {
	printf("cannot open file... FAIL!\n");
	exit(1);
      }

    float *kx, *ky, *kz;          /* K trajectory (3D vectors) */
    float *x, *y, *z;             /* X coordinates (3D vectors) */
    float *phiR, *phiI;           /* Phi values (complex) */
    float *goldQr, *goldQi;
    int numX_bm, numK_bm; /* should be the same value as numX and numK */

    inputData(inputName_c, &numK_bm, &numX_bm,
              &kx, &ky, &kz,
              &x,  &y,  &z,
              &phiR, &phiI);


    goldQr = (float *) malloc(sizeof(float) * numX);
    goldQi = (float *) malloc(sizeof(float) * numX);
    outputData(goldName_c, &goldQr, &goldQi, &numX_bm);


    unsigned i;
    for(i = 0; i < numK; i++) {
      inbuff[i] = (word_t) kx[i];
      inbuff[i + numK] = (word_t) ky[i];
      inbuff[i + 2 * numK] = (word_t) kz[i];
      inbuff[i + 3 * numK] = (word_t) phiR[i];
      inbuff[i + 4 * numK] = (word_t) phiI[i];
    }

    for(i = 0; i < numX; i++){
      inbuff[i + 5 * numK] = (word_t) x[i];
      inbuff[i + 5 * numK + numX] = (word_t) y[i];
      inbuff[i + 5 * numK + 2 * numX] = (word_t) z[i];
    }

    for(unsigned i = 0; i < dma_in_size; i++)
	for(unsigned k = 0; k < VALUES_PER_WORD; k++){
	  mem[i].word[k] = inbuff[i * VALUES_PER_WORD + k];
	  // print it out and put it as input
	  fbc << "in[" << i * VALUES_PER_WORD + k << "] = " <<mem[i].word[k] << "\n";
	}

    std::cout << "finish sending input data\n" << std::endl;

    // Set golden output

    for(unsigned j = 0; j < numX; j++){
    	outbuff_gold[j] = (word_t) goldQr[j];
	fbc << "gold[" << j << "] = " << outbuff_gold[j] << "\n";
    }

    for(unsigned j=0; j < numX; j++){
    	outbuff_gold[j + numX] = (word_t) goldQi[j];
	fbc << "gold[" << j + numX << "] = " << outbuff_gold[j+numX] << "\n";
    }


    std::cout << "finish sending golden data\n" << std::endl;
   
    // Call the TOP function
    top(mem, mem,
        /* <<--args-->> */
	 	 numX,
	 	 numK,
        load, store);


    std::cout << "return from accelerator!" << RESULTS_LOG << std::endl;
    // Validate
    uint32_t out_offset = dma_in_size;

    for(unsigned i = 0; i < dma_out_size; i++)
	for(unsigned k = 0; k < VALUES_PER_WORD; k++)
	  {
	    outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];
	    //	    std::cout << outbuff[i * VALUES_PER_WORD + k] << "\n";
	    // compare with bare metal output
	  }


    int errors = 0;
    float error_th = 0.05;
    float error_rate_total;
    float error_rate_th = 0.01;
    float error_rate;
    for(unsigned i = 0; i < 1; i++)
        for(unsigned j = 0; j < 2*numX; j++)
	  {
	    error_rate = abs(((float)(outbuff[i * out_words_adj + j]) - (float)(outbuff_gold[i * out_words_adj + j]))/(float)(outbuff_gold[i * out_words_adj + j]));
	    if (error_rate > error_th)
	      {
	    	errors++;
	      }
	  }
    std::cout << "Fail or pass?" << std::endl;
    error_rate_total = errors/(2 * numX);
    if (error_rate_total > error_rate_th){
    	std::cout << "Test FAILED with " << errors << " errors.\n" << std::endl;
    	std::cout << "error rate is " << error_rate_total << "\n" << std::endl;
    } else {
    	std::cout << "Test PASSED." << std::endl;
    }


    // Free memory
    free(kx);
    free(ky);
    free(kz);
    free(x);
    free(y);
    free(z);
    free(phiR);
    free(phiI);
    free(goldQr);
    free(goldQi);

    free(mem);
    free(inbuff);
    free(outbuff);
    free(outbuff_gold);
    free(load);
    free(store);


    fin.close();
    fpr.close();
    fbc.close();
    results.close();

    return 0;
} 
