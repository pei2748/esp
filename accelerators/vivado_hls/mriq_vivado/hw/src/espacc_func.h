 compute_data:
  //  const unsigned paral = 4;
  const word_t PI2 = 6.2831853071796;
  const word_t HALF_PI = ((word_t) (0.25)) * PI2;


  word_t x, y, z;

  word_t kx[PRL], ky[PRL], kz[PRL], phiR[PRL], phiI[PRL];
  word_t expArg[PRL], phiMag[PRL];
  word_t cosArg[PRL], sinArg[PRL];

  word_t Qracc_p[PRL], Qiacc_p[PRL];
  word_t Qracc, Qiacc;

#pragma HLS array_partition variable=kx complete
#pragma HLS array_partition variable=ky complete
#pragma HLS array_partition variable=kz complete
#pragma HLS array_partition variable=phiR complete
#pragma HLS array_partition variable=phiI complete
#pragma HLS array_partition variable=expArg complete
#pragma HLS array_partition variable=phiMag complete
#pragma HLS array_partition variable=cosArg complete
#pragma HLS array_partition variable=sinArg complete
#pragma HLS array_partition variable=Qracc_p complete
#pragma HLS array_partition variable=Qiacc_p complete

 c_label_x:for(unsigned indexX = 0; indexX < BATCH_SIZE_X; indexX++)
    {
      x = _inbuff_x[indexX];
      y = _inbuff_y[indexX];
      z = _inbuff_z[indexX];
      Qracc = 0;
      Qiacc = 0;

    c_label_k:for(unsigned indexK = 0; indexK < NUMK; indexK += PRL)
        {
#pragma HLS pipeline II=1


        c_label_k0:for(unsigned i=0; i<PRL; i++) {
#pragma HLS unroll
      	    unsigned idx = indexK + i;

	    kx[i] =   _inbuff_kx[idx];
	    ky[i] =   _inbuff_ky[idx];
	    kz[i] =   _inbuff_kz[idx];
	    phiR[i] = _inbuff_phiR[idx];
	    phiI[i] = _inbuff_phiI[idx];
          }

        c_label_k1:for(unsigned i=0; i<PRL; i++) {
#pragma HLS unroll
	    phiMag[i] = phiR[i] * phiR[i] + phiI[i] * phiI[i];
	    expArg[i] = PI2 * (kx[i] * x + ky[i] * y + kz[i] * z);
	    cosArg[i] = hls::cos(expArg[i]);
	    sinArg[i] = hls::sin(expArg[i]);
	    Qracc_p[i] = phiMag[i] * cosArg[i];
	    Qiacc_p[i] = phiMag[i] * sinArg[i];
          }

          c_label_k2:for(unsigned i=0; i<PRL; i++) {
#pragma HLS unroll
	    Qracc += Qracc_p[i];
	    Qiacc += Qiacc_p[i];
          }

        } // end of outer k                                                   

      _outbuff_Qr[indexX] = Qracc;
      _outbuff_Qi[indexX] = Qiacc;


    } // end of x                         
