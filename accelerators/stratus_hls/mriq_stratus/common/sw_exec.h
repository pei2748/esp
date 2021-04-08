#include <time.h>

#define PIx2 6.2831853071795864769253f

void sw_comp(int batch_size_k, int num_batch_k, 
		    int batch_size_x, int num_batch_x,
		    float *kx, float *ky, float *kz,
		    float *x, float *y, float *z,
		    float *phiR, float *phiI,
		    float *out)
{
  float expArg;
  float cosArg;
  float sinArg;
  float phiMag;
  int indexK, indexX;
  int numK = batch_size_k * num_batch_k;

  for (int b = 0; b < num_batch_x; b++) {
   
       int base = b * batch_size_x;

       for (indexX = 0; indexX < batch_size_x; indexX++) {
           // Sum the contributions to this point over all frequencies
           float Qracc = 0.0f;
	   float Qiacc = 0.0f;

	   for (indexK = 0; indexK < numK; indexK++) {

	     phiMag = phiR[indexK]*phiR[indexK] + phiI[indexK]*phiI[indexK];

	     expArg = PIx2 * (kx[indexK] * x[base + indexX] +
			      ky[indexK] * y[base + indexX] +
			      kz[indexK] * z[base + indexX]);
	     cosArg = cosf(expArg);
	     sinArg = sinf(expArg);

	     Qracc += phiMag * cosArg;
	     Qiacc += phiMag * sinArg;
	   }

	   out[base * 2 + indexX] = Qracc;
	   out[base * 2 + indexX + batch_size_x] = Qiacc;
       }
  }
}


void sw_exec(float *gold, float *in, 
	     int32_t batch_size_x, int32_t num_batch_x,
	     int32_t batch_size_k, int32_t num_batch_k)
{

  int numX, numK;
  float *kx, *ky, *kz, *x, *y, *z, *phiR, *phiI;
  
  numX = batch_size_x * num_batch_x;
  numK = batch_size_k * num_batch_k;

  kx = malloc(sizeof(numK));
  ky = malloc(sizeof(numK));
  kz = malloc(sizeof(numK));
  phiR = malloc(sizeof(numK));
  phiI = malloc(sizeof(numK));


  x = malloc(sizeof(numX));
  y = malloc(sizeof(numX));  
  z = malloc(sizeof(numX));


  for (int r = 0; r < num_batch_k; r++) {
    for (int i = 0; i < batch_size_k; i++) {
      int idx = r * batch_size_k + i;
      kx[idx] = in[5 * r * batch_size_k + i];
      ky[idx] = in[(5 * r + 1) * batch_size_k + i];
      kz[idx] = in[(5 * r + 2) * batch_size_k  + i];
      phiR[idx] = in[(5 * r+3) * batch_size_k  + i];
      phiI[idx] = in[(5 * r+4) * batch_size_k  + i];
    }
  }

  for (int r = 0; r < num_batch_x; r++) {
    for (int i = 0; i < batch_size_x; i++) {
      int idx = r * batch_size_x + i;
      x[idx] = in[5 * r * batch_size_x + i];
      y[idx] = in[(5 * r + 1) * batch_size_x + i];
      z[idx] = in[(5 * r + 2) * batch_size_x  + i];

    }
  }

  struct timespec ts_start, ts_end;
  unsigned long long sw_ns;


  printf("\n  ** START SW TESTING **\n");
  gettime(&ts_start);

  sw_comp(batch_size_k, num_batch_k, 
	  batch_size_x, num_batch_x,
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

  printf("\n    SW exec time : %llu (ns)\n", sw_ns);
  printf("\n  ** Done! **\n\n");

  free(x);
  free(y);
  free(z);
  free(kx);
  free(ky);
  free(kz);
  free(phiR);
  free(phiI);



}

