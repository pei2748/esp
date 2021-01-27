#include <stdlib.h>
#include <stdio.h>
//#include "file0.h"
#include <math.h>
//#include <time.h>

#define PI   3.1415926535897932384626433832795029f
#define PIx2 6.2831853071795864769252867665590058f

int main () {

  int numX, numK;               /* Number of X and K values */
  float *kx, *ky, *kz;          /* K trajectory (3D vectors) */
  float *x, *y, *z;             /* X coordinates (3D vectors) */
  float *phiR, *phiI;           /* Phi values (complex) */
  float *Qr, *Qi;               /* Q signal (complex) */

//  char *inName = "128x128x128.bin";
//  char *outName = "128x128x128.out";
//  char *txtout = "128.txt";
//  numK = 2097152;
//  numX = 2097152;
//

  char *inName = "generate_128_test/test_128_x4_k16K.bin";
  char *outName = "generate_128_test/test_128_x4_k16K.out";
  char *txtout = "128.txt";

  numK = 32768;
  numX = 4;

  // test if inName has right data.
  FILE* fid = fopen(inName, "r");
  if (fid == NULL)
    {
      fprintf(stderr, "Cannot open input file\n");
      exit(-1);
    }

  phiR = malloc(sizeof(float) * numK);
  fseek(fid, sizeof(float)* (2 + 3*numX + 3*numK), SEEK_SET);
  int ret = fread(phiR, sizeof(float), numK, fid);

  printf("return value of fread is %d\n", ret);

  printf("The 100th data in phiR is %f\n", phiR[99]);
  fclose(fid);


  // test if outname has the right data.

  FILE *fptr = fopen(outName, "r");
  if (fptr == NULL){
    fprintf(stderr, "can't open output fle\n");
    exit(-1);
  }


  Qr = malloc(sizeof(float) * numX);
  Qi = malloc(sizeof(float) * numX);

  int numXb;
  ret = fread(&numXb, sizeof(float), 1, fptr);
  printf("ret is %d (should be 1)\n", ret);

  ret = fread(Qr, sizeof(float), numX, fptr);
  printf("ret is %d (should be numX)\n", ret);

  ret = fread(Qi, sizeof(float), numX, fptr);
  printf("ret is %d (should be numX)\n", ret);

  printf("The 1st data in Qr is %f \n", Qr[0]);
  printf("The 1st data in Qi is %f \n", Qi[0]);

  fclose(fptr);

  FILE *ftxt = fopen(txtout, "wr");
  if(ftxt == NULL)
    fprintf(stderr, "open file failed!\n");

  for(int i = 0; i < numX; i++){
    fprintf(ftxt, "%f           %f\n", Qr[i], Qi[i]);    
  }
  fclose(ftxt);



  free(phiR);
  free(Qr);
  free(Qi);
  return 0;

}
