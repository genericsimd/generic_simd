/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __FFT_H__
#define __FFT_H__

#define PI     (3.1415926535897932384626433832795)
#define TWO_PI (6.283185307179586476925286766558)

int32_t fft_init(u_int32_t N, float ***XX, float ***x, float ***X);
void fft_quit(float **XX, float **x, float **X);
void fft_rec(u_int32_t N, int32_t offset, int32_t delta,float **x, float **X, float **XX);
void ifft(u_int32_t N, float (*x_r), float (*x_i), float (*X_R), float (*X_I),float **XX, float **x, float **X);
void fft(u_int32_t  N, float (*x_r), float (*x_i), float (*X_R), float (*X_I),float **XX, float **x, float **X);


#endif //__FFT_H__

