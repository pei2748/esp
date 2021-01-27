#include <stdlib.h>
#include <stdio.h>
#include "file0.h"
#include <math.h>
#include <time.h>

#define PI   3.1415926535897932384626433832795029f
#define PIx2 6.2831853071795864769252867665590058f

/*  First arg : value of numK.
 * e.g. if you want 4K (4096), first arg should be 4.
 */
int main (int argc, char **argv) {

//  char *arg;
//  int num_of_numK = atoi(arg);
//  fprintf(stderr, "value of argument is %d\n", num_of_numK);
//
  int numX, numK;               /* Number of X and K values */
  float *kx, *ky, *kz;          /* K trajectory (3D vectors) */
  float *x, *y, *z;             /* X coordinates (3D vectors) */
  float *phiR, *phiI;           /* Phi values (complex) */
  float *Qr, *Qi;               /* Q signal (complex) */

//  char *input_name = "128x128x128.bin";
//  char *outName = "gen128.bin";
//  char *outputName = "128x128x128.out";
//




  char *input_name = "../128x128x128.bin";
  char *outName    = "test_128_x256_k256K.bin";
  char *outputName = "test_128_x256_k256K.out";


  // read out kx,ky,kz, x, y, z.
  inputData(input_name, &numK, &numX,
            &kx, &ky, &kz,
            &x,  &y,  &z,
            &phiR, &phiI);


  FILE* fid = fopen(outName, "wr");
  if (fid == NULL)
    {
      fprintf(stderr, "Cannot open output file\n");
      exit(-1);
    }

  printf("numK : %d\n", numK);
  printf("numX : %d\n", numX);
  
  int ret;
  numK = 262144; // 1024 * 16
  numX = 256;

  // write numK, numX to the bin file

  ret = fwrite(&numK,sizeof(int),1, fid);  
  ret = fwrite(&numX,sizeof(int),1, fid);  
  // write kx, ky, kz to the bin file

  ret = fwrite(kx,sizeof(float),numK, fid);
  if(ret!=numK){
    printf("not writing correctly!\n");
  }
  ret = fwrite(ky,sizeof(float),numK, fid);
  if(ret!=numK){
    printf("not writing correctly!\n");
  }
  ret = fwrite(kz,sizeof(float),numK, fid);
  if(ret!=numK){
    printf("not writing correctly!\n");
  }

  // writing x, y, z to the bin file
  ret = fwrite(x,sizeof(float),numX, fid);
  if(ret!=numX){
    printf("not writing correctly!\n");
  }
  ret = fwrite(y,sizeof(float),numX, fid);
  if(ret!=numX){
    printf("not writing correctly!\n");
  }
  ret = fwrite(z,sizeof(float),numX, fid);
  if(ret!=numX){
    printf("not writing correctly!\n");
  }
  // write phiR and phiI
  ret = fwrite(phiR,sizeof(float),numK, fid);
  if(ret!=numK){
    printf("not writing correctly!\n");
  }
  ret = fwrite(phiI,sizeof(float),numK, fid);
  if(ret!=numK){
    printf("not writing correctly!\n");
  }

  fclose(fid);

  

  //
  createDataStructsforCompute(numX, &Qr, &Qi);

  fprintf(stderr, "after creating data structure for Qr and Qi\n");

  ComputeQ(numK, numX,
	   kx,
	   ky,
	   kz,
	   x,
	   y,
	   z,
	   phiR,
	   phiI,
	   Qr,Qi);
  fprintf(stderr, "after compute\n");


  FILE *fout = fopen(outputName, "wr");
  if(fout == NULL){
    printf("can't open file");
  }

  ret = fwrite(&numX, sizeof(float), 1, fout);

  if(ret!= 1){
    printf("not writing correctly!\n");
  }

  ret = fwrite(Qr, sizeof(float), numX, fout);
  if(ret!=numX){
    printf("not writing correctly!\n");
  }

  ret = fwrite(Qi, sizeof(float), numX, fout);
  if(ret!=numX){
    printf("not writing correctly!\n");
  }

  fclose(fout);

  free(Qr);
  free(Qi);
  free(x);
  free(y);
  free(z);
  free(kx);
  free(ky);
  free(kz);
  free(phiR);
  free(phiI);

  return 0;

}
