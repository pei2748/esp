/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

#include <endian.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>


#include <string.h>


#define PI   3.1415926535897932384626433832795029f
#define PIx2 6.2831853071795864769252867665590058f


extern "C"
void inputData(char* fName, int * _numK, int* _numX,
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



extern "C"
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
/*

void init_buff(token_t *inbuff, token_t *outbuff_gold)
{

  std::string inputName = "/home/esp2020/pl2748/new-esp/esp/accelerators/vivado_hls/mriq_vivado/hw/tb/test_small.bin";
  std::string goldName = "/home/esp2020/pl2748/new-esp/esp/accelerators/vivado_hls/mriq_vivado/hw/tb/test_small.out";
  std::string data4barec = "/home/esp2020/pl2748/new-esp/esp/accelerators/vivado_hls/mriq_vivado/hw/tb/test_barec.h";

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

  if(!fin.is_open() || !fpr.is_open()){
    fprintf(stderr, "cannot open file... FAIL!!!!!!!!\n");

    exit(1);
  }


  float *kx, *ky, *kz;         
  float *x, *y, *z;            
  float *phiR, *phiI;          
  float *goldQr, *goldQi;
  int numX_bm, numK_bm; 

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
      // print it out and put it as input to barec
      fbc << "in[" << i * VALUES_PER_WORD + k << "] = " <<mem[i].word[k] << "\n";
    }


  // Set golden output
  for(unsigned j = 0; j < numX; j++){
    outbuff_gold[j] = (word_t) goldQr[j];
    fbc << "gold[" << j << "] = " << outbuff_gold[j] << "\n";
  }

  for(unsigned j=0; j < numX; j++){
    outbuff_gold[j + numX] = (word_t) goldQi[j];
    fbc << "gold[" << j + numX << "] = " << outbuff_gold[j+numX] << "\n";
  }


}


void validate_buff(token_t *outbuff, token_t *outbuff_gold)
{
  // Validate
  uint32_t out_offset = dma_in_size;
  for(unsigned i = 0; i < dma_out_size; i++)
    for(unsigned k = 0; k < VALUES_PER_WORD; k++)
      outbuff[i * VALUES_PER_WORD + k] = mem[out_offset + i].word[k];

  int errors = 0;
  float error_th = 0.05;
  float error_rate_total;
  float error_rate_th = 0.01;
  float error_rate;
  for(unsigned i = 0; i < 1; i++)
    for(unsigned j = 0; j < 2*numX; j++)
      {
	error_rate = abs(((float)(outbuff[i * out_words_adj + j]) - (float)(outbuff_gold[i * out_words_adj + j]))/(float)(outbuff_gold[i * out_words_adj+ j\
																       ]));
	if (error_rate > error_th)
	  {
	    errors++;
	  }
      }


  error_rate_total = errors/(2 * numX);

  if (error_rate_total > error_rate_th){
    std::cout << "Test FAILED with " << errors << " errors.\n" << std::endl;
    std::cout << "error rate is " << error_rate_total << "\n" << std::endl;
  } else {
    std::cout << "Test PASSED." << std::endl;
  }




}
*/
