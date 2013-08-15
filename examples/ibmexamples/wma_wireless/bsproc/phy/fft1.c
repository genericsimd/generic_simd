/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "fft.h"
#include "phy_ul_rx_common.h"

#if defined VSXOPT
//#if 0 
#include <altivec.h>


#define NFFT 1024
#define D    0.70710678118654752440

float *aux11, *aux21;
float *WTF1, *WFT1;

/* For 1024-FFT only!!! */
int fft_init_p1(unsigned int N, float ***XX, float ***x, float ***X)
{
    int i,j;
    float *W;
    posix_memalign((void **)&aux11, 16, sizeof(float)*(NFFT)*2);
    posix_memalign((void **)&aux21,  16, sizeof(float)*(NFFT)*2);
    posix_memalign((void **)&W, 16, sizeof(float)*(NFFT)*2);
    posix_memalign((void **)&WTF1,16, sizeof(float)*(NFFT)*2);
    posix_memalign((void **)&WFT1,16, sizeof(float)*(NFFT)*2);

    for (i=0; i<(NFFT)/8; i++) {
      W[2*i  ] = cos(i*2.0*PI/(NFFT));
      W[2*i+1] =-sin(i*2.0*PI/(NFFT));
    }
    W[1*(NFFT/4)  ] = D;
    W[1*(NFFT/4)+1] =-D;
    W[2*(NFFT/4)  ] = 0.;
    W[2*(NFFT/4)+1] =-1.;
    W[3*(NFFT/4)  ] =-D;
    W[3*(NFFT/4)+1] =-D;
    W[4*(NFFT/4)  ] =-1.;
    W[4*(NFFT/4)+1] = 0.;
    W[5*(NFFT/4)  ] =-D;
    W[5*(NFFT/4)+1] = D;
    W[6*(NFFT/4)  ] = 0.;
    W[6*(NFFT/4)+1] = 1.;
    W[7*(NFFT/4)  ] = D;
    W[7*(NFFT/4)+1] = D;

    j = (NFFT)/8 - 1;
    for (i=1; i<(NFFT)/8; i++) {
      W[1*(NFFT/4)+2*i  ] =-W[0*(NFFT/4)+2*j+1];
      W[1*(NFFT/4)+2*i+1] =-W[0*(NFFT/4)+2*j  ];
      W[2*(NFFT/4)+2*i  ] = W[0*(NFFT/4)+2*i+1];
      W[2*(NFFT/4)+2*i+1] =-W[0*(NFFT/4)+2*i  ];
      W[3*(NFFT/4)+2*i  ] =-W[0*(NFFT/4)+2*j  ];
      W[3*(NFFT/4)+2*i+1] = W[0*(NFFT/4)+2*j+1];
      W[4*(NFFT/4)+2*i  ] =-W[0*(NFFT/4)+2*i  ];
      W[4*(NFFT/4)+2*i+1] =-W[0*(NFFT/4)+2*i+1];
      W[5*(NFFT/4)+2*i  ] =-W[1*(NFFT/4)+2*i  ];
      W[5*(NFFT/4)+2*i+1] =-W[1*(NFFT/4)+2*i+1];
      W[6*(NFFT/4)+2*i  ] =-W[2*(NFFT/4)+2*i  ];
      W[6*(NFFT/4)+2*i+1] =-W[2*(NFFT/4)+2*i+1];
      W[7*(NFFT/4)+2*i  ] = W[0*(NFFT/4)+2*j  ];
      W[7*(NFFT/4)+2*i+1] =-W[0*(NFFT/4)+2*j+1];
      j = j-1;
    }
    for (j=0; j<16; j++) {
      for (i=0; i<16; i++) {
        WTF1[128*j+8*i  ] = W[4*(i+1)*j      ];
        WTF1[128*j+8*i+1] = W[4*(i+1)*j      ];
        WTF1[128*j+8*i+2] = W[4*(i+1)*j+2*(i+1)  ];
        WTF1[128*j+8*i+3] = W[4*(i+1)*j+2*(i+1)  ];
        WTF1[128*j+8*i+4] =-W[4*(i+1)*j    +1];
        WTF1[128*j+8*i+5] = W[4*(i+1)*j    +1];
        WTF1[128*j+8*i+6] =-W[4*(i+1)*j+2*(i+1)+1];
        WTF1[128*j+8*i+7] = W[4*(i+1)*j+2*(i+1)+1];
      }
    }

    for (i=0; i<(NFFT)/8; i++) {
      W[2*i  ] = cos(i*2.0*PI/(NFFT));
      W[2*i+1] = sin(i*2.0*PI/(NFFT));
    }
    W[1*(NFFT/4)  ] = D;
    W[1*(NFFT/4)+1] = D;
    W[2*(NFFT/4)  ] = 0.;
    W[2*(NFFT/4)+1] = 1.;
    W[3*(NFFT/4)  ] =-D;
    W[3*(NFFT/4)+1] = D;
    W[4*(NFFT/4)  ] =-1.;
    W[4*(NFFT/4)+1] = 0.;
    W[5*(NFFT/4)  ] =-D;
    W[5*(NFFT/4)+1] =-D;
    W[6*(NFFT/4)  ] = 0.;
    W[6*(NFFT/4)+1] =-1.;
    W[7*(NFFT/4)  ] = D;
    W[7*(NFFT/4)+1] =-D;

    j = (NFFT)/8 - 1;
    for (i=1; i<(NFFT)/8; i++) {
      W[1*(NFFT/4)+2*i  ] = W[0*(NFFT/4)+2*j+1];
      W[1*(NFFT/4)+2*i+1] = W[0*(NFFT/4)+2*j  ];
      W[2*(NFFT/4)+2*i  ] =-W[0*(NFFT/4)+2*i+1];
      W[2*(NFFT/4)+2*i+1] = W[0*(NFFT/4)+2*i  ];
      W[3*(NFFT/4)+2*i  ] =-W[0*(NFFT/4)+2*j  ];
      W[3*(NFFT/4)+2*i+1] = W[0*(NFFT/4)+2*j+1];
      W[4*(NFFT/4)+2*i  ] =-W[0*(NFFT/4)+2*i  ];
      W[4*(NFFT/4)+2*i+1] =-W[0*(NFFT/4)+2*i+1];
      W[5*(NFFT/4)+2*i  ] =-W[1*(NFFT/4)+2*i  ];
      W[5*(NFFT/4)+2*i+1] =-W[1*(NFFT/4)+2*i+1];
      W[6*(NFFT/4)+2*i  ] =-W[2*(NFFT/4)+2*i  ];
      W[6*(NFFT/4)+2*i+1] =-W[2*(NFFT/4)+2*i+1];
      W[7*(NFFT/4)+2*i  ] = W[0*(NFFT/4)+2*j  ];
      W[7*(NFFT/4)+2*i+1] =-W[0*(NFFT/4)+2*j+1];
      j = j-1;
    }
    for (j=0; j<16; j++) {
      for (i=0; i<16; i++) {
        WFT1[128*j+8*i  ] = 1.0/(NFFT) *W[4*i*j];
        WFT1[128*j+8*i+1] = 1.0/(NFFT) *W[4*i*j];
        WFT1[128*j+8*i+2] = 1.0/(NFFT) *W[4*i*j+i];
        WFT1[128*j+8*i+3] = 1.0/(NFFT) *W[4*i*j+i];
        WFT1[128*j+8*i+4] =-1.0/(NFFT) *W[4*i*j  +1];
        WFT1[128*j+8*i+5] = 1.0/(NFFT) *W[4*i*j  +1];
        WFT1[128*j+8*i+6] =-1.0/(NFFT) *W[4*i*j+i+1];
        WFT1[128*j+8*i+7] = 1.0/(NFFT) *W[4*i*j+i+1];
      }
    }
    free(W);
    return 0;
}

