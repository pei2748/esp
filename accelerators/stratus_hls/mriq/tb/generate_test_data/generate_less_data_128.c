#include <stdlib.h>
#include <stdio.h>
#include "file0.h"
#include <math.h>

#define PI   3.1415926535897932384626433832795029f
#define PIx2 6.2831853071795864769252867665590058f



int main () {

  int numX, numK;               /* Number of X and K values */
  float *kx, *ky, *kz;          /* K trajectory (3D vectors) */
  float *x, *y, *z;             /* X coordinates (3D vectors) */
  float *phiR, *phiI;           /* Phi values (complex) */
  float *Qr, *Qi;               /* Q signal (complex) */

  char *input_name = "128x128x128.bin"; // with non-zero phiR and phiI.

  int numK_small_size = 32768; //  1024 * 32
  int numX_small_size = 4;    // 4, 16, 32



 //  char *output_name = "32_32_32_dataset.out";

  printf("\n start! \n");
  

  inputData(input_name, &numK, &numX,
	    &kx, &ky, &kz,
	    &x,  &y,  &z,
	    &phiR, &phiI);
   

  float nx[numX_small_size];
  float ny[numX_small_size];
  float nz[numX_small_size];

  float nkx[numK_small_size];
  float nky[numK_small_size]; 
  float nkz[numK_small_size];
  float nphiR[numK_small_size];
  float nphiI[numK_small_size];

  for(int i = 0; i < numX_small_size; i++)
    {
      nx[i] = x[i];
      ny[i] = y[i];
      nz[i] = z[i];     
    }

  for(int i = 0; i < numK_small_size; i++)
    {
      nkx[i] = kx[i];
      nky[i] = ky[i];
      nkz[i] = kz[i];
      nphiR[i] = phiR[i];
      nphiI[i] = phiI[i];      
    }



  
  createDataStructsforCompute(numX_small_size, &Qr, &Qi);
  
  ComputeQ(numK_small_size, numX_small_size,	\
	     nkx,\
	     nky,\
	     nkz,\
	     nx,\
	     ny,\
	     nz,\
	     nphiR,\
	     nphiI,\
	     Qr,Qi);

  
  
  FILE *fin = fopen("128_x4_k32K.bin", "w");
  if(fin == NULL)
    fprintf(stderr, "open file failed!\n");
  int ret;

  ret = fwrite(&numK_small_size, sizeof(int), 1, fin);
  if (ret != 1)
    fprintf(stderr, "writing failed!\n");

  ret = fwrite(&numX_small_size, sizeof(int), 1, fin);
  if (ret != 1)
    fprintf(stderr, "writing failed!\n");
  
  ret = fwrite(nkx, sizeof(float), numK_small_size, fin);
  if (ret != numK_small_size)
    fprintf(stderr, "writing failed!\n");

  ret = fwrite(nky, sizeof(float), numK_small_size, fin);
  if (ret != numK_small_size)
    fprintf(stderr, "writing failed!\n");

  ret = fwrite(nkz, sizeof(float), numK_small_size, fin);
  if (ret != numK_small_size)
    fprintf(stderr, "writing failed!\n");

  ret = fwrite(nx, sizeof(float), numX_small_size, fin);
  if (ret != numX_small_size)
    fprintf(stderr, "writing failed!\n");

  ret = fwrite(ny, sizeof(float), numX_small_size, fin);
  if (ret != numX_small_size)
    fprintf(stderr, "writing failed!\n");

  ret = fwrite(nz, sizeof(float), numX_small_size, fin);
  if (ret != numX_small_size)
    fprintf(stderr, "writing failed!\n");
  
  // write phiR, phiI
  ret = fwrite(nphiR, sizeof(float), numK_small_size, fin);
  if (ret != numK_small_size)
    fprintf(stderr, "writing failed!\n");

  ret = fwrite(nphiI, sizeof(float), numK_small_size, fin);
  if (ret != numK_small_size)
    fprintf(stderr, "writing failed!\n");

  fclose(fin);
  ////////////////////////////end of writing to test.bin /////////
  

  //////// write to test.out ////////////////////////
  FILE *fptr = fopen("128_x4_k32K.out", "w"); // write only 
           
  // test for files not existing. 
  if (fptr == NULL) 
    {   
      printf("Error! Could not open file\n"); 
      exit(-1); // must include stdlib.h 
    } 
           

  // write to file vs write to screen 
  ret = fwrite(&numX_small_size, sizeof(int), 1, fptr);
  if (ret != 1)
    fprintf(stderr, "writing failed!\n");
  
  ret = fwrite(Qr, sizeof(float), numX_small_size, fptr);
  if (ret != numX_small_size)
    fprintf(stderr, "writing failed!\n");

  ret = fwrite(Qi, sizeof(float), numX_small_size, fptr);
  if (ret != numX_small_size)
    fprintf(stderr, "writing failed!\n");
  
  fclose(fptr);

  return(0);
}
   

