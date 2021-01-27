#include <stdio.h>
#include <stdlib.h>

#include "file0.h"
#include <math.h>

float diff(float a, float gold){
  float diff_r;

  if (gold == 0 && a == 0)
    diff_r = 0;
  else if(gold == 0 || a == 0)
    diff_r = fabs(gold - a);
  else
    diff_r = fabs((gold - a)/gold);
  
  return diff_r;
 
}

int main(){
    int numX, numK;
    float *kx, *ky, *kz;          /* K trajectory (3D vectors) */
    float *x, *y, *z;             /* X coordinates (3D vectors) */
    float *phiR, *phiI;           /* Phi values (complex) */
    float *Qr, *Qi;

    char *inputName = "test.bin";
    char *goldName = "test.out";

    inputData(inputName, &numK, &numX,
	      &kx, &ky, &kz,
	      &x,  &y,  &z,
	      &phiR, &phiI);


    createDataStructsforCompute(numX, &Qr, &Qi);

    ComputeQ(numK, numX,\
	     kx,\
	     ky,\
	     kz,\
	     x,\
	     y,\
	     z,\
	     phiR,\
	     phiI,\
	     Qr,Qi);

    float *goldQr, *goldQi;
    goldQr = malloc(sizeof(float) * numX);
    goldQi = malloc(sizeof(float) * numX);

    outputData(goldName, &goldQr, &goldQi, &numX);

    float thresh = 0.01;
    int errors = 0;
    for(int i = 0; i < numX; i++){
      printf("Qr[%d] = %f, goldQr[%d] = %f\n", i, Qr[i], i, goldQr[i]);
      printf("Qi[%d] = %f, goldQi[%d] = %f\n", i, Qi[i], i, goldQi[i]);

      if(diff(Qr[i], goldQr[i]) > thresh)
	errors += 1;

      if(diff(Qi[i], goldQi[i]) > thresh)
	errors += 1;
       
    }
    
    printf("errors = %d\n", errors);

    free(goldQr);
    free(goldQi);
    free(kx);
    free(ky);
    free(kz);
    free(x);
    free(y);
    free(z);
    free(phiR);
    free(phiI);
    free(Qr);
    free(Qi);
    return 0;

}