void fft_quit_p1(float **XX, float **x, float **X)
{
    /* Free memory. */
    free(aux11);
    free(aux21);
    free(WTF1);
    free(WFT1);
}

void fft_p1(unsigned int N, float (*x_r), float (*x_i), float (*X_R), float (*X_I),float **XX, float **x, float **X)
{
    unsigned int i, j;
    vector float xr, xi, x1, x2;
    vector unsigned char vmask1 = {0x00, 0x01, 0x02, 0x03, 0x08, 0x09, 0x0A, 0x0B, 0x10, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1A, 0x1B};
    vector unsigned char vmask2 = {0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F, 0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F};
    // TODO: Check performance whether we need more unroll instead of auto-unroll by compiler.
#ifdef CHECK_ALIGNMENT
    if( 0x0f & (unsigned long)x_r)
         printf(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  x_r  unaligned \n" );
    if( 0x0f & (unsigned long)x_i)
         printf(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!   x_i  unaligned \n" );
    if( 0x0f & (unsigned long)X_R)
         printf(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!   X_R  unaligned \n" );
    if( 0x0f & (unsigned long)X_I)
         printf(" !!!!!!!!!!!!!!!!!!!!!!!!!!!!!   X_I  unaligned \n" );
#endif
   
#if defined PERF
struct timeval timev1, timev2;
int elapsed_microsecs;
double elapsed_time;
gettimeofday(&timev1, NULL);
for (j = 0; j < 100; j++) {
#endif
    for (i=0;i<N/4;i++) {
        xr = vec_ld(0, &x_r[i*4]);
        xi = vec_ld(0, &x_i[i*4]);
        x1 = vec_mergeh(xr, xi);
        x2 = vec_mergel(xr, xi);
        vec_st(x1, 0, &aux11[i*8]);
        vec_st(x2,16, &aux11[i*8]);
    }


    /* Calculate FFT by a recursion. */
    fft_32_s1(1, 1.0, aux11, aux21, WTF1);
    fft_32_s2(1, aux21, aux11);

    for (i=0;i<N/4;i++) {
        x1 = vec_ld( 0, &aux11[i*8]);
        x2 = vec_ld(16, &aux11[i*8]);
        xr = vec_perm(x1, x2, vmask1);
        xi = vec_perm(x1, x2, vmask2);
        vec_st(xr, 0, &X_R[i*4]);
        vec_st(xi, 0, &X_I[i*4]);
    }
#if defined PERF
}
gettimeofday(&timev2, NULL);
elapsed_microsecs = (int)(timev2.tv_sec  - timev1.tv_sec) * 1000000 + (int)(timev2.tv_usec - timev1.tv_usec);
elapsed_time = elapsed_microsecs * 0.000001;
printf("FFT Perf: %10.4f GFLops, Time: %10.6f s\n", 5.0*1024*10*100*0.000000001/elapsed_time, elapsed_time);
#endif
}

#if 1
/* IFFT */
void ifft_p1(unsigned int N, float (*x_r), float (*x_i), float (*X_R), float (*X_I), float **XX, float **x, float **X)
{
    unsigned int i, j;
    vector float xr, xi, x1, x2;
    vector unsigned char vmask1 = {0x00, 0x01, 0x02, 0x03, 0x08, 0x09, 0x0A, 0x0B, 0x10, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1A, 0x1B};
    vector unsigned char vmask2 = {0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F, 0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F};
    // TODO: Check performance whether we need more unroll instead of auto-unroll by compiler.
#ifdef CHECK_ALIGNMENT
    if( 0x0f & (unsigned long)x_r)
         printf(" ===!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  x_r  unaligned \n" );
    if( 0x0f & (unsigned long)x_i)
         printf("=== !!!!!!!!!!!!!!!!!!!!!!!!!!!!!   x_i  unaligned \n" );
    if( 0x0f & (unsigned long)X_R)
         printf("=== !!!!!!!!!!!!!!!!!!!!!!!!!!!!!   X_R  unaligned \n" );
    if( 0x0f & (unsigned long)X_I)
         printf("=== !!!!!!!!!!!!!!!!!!!!!!!!!!!!!   X_I  unaligned \n" );
#endif
#if defined PERF
struct timeval timev1, timev2;
int elapsed_microsecs;
double elapsed_time;
gettimeofday(&timev1, NULL);
for (j = 0; j < 100; j++) {
#endif
    for (i=0;i<N/4;i++) {
        xr = vec_ld(0, &X_R[i*4]);
        xi = vec_ld(0, &X_I[i*4]);
        x1 = vec_mergeh(xr, xi);
        x2 = vec_mergel(xr, xi);
        vec_st(x1, 0, &aux11[i*8]);
        vec_st(x2,16, &aux11[i*8]);
    }

    /* Calculate FFT by a recursion. */
    fft_32_s1(-1, 1.0/(NFFT), aux11, aux21, WFT1);
    fft_32_s2(-1, aux21, aux11);

    for (i=0;i<N/4;i++) {
        x1 = vec_ld( 0, &aux11[i*8]);
        x2 = vec_ld(16, &aux11[i*8]);
        xr = vec_perm(x1, x2, vmask1);
        xi = vec_perm(x1, x2, vmask2);
        vec_st(xr, 0, &x_r[i*4]);
        vec_st(xi, 0, &x_i[i*4]);
    }
#if defined PERF
}
gettimeofday(&timev2, NULL);
elapsed_microsecs = (int)(timev2.tv_sec  - timev1.tv_sec) * 1000000 + (int)(timev2.tv_usec - timev1.tv_usec);
elapsed_time = elapsed_microsecs * 0.000001;
printf("IFFT Perf: %10.4f GFLops, Time: %10.6f s\n", 5.0*1024*10*100*0.000000001/elapsed_time, elapsed_time);
#endif
}
//#else
#endif

#endif

