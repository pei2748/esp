#include <stdlib.h>
#include <stdio.h>
#include "file0.h"
#include <math.h>
#include <time.h>

#define PI   3.1415926535897932384626433832795029f
#define PIx2 6.2831853071795864769252867665590058f

int main () {

  //  clock_t start, end;
  //  double cpu_time_used;
  int numX, numK;               /* Number of X and K values */
  float *kx, *ky, *kz;          /* K trajectory (3D vectors) */
  float *x, *y, *z;             /* X coordinates (3D vectors) */
  float *phiR, *phiI;           /* Phi values (complex) */
  float *Qr, *Qi;               /* Q signal (complex) */
  char *input_name = "32_32_32_dataset.bin";
  //  char *input_name = "64_64_64_dataset.bin";
  //char *input_name = "128x128x128.bin";


  char *output_name = "32_32_32_dataset.out";
 
  int numK_small_size /*= 3072*/;
  int numX_small_size /*= 32768*/;


//  int numK_small_size = 2048;
//  int numX_small_size = 262144;
// 
  printf("\n start! \n");
  

   inputData(input_name, &numK, &numX,
	    &kx, &ky, &kz,
	    &x,  &y,  &z,
	    &phiR, &phiI);


   printf("numK = %d\n", numK);
   printf("numX = %d\n", numX);


   //****************************************************//
   // Set the numK and numX as we need. 
   // This should be the same as the value in system.hpp and mriq_conf_info.hpp
   
   numK_small_size = 16;
   numX_small_size = 4;
//   numK_small_size = 3072;
//   numX_small_size = 3072;
//


   float nx[numX_small_size], ny[numX_small_size], nz[numX_small_size];
   float nkx[numK_small_size], nky[numK_small_size], nkz[numK_small_size], nphiR[numK_small_size], nphiI[numK_small_size];

   int a, j;

   j = 0; // from 0 ~ 7

   a = 32768*j;

   

//   
//   FILE *fkp;
//   fkp = fopen("kValues_128.csv","a");
//   if(fkp == NULL){
//     fprintf(stderr, "can't open the file\n");
//     exit(-1);
//   }
//   fprintf(fkp, "\n,kx,ky,kz,phiR,phiI\n");     
//
//   for(int i = 0; i < numK_small_size; i++)
//     {
//       nkx[i] = kx[a+i];
//       nky[i] = ky[a+i];
//       nkz[i] = kz[a+i];
//       nphiR[i] = phiR[a+i];
//       nphiI[i] = phiI[a+i];
//       //     fprintf(fkp, "%d, %f, %f, %f, %f, %f\n", a+i, nkx[i], nky[i], nkz[i], nphiR[i], nphiI[i]); 
//
//     }
//   //   fclose(fkp);


   
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
       fprintf(fkp, "%f\n", nkx[i]); 
     }

  for(int i = 0; i < numK_small_size; i++)
     {
       nky[i] = ky[a+i];
       fprintf(fkp, "%f\n", nky[i]); 
     }
  for(int i = 0; i < numK_small_size; i++)
     {
       nkz[i] = kz[a+i];
       fprintf(fkp, "%f\n", nkz[i]); 
     }
  for(int i = 0; i < numK_small_size; i++)
     {
       nphiR[i] = phiR[a+i];
       fprintf(fkp, "%f\n", nphiR[i]); 
     }
  for(int i = 0; i < numK_small_size; i++)
     {
       nphiI[i] = phiI[a+i];
       fprintf(fkp, "%f\n", nphiI[i]); 
     }
  //*/

   for(int i = 0; i < numX_small_size; i++)
     {
       nx[i] = x[a+i];
       fprintf(fkp,"%f\n",nx[i]);     
     }
   for(int i = 0; i < numX_small_size; i++)
     {
       ny[i] = y[a+i];
       fprintf(fkp,"%f\n",ny[i]);     
     }
   for(int i = 0; i < numX_small_size; i++)
     {
       nz[i] = z[a+i];
       fprintf(fkp,"%f\n",nz[i]);     
     }
  fclose(fkp);


//   FILE *fxp;
//   fxp = fopen("xValues_128.csv","a");
//   if(fxp == NULL){
//     fprintf(stderr, "can't open the file\n");
//     exit(-1);
//   }
//
//   fprintf(fxp, "\n,x,y,z\n");     

