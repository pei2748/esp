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
//#include <legacy_C_header.h>

#include <string.h>


#define PI   3.1415926535897932384626433832795029f
#define PIx2 6.2831853071795864769252867665590058f




/*
#if __BYTE_ORDER != __LITTLE_ENDIAN
# error "File I/O is not implemented for this system: wrong endianness."
#endif
*/

extern "C"
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



extern "C"
void ComputeQ(int numK, int numX,
	      float *plm_kx, float *plm_ky, float *plm_kz,
	      float* plm_x, float* plm_y, float* plm_z,
	      float *plm_phiR, float *plm_phiI,
	      float *plm_Qr, float *plm_Qi) {
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
    plm_Qr[indexX] = Qracc;
    plm_Qi[indexX] = Qiacc;
  }
}

extern "C"
void createDataStructsforCompute(int numX, float** Qr, float** Qi)
{

    *Qr = (float*) memalign(16, numX * sizeof (float));
    memset((void *)*Qr, 0, numX * sizeof(float));
    *Qi = (float*) memalign(16, numX * sizeof (float));
    memset((void *)*Qi, 0, numX * sizeof(float));
}

extern "C"
void
Compute4testsin(int numK, int numX,
	      float *plm_kx, float *plm_ky, float *plm_kz,
		  float* plm_x, float* plm_y, float* plm_z,
		  float *expArg){
	    
  //float expArg;
  
  int indexX;
  int indexK;

  FILE *fp;
  fp = fopen("expArg.txt", "w");
    for (indexX = 0; indexX < numX; indexX++) {

      for (indexK = 0; indexK < numK; indexK++) {
	 expArg[indexX * numK + indexK] = PIx2 * (plm_kx[indexK] * plm_x[indexX] +
			  plm_ky[indexK] * plm_y[indexX] +
			  plm_kz[indexK] * plm_z[indexX]);
	 fprintf(fp, "%f\n",expArg[indexX * numK + indexK]);
       }
  }
    fclose(fp);
}

extern "C"
void createDataStructs4testsin(int numX, int numK,  float**expArg)
{

    *expArg = (float*) memalign(16, numX*numK * sizeof (float));
    memset((void *)*expArg, 0, numX *numK* sizeof(float));
}


