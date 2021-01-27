#include <stdlib.h>
#include <stdio.h>
#include "file0.h"
#include <math.h>
#include <time.h>

#define PI   3.1415926535897932384626433832795029f
#define PIx2 6.2831853071795864769252867665590058f

int main () {

  int numX, numK;               /* Number of X and K values */
  float *kx, *ky, *kz;          /* K trajectory (3D vectors) */
  float *x, *y, *z;             /* X coordinates (3D vectors) */
  float *phiR, *phiI;           /* Phi values (complex) */
  float *Qr, *Qi;               /* Q signal (complex) */

//  char *input_name = "128x128x128.bin";
//  char *outName = "gen128.bin";
//  char *outputName = "128x128x128.out";
//
  char *input_name = "128x128x128.bin";
  char *outName = "test_128_1x.bin";
  char *outputName = "test_128_1x.out";


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
  numX = 1;
  // write numK, numX, kx, ky, kz, x, y, z phiR phiI to a new file.

  ret = fwrite(&numK,sizeof(int),1, fid);  
  ret = fwrite(&numX,sizeof(int),1, fid);  

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

  float new_phiR = 0.484375;
  float new_phiI = 0.484375;
  ret = 0;
  for(int i=0; i<numK; i++){
    ret += fwrite(&new_phiR,sizeof(float),1, fid); 
  }
  if(ret!=numK){
    printf("not writing correctly!\n");
  }

  ret = 0;
  for(int i=0; i<numK; i++){
    ret += fwrite(&new_phiI,sizeof(float),1, fid); 
  }
  if(ret!=numK){
    printf("not writing correctly!\n");
  }

  fclose(fid);

  // read out value from the newly generated file.
  inputData(outName, &numK, &numX,
            &kx, &ky, &kz,
            &x,  &y,  &z,
            &phiR, &phiI);



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
  fprintf(stderr, "Qr = %f\n", *Qr);
  fprintf(stderr, "Qi = %f\n", *Qi);

  
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
  return 0;

}
