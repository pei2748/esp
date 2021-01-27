#include "libesp.h"
#include "cfg.h"
#include <malloc.h>


#include <stdio.h>
#include <stdlib.h>
#include <math.h>


static unsigned in_words_adj;
static unsigned out_words_adj;
static unsigned in_len;
static unsigned out_len;
static unsigned in_size;
static unsigned out_size;
static unsigned out_offset;
static unsigned size;


/* User-defined code */
static int validate_buffer(token_t *out, token_t *gold)
{
	int i;
	int j;
	unsigned errors = 0;
	float diff;
	float error_th = 0.01;

	for (i = 0; i < 1; i++)
	  for (j = 0; j < 2*numX; j++){
	    diff = fabs((fx2float(gold[i * out_words_adj + j], FX_IL) - fx2float(out[i * out_words_adj + j], FX_IL))
			/fx2float(gold[i * out_words_adj + j], FX_IL));

	    if (diff > error_th)
	      errors++;
	  }

	return errors;
}

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


/* User-defined code */
static void init_buffer(token_t *in, token_t *gold, char* inputFile, char* goldFile)
{
       int numX_bm, numK_bm;

       float *kx, *ky, *kz, *phiR, *phiI, *x, *y, *z;
       float *Qr, *Qi;
       inputData(inputFile, &numK_bm, &numX_bm, &kx, &ky, &kz, &x, &y, &z, &phiR, &phiI);
       outputData(goldFile, &Qr, &Qi, &numX_bm);
       int i;

	FILE *fin;
	fin = fopen(inputFile, "r");
    // load golden output from .out file
	FILE *fpr;
	fpr = fopen(goldFile, "r");

	if(!fin || !fpr)
	  {
	    printf("cannot open file... FAIL!\n");
	    exit(1);
	  }

	for (i = 0; i < numK; i++)
	  {
	    in[i] = float2fx(kx[i], FX_IL);
	    in[i + numK]   = float2fx(ky[i], FX_IL);
	    in[i + 2*numK] = float2fx(kz[i], FX_IL);
	    in[i + 3*numK] = float2fx(phiR[i], FX_IL);
	    in[i + 4*numK] = float2fx(phiI[i], FX_IL);
	  }

	for (i = 0; i < numX; i++)
	  {
	    in[i +            5*numK] = float2fx(x[i], FX_IL);
	    in[i + 1 * numX + 5*numK] = float2fx(y[i], FX_IL);
	    in[i + 2 * numX + 5*numK] = float2fx(z[i], FX_IL);
	  }

	for(i = 0; i < numX; i++)
	  {
	    gold[i] = float2fx(Qr[i], FX_IL);
	    gold[i + numX] = float2fx(Qi[i], FX_IL);
	  }
}



/* User-defined code */
static void init_parameters()
{
	if (DMA_WORD_PER_BEAT(sizeof(token_t)) == 0) {
		in_words_adj = 3*numX + 5*numK;
		out_words_adj = 2*numX;
	} else {
		in_words_adj = round_up(3*numX + 5*numK, DMA_WORD_PER_BEAT(sizeof(token_t)));
		out_words_adj = round_up(2*numX, DMA_WORD_PER_BEAT(sizeof(token_t)));
	}
	in_len = in_words_adj * (1);
	out_len =  out_words_adj * (1);
	in_size = in_len * sizeof(token_t);
	out_size = out_len * sizeof(token_t);
	out_offset = in_len;
	size = (out_offset * sizeof(token_t)) + out_size;
}


int main(int argc, char **argv)
{
	int errors;
	token_t *gold;
	token_t *buf;
	char* inputFile;
	char* goldFile;
	float ERROR_RATE_TH = 0.001;
	float error_rate;

	inputFile = argv[1];
	goldFile = argv[2];

	init_parameters();

	buf = (token_t *) esp_alloc(size);
	gold = malloc(out_size);

	init_buffer(buf, gold, inputFile, goldFile);

	printf("\n====== %s ======\n\n", cfg_000[0].devname);
	/* <<--print-params-->> */
	printf("  .numX = %d\n", numX);
	printf("  .numK = %d\n", numK);
	printf("\n  ** START **\n");

	esp_run(cfg_000, NACC);

	printf("\n  ** DONE **\n");

	errors = validate_buffer(&buf[out_offset], gold);

	free(gold);
	esp_cleanup();

	error_rate = (float) (errors*0.5 / numX);

	printf("error number = %d; error rate = %f\n", errors, error_rate);
	if (error_rate < ERROR_RATE_TH)
		printf("+ Test PASSED\n");
	else
		printf("+ Test FAILED\n");

	printf("\n====== %s ======\n\n", cfg_000[0].devname);

	return errors;
}
