#include <stdlib.h>
#include <stdio.h>
#include "file0.h"
#include <math.h>
#include <time.h>
#include <ap_fixed.h>
#include <espacc_config.h>
#include <espacc.h>
#define PI   3.1415926535897932384626433832795029f
#define PIx2 6.2831853071795864769252867665590058f

int main () {

 
  int numX, numK;               /* Number of X and K values */
  float *kx, *ky, *kz;          /* K trajectory (3D vectors) */
  float *x, *y, *z;             /* X coordinates (3D vectors) */
  float *phiR, *phiI;           /* Phi values (complex) */
  float *Qr, *Qi;               /* Q signal (complex) */
  char *input_name = "32_32_32_dataset.bin";
  char *output_name = "32_32_32_dataset.out";

  int numK_small_size /*= 3072*/;
  int numX_small_size /*= 32768*/;

  printf("\n start! \n");


  inputData(input_name, &numK, &numX,
            &kx, &ky, &kz,
            &x,  &y,  &z,
            &phiR, &phiI);


  printf("numK = %d\n", numK);
  printf("numX = %d\n", numX);

  numK_small_size = 16;
  numX_small_size = 4;

  float nx[numX_small_size], ny[numX_small_size], nz[numX_small_size];
  float nkx[numK_small_size], nky[numK_small_size], nkz[numK_small_size], nphiR[numK_small_size], nphiI[numK_small_size];

  FILE *fkp;
  fkp = fopen("test_input.txt","w");
  if(fkp == NULL){
    fprintf(stderr, "can't open the file\n");
    exit(-1);
  }

  //   /*                                                                                                                                                    
  for(int i = 0; i < numK_small_size; i++)
    {
      nkx[i] = kx[a+i];
      fprintf(fkp, "%f ", nkx[i]);
    }

  for(int i = 0; i < numK_small_size; i++)
    {
      nky[i] = ky[a+i];
      fprintf(fkp, "%f ", nky[i]);
    }
  for(int i = 0; i < numK_small_size; i++)
    {
      nkz[i] = kz[a+i];
      fprintf(fkp, "%f ", nkz[i]);
    }
  for(int i = 0; i < numK_small_size; i++)
    {
      nphiR[i] = phiR[a+i];
      fprintf(fkp, "%f ", nphiR[i]);
    }                                                                                                                                                        
  for(int i = 0; i < numK_small_size; i++)
    {
      nphiI[i] = phiI[a+i];
      fprintf(fkp, "%f ", nphiI[i]);
    }
  //*/                                                                                                                                                        

  for(int i = 0; i < numX_small_size; i++)
    {
      nx[i] = x[a+i];
      fprintf(fkp,"%f ",nx[i]);
    }
  for(int i = 0; i < numX_small_size; i++)
    {
      ny[i] = y[a+i];
      fprintf(fkp,"%f ",ny[i]);
    }
  for(int i = 0; i < numX_small_size; i++)
    {
      nz[i] = z[a+i];
      fprintf(fkp,"%f ",nz[i]);
    }



  float *gold_Qr, *gold_Qi;                                                                                                                                  
  int numX_bm;                                                                                                                                               
                                                                                                                                                             
  outputData(output_name, &gold_Qr, &gold_Qi, &numX_bm);                                                                                                     

  for(int i = 0; i < numX_small_size; i++)
    {
      fprintf(fkp, "%f", gold_Qr[i]);
    }
  fclose(fkp);

  return 0;
}
