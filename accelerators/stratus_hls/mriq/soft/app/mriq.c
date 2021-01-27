#include "libesp.h"
#include "cfg.h"
#include <math.h>
#include <fixed_point.h>
#include <test/time.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#define PIx2 6.2831853071795864769252867665590058f
static unsigned in_words_adj;
static unsigned out_words_adj;

static unsigned in_len;
static unsigned out_len;

static unsigned in_size;
static unsigned out_size;

static unsigned out_offset;
static unsigned size;

/* User-defined code */


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


void comp(int numK, int numX,
              float *plm_kx, float *plm_ky, float *plm_kz,
              float* plm_x, float* plm_y, float* plm_z,
              float *plm_phiR, float *plm_phiI,
              float *plm_out) {
  float expArg;
  float cosArg;
  float sinArg;
  float phiMag;
  int indexK, indexX;
  for (indexX = 0; indexX < numX; indexX++) {
    // Sum the contributions to this point over all frequencies
    float Qracc = 0.0f;
    float Qiacc = 0.0f;
    for (indexK = 0; indexK < numK; indexK++) {
      phiMag = plm_phiR[indexK]*plm_phiR[indexK] + plm_phiI[indexK]*plm_phiI[indexK];


      expArg = PIx2 * (plm_kx[indexK] * plm_x[indexX] +
                       plm_ky[indexK] * plm_y[indexX] +
                       plm_kz[indexK] * plm_z[indexX]);
      cosArg = cosf(expArg);
      sinArg = sinf(expArg);

      Qracc += phiMag * cosArg;
      Qiacc += phiMag * sinArg;
    }
    plm_out[indexX] = Qracc;
    plm_out[numX + indexX] = Qiacc;
  }
}


static void init_buffer(token_t *in, float *gold, char* inputFile, char* goldFile)
{
  int numX_bm, numK_bm;

  float *kx, *ky, *kz, *phiR, *phiI, *x, *y, *z;
  float *Qr, *Qi;
  inputData(inputFile, &numK_bm, &numX_bm, &kx, &ky, &kz, &x, &y, &z, &phiR, &phiI);
  outputData(goldFile, &Qr, &Qi, &numX_bm);

  int i, j;
  int offset_idx = 0;
  int idx_single;

  for(i=0; i < num_batch_k; i++)
    {
      idx_single = i * num_batch_k;

      for(j=0; j < batch_size_k; j++)	{  
	in[offset_idx + j] = float2fx(kx[idx_single + j], FX_IL);
      }
      offset_idx += batch_size_k; // 1 

      for(j=0; j < batch_size_k; j++){
	  in[offset_idx + j] = float2fx(ky[idx_single + j], FX_IL);
      }
      offset_idx += batch_size_k; // 2

      for(j=0; j < batch_size_k; j++){
	  in[offset_idx + j] = float2fx(kz[idx_single + j], FX_IL);
      }      
      offset_idx += batch_size_k; // 3

      for(j=0; j < batch_size_k; j++){
	  in[offset_idx + j] = float2fx(phiR[idx_single + j], FX_IL);
      }
      offset_idx += batch_size_k; // 4

      for(j=0; j < batch_size_k; j++){
	  in[offset_idx + j] = float2fx(phiI[idx_single + j], FX_IL);
      }
      offset_idx += batch_size_k; // 5
    }


  for(i = 0; i < num_batch_x; i++)
    {
      idx_single = i * num_batch_x;

      for(j=0; j < batch_size_x; j++)
	  in[offset_idx + j] = float2fx(x[idx_single + j], FX_IL);
      offset_idx += batch_size_x; // 1

      for(j=0; j < batch_size_x; j++)
	  in[offset_idx + j] = float2fx(y[idx_single + j], FX_IL);
      offset_idx += batch_size_x; // 2

      for(j=0; j < batch_size_x; j++)
	  in[offset_idx + j] = float2fx(z[idx_single + j], FX_IL);
      offset_idx += batch_size_x; // 3
    }

  /*load golden output into the gold buffer */
  for(i = 0; i < numX; i++)
    {
      gold[i] = Qr[i];
      gold[i + numX] = Qi[i];
    }



}


static void init_parameters()
{
        if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
	  in_words_adj      = 3*numX + 5*numK;
	  out_words_adj     = 2 * numX;
        } else {
	  in_words_adj   = round_up(3*numX + 5*numK, DMA_WORD_PER_BEAT(sizeof(token_t)));
	  out_words_adj  = round_up(2*numX, DMA_WORD_PER_BEAT(sizeof(token_t)));
        }
        in_len      = in_words_adj      * (1);
        out_len     = out_words_adj     * (1);
  
        in_size   = in_len   * sizeof(token_t);
        out_size  = out_len  * sizeof(token_t);
        out_offset =   in_len ;
        size       =  (out_offset * sizeof(token_t)) + out_size;
}


