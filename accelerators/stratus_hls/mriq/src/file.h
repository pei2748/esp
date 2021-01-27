/***************************************************************************
 *cr
 *cr            (C) Copyright 2007 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void inputData(char* fName, int* _numK, int* _numX,
               float** kx, float** ky, float** kz,
               float** x, float** y, float** z,
               float** phiR, float** phiI);

void outputData(char* fName, float** outR, float** outI, int* numX);

void ComputeQ(int numK, int numX,
	 float *plm_kx, float *plm_ky, float *plm_kz,
	 float* plm_x, float* plm_y, float* plm_z,
	      float *plm_phiR, float *plm_phiI, float *plm_Qr, float *plm_Qi);

#ifdef __cplusplus
}
#endif