//   for(int i = 0; i < numX_small_size; i++)
//     {
//       nx[i] = x[a+i];
//       ny[i] = y[a+i];
//       nz[i] = z[a+i];
//       //     fprintf(fxp,"%d,%f,%f,%f\n", a+i,nx[i],ny[i],nz[i]);     
//     }
//   //   fclose(fxp);



   /*
   FILE *fp;
   fp = fopen("maxmin.csv","a");
   if(fp == NULL){
     fprintf(stderr, "can't open the file\n");
     exit(-1);
   }

   fprintf(fp, "\nX range: %d ~ %d \n", a, a+32768);

   fprintf(fp, " , x, y, z, kx, ky, kz, phiI, phiR\n");     

   float max_x=0; float max_y=0; float max_z=0; float max_kx=0; float max_ky=0; float max_kz=0; float max_phiR=0; float max_phiI;
   max_x = find_maximum(nx, numX_small_size);
   max_y = find_maximum(ny, numX_small_size);
   max_z = find_maximum(nz, numX_small_size);

   max_kx = find_maximum(nkx, numK_small_size);
   max_ky = find_maximum(nky, numK_small_size);
   max_kz = find_maximum(nkz, numK_small_size);
   max_phiR = find_maximum(nphiR, numK_small_size);
   max_phiI = find_maximum(nphiI, numK_small_size);

   fprintf(fp,"Max, %f, %f, %f, %f, %f, %f, %f, %f\n", max_x, max_y, max_z, max_kx, max_ky, max_kz, max_phiR, max_phiI);   

   float min_x, min_y, min_z, min_kx, min_ky, min_kz, min_phiR, min_phiI;

   min_x = find_minimum(nx, numX_small_size);
   min_y = find_minimum(ny, numX_small_size);
   min_z = find_minimum(nz, numX_small_size);

   min_kx = find_minimum(nkx, numK_small_size);
   min_ky = find_minimum(nky, numK_small_size);
   min_kz = find_minimum(nkz, numK_small_size);
   min_phiR = find_minimum(nphiR, numK_small_size);
   min_phiI = find_minimum(nphiI, numK_small_size);

   fprintf(fp,"Min, %f, %f, %f, %f, %f, %f, %f, %f\n", min_x, min_y, min_z, min_kx, min_ky, min_kz, min_phiR, min_phiI);  

   fclose(fp);
*/

  // ***********The following is to compute golden output and write into a file****************
  //  /*

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
  

   
   FILE *flp;
   flp = fopen("test_gold.txt","w");
   if(flp == NULL){
     fprintf(stderr, "can't open the file\n");
     exit(-1);
   }
   
   // /*
   for(int i=0; i < numX_small_size; i++)       
     {
       fprintf(flp, "%f\n",Qr[i]);
       printf("%f\n",Qr[i]);
     }
   for(int i=0; i < numX_small_size; i++)       
       fprintf(flp, "%f\n",Qi[i]);
//   */
   /*
   for(int i=0; i < numX_small_size; i++)       
     {
       fprintf(flp, "%f ",Qr[i]);
       fprintf(flp, "%f ",Qi[i]);
     }
*/
   fclose(flp);
//*/
   //*******************************END*********************************************************
   /*
   // print out the golden output into a file

   float *gold_Qr, *gold_Qi;
   int numX_bm;

   outputData(output_name, &gold_Qr, &gold_Qi, &numX_bm);

   FILE *fbp;
   fbp = fopen("32_32_32_dataset_gold.txt","w");
   if(fbp == NULL){
     fprintf(stderr, "can't open the file\n");
     exit(-1);
   }

   //   fprintf(fbp, "\nX range: %d ~ %d \n", a, a+32768);

//   int n;
//   n = 32768 * 16;
   for(int i=0; i<numX_small_size; i++)
     {
       fprintf(fbp, "%f ", gold_Qr[i]);
     }
   for(int i=0; i<numX_small_size; i++)
     {
       fprintf(fbp, "%f ", gold_Qi[i]);
     }

   fclose(fbp);

   */
   return(0);
}
   