static int validate_buf(token_t *out, token_t *gold)
{
  int i, j;
  unsigned errors = 0;
  float error_thresh = 0.1;
  float diff_Qr, diff_Qi;
  /* index for gold and out  */
  int i_g_Qr, i_g_Qi, i_o_Qr, i_o_Qi; 
  

  for(i = 0; i < num_batch_x; i++){
      for(j=0; j<batch_size_x; j++)
	{
	  /* golden output: Qr -> Qi */
	  i_g_Qr = i * batch_size_x + j;  
	  i_g_Qi = i * batch_size_x + j + numX;
	  // **** Qr ****  -
	  //               |
	  //          batch_size_x
	  //               |
	  // **** Qi ***   -
	  //               |
	  //          batch_size_x
	  //               |
	  // **** Qr ***   -
	  // Qr_idx = 0,1,2,3, Qi_idx = 4,5,6,7
	  // Qr_idx = 8,9,10,11, Qi_idx = 12,13,14,15
	  // Qr_idx = 16,17,18,19, Qi_idx = 20,21,22,23

	  i_o_Qr = 2 * i * batch_size_x + j;  
	  i_o_Qi = (2 * i + 1) * batch_size_x + j;

	  if(!out[i_o_Qr]) 
	    diff_Qr  = fabs((gold[i_g_Qr] - out[i_o_Qr])/gold[i_g_Qr]);
	  else if(!gold[i_g_Qr])
	    diff_Qr  = fabs((gold[i_g_Qr] - out[i_o_Qr])/out[i_o_Qr]);
	  else
	    diff_Qr = 0;

	  if(!out[i_o_Qi])
	    diff_Qi  = fabs((gold[i_g_Qi] - out[i_o_Qi])/gold[i_g_Qi]);
	  else if(!gold[i_g_Qi])
	    diff_Qi  = fabs((gold[i_g_Qi] - out[i_o_Qi])/out[i_o_Qi]);
	  else
	    diff_Qi = 0;

	  if(diff_Qr > error_thresh){
	    errors++;
	    printf("out_Qr[%d] = %f ; ", i_o_Qr, fx2float(out[i_o_Qr], FX_IL));
	    printf("gold_Qr[%d] = %f\n", i_g_Qr, fx2float(gold[i_g_Qr], FX_IL));
	  }
	  if(diff_Qi > error_thresh){
	    errors++;
	    printf("out_Qi[%d] = %f ; ", i_o_Qr, fx2float(out[i_o_Qi], FX_IL));
	    printf("gold_Qi[%d] = %f\n", i_g_Qr, fx2float(gold[i_g_Qi], FX_IL));
	  }
	}
  } /* end of for of num_batch_x  */
  return errors;
}


int main(int argc, char **argv)
{
  
  
  char* inputFile;
  char* goldFile;
  int errors = 0;
  token_t *mem;
  float ERROR_COUNT_TH = 0.001;
  float *gold;
	
  init_parameters();

  mem = (token_t *) esp_alloc(size);
  gold = (float *)malloc(out_size);
	
  inputFile = argv[1];
  goldFile = argv[2];

  init_buffer(mem, gold, inputFile, goldFile);

  printf("\n====== %s ======\n\n", cfg_000[0].devname);
  /* <<--print-params-->> */
  
  
  printf("  .numX = %d\n", numX);
  printf("  .numK = %d\n", numK);
  printf("  .batch_size_x = %d\n", batch_size_x);
  printf("  .batch_size_k = %d\n", batch_size_k);
  printf("  .num_batch_x  = %d\n", num_batch_x);
  printf("  .num_batch_k  = %d\n", num_batch_k);


  printf("\n  ********* START HW testing *******\n");
  esp_run(cfg_000, NACC);

  printf("\n  ** DONE **\n");


  printf("\n==== START SW testing ======\n");
  struct timespec ts_start, ts_end;
  unsigned long long sw_ns;
  float *kx, *ky, *kz, *phiR, *phiI, *x, *y, *z;
  int numK_bm, numX_bm;

  inputData(inputFile, &numK_bm, &numX_bm, &kx, &ky, &kz, &x, &y, &z, &phiR, &phiI);

  gettime(&ts_start);
  comp(numK, numX,   \
       kx,\
       ky,\
       kz,\
       x,\
       y,\
       z,\
       phiR,\
       phiI,\
       gold);
  gettime(&ts_end);

  sw_ns = ts_subtract(&ts_start, &ts_end);
  printf("\n sw exec time : %llu ns\n", sw_ns);



  printf("==== Compare hw and sw results: .....\n");
  int i;
  token_t *gold_fx;
  float error_rate;

  printf("mem size by esp_alloc(): %d\n", size);
  gold_fx = (token_t *)malloc(out_size);
  for(i=0; i<2*numX; i++)
    gold_fx[i] = float2fx(gold[i], FX_IL);

  errors = validate_buf(&mem[out_offset], gold_fx);
  printf("error number = %d\n", errors);
  error_rate = errors / out_len;
  printf("error rate = %f\n", error_rate);

  if (error_rate > ERROR_COUNT_TH)
    printf("+ Test FAIL!! : exceeding error count threshold.\n");
  else
    printf("+ Test PASS!! : NOT exceeding error count threshold.\n");

  printf("\n====== %s ======\n\n", cfg_000[0].devname);
  
  free(gold_fx);
  esp_cleanup();

  return errors;
}
