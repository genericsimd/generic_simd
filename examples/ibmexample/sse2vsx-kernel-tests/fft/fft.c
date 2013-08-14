/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#define PERF

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "fft.h"

#if defined PERF
#include <sys/time.h>
#endif

#ifdef SSE2OPT 
#include "ipp.h"
#include "ipps.h"

IppsFFTSpec_C_32f* pFFTSpecInv_ul;
Ipp8u* BufInv_ul;
IppsFFTSpec_C_32f* pFFTSpecFwd_ul;
Ipp8u* BufFwd_ul;

IppsFFTSpec_C_32f* g_pFFTSpecInv;
Ipp8u* g_BufInv;

int fft_init(unsigned int N, float ***XX, float ***x, float ***X)
{
    ippsFFTInitAlloc_C_32f(&pFFTSpecFwd_ul, (int)(log(1024)/log(2)), IPP_FFT_NODIV_BY_ANY, ippAlgHintFast);
    int BufSizeFwd;
    ippsFFTGetBufSize_C_32f(pFFTSpecFwd_ul, &BufSizeFwd);
    BufFwd_ul = malloc(BufSizeFwd*sizeof(Ipp8u));
    if(BufFwd_ul == NULL)
    {
        printf("malloc failed in fft_init\n");
        return -1;
    }

    ippsFFTInitAlloc_C_32f(&g_pFFTSpecInv,
                           (int32_t)(log(1024)/log(2)),
                           IPP_FFT_DIV_INV_BY_N,
                           ippAlgHintFast);
    int32_t BufSize;
    ippsFFTGetBufSize_C_32f(g_pFFTSpecInv, &BufSize);
    g_BufInv = malloc(BufSize*sizeof(Ipp8u));
    if ( g_BufInv == NULL)
    {
        printf("NULL PTR of malloc in function ifft_init\n");
        return 1;
    }


}


void fft(unsigned int N, float (*x_r), float (*x_i), float (*X_R), float (*X_I),float **XX, float **x, float **X)
{
#if defined PERF
  int j;
  struct timeval timev1, timev2;
  int elapsed_microsecs;
  double elapsed_time;
  gettimeofday(&timev1, NULL);
  for (j = 0; j < 100; j++) {
#endif

    ippsFFTFwd_CToC_32f(x_r, x_i, X_R, X_I, pFFTSpecFwd_ul, BufFwd_ul);

#if defined PERF
  }
  gettimeofday(&timev2, NULL);
  elapsed_microsecs = (int)(timev2.tv_sec  - timev1.tv_sec) * 1000000 + (int)(timev2.tv_usec - timev1.tv_usec);
  elapsed_time = elapsed_microsecs * 0.000001;
  printf("FFT Perf: %10.4f GFLops, Time: %10.6f s\n", 5.0*1024*10*100*0.000000001/elapsed_time, elapsed_time);
#endif

}

void ifft(unsigned int N, float (*x_r), float (*x_i), float (*X_R), float (*X_I), float **XX, float **x, float **X)
{
#if defined PERF
  int j;
  struct timeval timev1, timev2;
  int elapsed_microsecs;
  double elapsed_time;
  gettimeofday(&timev1, NULL);
  for (j = 0; j < 100; j++) {
#endif
    ippsFFTInv_CToC_32f(X_R, X_I, x_r, x_i, g_pFFTSpecInv, g_BufInv);
#if defined PERF
  }
  gettimeofday(&timev2, NULL);
  elapsed_microsecs = (int)(timev2.tv_sec  - timev1.tv_sec) * 1000000 + (int)(timev2.tv_usec - timev1.tv_usec);
  elapsed_time = elapsed_microsecs * 0.000001;
  printf("IFFT Perf: %10.4f GFLops, Time: %10.6f s\n", 5.0*1024*10*100*0.000000001/elapsed_time, elapsed_time);
#endif
}

void fft_quit(float **XX, float **x, float **X)
{

    ippsFFTFree_C_32f(pFFTSpecFwd_ul);
    if(BufFwd_ul!=NULL)
    {
        free(BufFwd_ul);
        BufFwd_ul = NULL;
    }


    ippsFFTFree_C_32f(g_pFFTSpecInv);
    if(g_BufInv!=NULL)
    {
        free(g_BufInv);
        g_BufInv = NULL;
    }

}
#elif defined VSXOPT
#include <stdlib.h>
#include <altivec.h>


#define NFFT 1024
#define D    0.70710678118654752440

float *aux1, *aux2;
float *WTF, *WFT;

/* For 1024-FFT only!!! */
int fft_init(unsigned int N, float ***XX, float ***x, float ***X)
{
    int i,j;
    float *W;
    posix_memalign((void **)&aux1, 16, sizeof(float)*(NFFT)*2);
    posix_memalign((void **)&aux2,  16, sizeof(float)*(NFFT)*2);
    posix_memalign((void **)&W, 16, sizeof(float)*(NFFT)*2);
    posix_memalign((void **)&WTF,16, sizeof(float)*(NFFT)*2);
    posix_memalign((void **)&WFT,16, sizeof(float)*(NFFT)*2);

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
        WTF[128*j+8*i  ] = W[4*(i+1)*j      ];
        WTF[128*j+8*i+1] = W[4*(i+1)*j      ];
        WTF[128*j+8*i+2] = W[4*(i+1)*j+2*(i+1)  ];
        WTF[128*j+8*i+3] = W[4*(i+1)*j+2*(i+1)  ];
        WTF[128*j+8*i+4] =-W[4*(i+1)*j    +1];
        WTF[128*j+8*i+5] = W[4*(i+1)*j    +1];
        WTF[128*j+8*i+6] =-W[4*(i+1)*j+2*(i+1)+1];
        WTF[128*j+8*i+7] = W[4*(i+1)*j+2*(i+1)+1];
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
        WFT[128*j+8*i  ] = 1.0/(NFFT) *W[4*i*j];
        WFT[128*j+8*i+1] = 1.0/(NFFT) *W[4*i*j];
        WFT[128*j+8*i+2] = 1.0/(NFFT) *W[4*i*j+i];
        WFT[128*j+8*i+3] = 1.0/(NFFT) *W[4*i*j+i];
        WFT[128*j+8*i+4] =-1.0/(NFFT) *W[4*i*j  +1];
        WFT[128*j+8*i+5] = 1.0/(NFFT) *W[4*i*j  +1];
        WFT[128*j+8*i+6] =-1.0/(NFFT) *W[4*i*j+i+1];
        WFT[128*j+8*i+7] = 1.0/(NFFT) *W[4*i*j+i+1];
      }
    }
    free(W);
    return 0;
}

void fft_quit(float **XX, float **x, float **X)
{
    /* Free memory. */
    free(aux1);
    free(aux2);
    free(WTF);
    free(WFT);
}

/* A modified split-radix FFT with fewer arithmetic operations. Steven G. Johnson & Matteo Frigo, 2007 */
void fft_32_s1(int isign, float scale, float *in, float *out, float *w)
{
    int i, j;
    int I0, I1, I2, I3, I4, I5, I6, I7, I8, I9;
    int I10, I11, I12, I13, I14, I15, I16, I17, I18, I19;
    int I20, I21, I22, I23, I24, I25, I26, I27, I28, I29;
    int I30, I31;
    vector float x0, x1, x2, x3, x4, x5, x6, x7, x8, x9;
    vector float x10, x11, x12, x13, x14, x15, x16, x17, x18, x19;
    vector float x20, x21, x22, x23, x24, x25, x26, x27, x28, x29;
    vector float x30, x31;
    vector float y0,y1,y2,y3,y4,y5,y6,y7,y8,y9;
    vector float y10,y11,y12,y13,y14,y15,y16,y17,y18,y19;
    vector float y20,y21,y22,y23,y24,y25,y26,y27,y28,y29;
    vector float y30,y31;
    vector float xx0,xx1,xx2,xx3,xx4,xx5,xx6,xx7,xx8,xx9;
    vector float xx10,xx11,xx12,xx13,xx14,xx15;
    vector float a0,a1,a2,a3,a01,a11,a21,a31,a41,a51,a61,a71;
    vector float b0,b1,b2,b3,b01,b11,b21,b31,b41,b51,b61,b71;
    vector float a00,a10,a20,a30,a40,a50,a60,a70;
    vector float b00,b10,b20,b30,b40,b50,b60,b70,b80,b90;
    vector float b100,b110,b120,b130,b140,b150;
    vector float z11,z31,z12,z32,t11,t31,t12,t32;
    vector float z111,z112,z113,z114,z121,z122,z123,z124;
    vector float v11,v21,v31,v41,v51,v61,v71,v81;
    vector float v12,v22,v32,v42,v52,v62,v72,v82;
    vector float v111,v112,v113,v114,v121,v122,v123,v124;
    vector float w1r,w2r,w3r,w4r,w5r,w6r,w7r,w8r;
    vector float w9r,w10r,w11r,w12r,w13r,w14r,w15r,w16r;
    vector float w1i,w2i,w3i,w4i,w5i,w6i,w7i,w8i;
    vector float w9i,w10i,w11i,w12i,w13i,w14i,w15i,w16i;
    vector float d0,d1,d2,d3;
    vector float vSIGN;
    vector float vSIGN1= {-1.0, 1.0,-1.0, 1.0};
    vector float vSIGN2= { 1.0,-1.0, 1.0,-1.0};
    vector float vSCALE;
    vector float vC1 = {0.98078528040323044913, 0.98078528040323044913, 0.98078528040323044913, 0.98078528040323044913};
    vector float vC2 = {0.92387953251128675613, 0.92387953251128675613, 0.92387953251128675613, 0.92387953251128675613};
    vector float vC3 = {0.83146961230254523708, 0.83146961230254523708, 0.83146961230254523708, 0.83146961230254523708};
    vector float vC4 = {0.70710678118654752440, 0.70710678118654752440, 0.70710678118654752440, 0.70710678118654752440};
    vector float vT1 = {0.19891236737965800691, 0.19891236737965800691, 0.19891236737965800691, 0.19891236737965800691};
    vector float vT2 = {0.41421356237309504880, 0.41421356237309504880, 0.41421356237309504880, 0.41421356237309504880};
    vector float vT3 = {0.66817863791929892000, 0.66817863791929892000, 0.66817863791929892000, 0.66817863791929892000};
    vector unsigned char vmaskx = {0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x1C, 0x1D, 0x1E, 0x1F, 0x18, 0x19, 0x1A, 0x1B};

    if (isign >= 0) {
        vSIGN = vSIGN1;
    } else {
        vSIGN = vSIGN2;
    }
    vSCALE = (vector float){scale, scale, scale, scale};

    I0 = 0;
    I1 = I0 + 32*sizeof(float)*2;
    I2 = I1 + 32*sizeof(float)*2;
    I3 = I2 + 32*sizeof(float)*2;
    I4 = I3 + 32*sizeof(float)*2;
    I5 = I4 + 32*sizeof(float)*2;
    I6 = I5 + 32*sizeof(float)*2;
    I7 = I6 + 32*sizeof(float)*2;
    I8 = I7 + 32*sizeof(float)*2;
    I9 = I8 + 32*sizeof(float)*2;
    I10 = I9 + 32*sizeof(float)*2;
    I11 = I10 + 32*sizeof(float)*2;
    I12 = I11 + 32*sizeof(float)*2;
    I13 = I12 + 32*sizeof(float)*2;
    I14 = I13 + 32*sizeof(float)*2;
    I15 = I14 + 32*sizeof(float)*2;
    I16 = I15 + 32*sizeof(float)*2;
    I17 = I16 + 32*sizeof(float)*2;
    I18 = I17 + 32*sizeof(float)*2;
    I19 = I18 + 32*sizeof(float)*2;
    I20 = I19 + 32*sizeof(float)*2;
    I21 = I20 + 32*sizeof(float)*2;
    I22 = I21 + 32*sizeof(float)*2;
    I23 = I22 + 32*sizeof(float)*2;
    I24 = I23 + 32*sizeof(float)*2;
    I25 = I24 + 32*sizeof(float)*2;
    I26 = I25 + 32*sizeof(float)*2;
    I27 = I26 + 32*sizeof(float)*2;
    I28 = I27 + 32*sizeof(float)*2;
    I29 = I28 + 32*sizeof(float)*2;
    I30 = I29 + 32*sizeof(float)*2;
    I31 = I30 + 32*sizeof(float)*2;

    for (j = 0; j < 32; j+=2) {
        x0 = vec_ld(I0, &in[2*j]);
        x1 = vec_ld(I1, &in[2*j]);
        x2 = vec_ld(I2, &in[2*j]);
        x3 = vec_ld(I3, &in[2*j]);
        x4 = vec_ld(I4, &in[2*j]);
        x5 = vec_ld(I5, &in[2*j]);
        x6 = vec_ld(I6, &in[2*j]);
        x7 = vec_ld(I7, &in[2*j]);
        x8 = vec_ld(I8, &in[2*j]);
        x9 = vec_ld(I9, &in[2*j]);
        x10 = vec_ld(I10, &in[2*j]);
        x11 = vec_ld(I11, &in[2*j]);
        x12 = vec_ld(I12, &in[2*j]);
        x13 = vec_ld(I13, &in[2*j]);
        x14 = vec_ld(I14, &in[2*j]);
        x15 = vec_ld(I15, &in[2*j]);
        x16 = vec_ld(I16, &in[2*j]);
        x17 = vec_ld(I17, &in[2*j]);
        x18 = vec_ld(I18, &in[2*j]);
        x19 = vec_ld(I19, &in[2*j]);
        x20 = vec_ld(I20, &in[2*j]);
        x21 = vec_ld(I21, &in[2*j]);
        x22 = vec_ld(I22, &in[2*j]);
        x23 = vec_ld(I23, &in[2*j]);
        x24 = vec_ld(I24, &in[2*j]);
        x25 = vec_ld(I25, &in[2*j]);
        x26 = vec_ld(I26, &in[2*j]);
        x27 = vec_ld(I27, &in[2*j]);
        x28 = vec_ld(I28, &in[2*j]);
        x29 = vec_ld(I29, &in[2*j]);
        x30 = vec_ld(I30, &in[2*j]);
        x31 = vec_ld(I31, &in[2*j]);

        xx0 = vec_add(x0,x16);
        xx8 = vec_add(x8,x24);
        a00 = vec_add(xx0,xx8);
        b00 = vec_sub(xx0,xx8);
        xx4 = vec_add(x4,x20);
        xx12 = vec_add(x12,x28);
        a40 = vec_add(xx4,xx12);
        b40 = vec_sub(xx4,xx12);
        a0 = vec_add(a00,a40);
        b0 = vec_sub(a00,a40);

        xx2 = vec_add(x2,x18);
        xx10 = vec_add(x10,x26);
        a20 = vec_add(xx2,xx10);
        b20 = vec_sub(xx2,xx10);
        xx6 = vec_add(x6,x22);
        xx14 = vec_add(x14,x30);
        a60 = vec_add(xx6,xx14);
        b60 = vec_sub(xx6,xx14);
        a2 = vec_add(a20,a60);
        b2 = vec_sub(a20,a60);
        a01 = vec_add(a0,a2);
        b01 = vec_sub(a0,a2);

        xx1 = vec_add(x1,x17);
        xx9 = vec_add(x9,x25);
        a10 = vec_add(xx1,xx9);
        b10 = vec_sub(xx1,xx9);
        xx7 = vec_add(x7,x23);
        xx15 = vec_add(x15,x31);
        a70 = vec_add(xx7,xx15);
        b70 = vec_sub(xx7,xx15);

        xx3 = vec_add(x3,x19);
        xx11 = vec_add(x11,x27);
        a30 = vec_add(xx3,xx11);
        b30 = vec_sub(xx3,xx11);
        xx5 = vec_add(x5,x21);
        xx13 = vec_add(x13,x29);
        a50 = vec_add(xx5,xx13);
        b50 = vec_sub(xx5,xx13);
        a1 = vec_add(a10,a50);
        b1 = vec_sub(a10,a50);
        a3 = vec_add(a30,a70);
        b3 = vec_sub(a30,a70);
        a11 = vec_add(a1,a3);
        b11 = vec_sub(a1,a3);
        b11 = vec_perm(b11,b11,vmaskx);

        y0 = vec_add(a01,a11);
        y16 = vec_sub(a01,a11);
        y8 = vec_nmsub(b11,vSIGN,b01);
        y24 = vec_madd(b11,vSIGN,b01);

        w8r = vec_ld(224,&w[64*j]);
        w8i = vec_ld(240,&w[64*j]);
        w16r = vec_ld(480,&w[64*j]);
        w16i = vec_ld(496,&w[64*j]);
        d0 = vec_perm(y8,y8,vmaskx);
        d1 = vec_perm(y16,y16,vmaskx);
        d2 = vec_perm(y24,y24,vmaskx);
        y0 = vec_mul (y0,vSCALE);
        y8 = vec_mul (y8,w8r);
        y8 = vec_madd(d0,w8i,y8);
        y16 = vec_mul (y16,w16r);
        y16 = vec_madd(d1,w16i,y16);
        y24 = vec_mul (y24,w8r);
        y24 = vec_nmsub(d2,w8i,y24);
        vec_st(y0,I0,&out[2*j]);
        vec_st(y8,I8,&out[2*j]);
        vec_st(y16,I16,&out[2*j]);
        vec_st(y24,I24,&out[2*j]);

        a11 = vec_sub(b1,b3);
        b11 = vec_add(b1,b3);

        v21 = vec_madd(a11,vC4,b0);
        v61 = vec_nmsub(a11,vC4,b0);
        v22 = vec_madd(b11,vC4,b2);
        v22 = vec_perm(v22,v22,vmaskx);
        v62 = vec_nmsub(b11,vC4,b2);
        v62 = vec_perm(v62,v62,vmaskx);

        y4 = vec_nmsub(v22,vSIGN,v21);
        y28 = vec_madd(v22,vSIGN,v21);
        y12 = vec_madd(v62,vSIGN,v61);
        y20 = vec_nmsub(v62,vSIGN,v61);

        w4r = vec_ld( 96,&w[64*j]);
        w4i = vec_ld(112,&w[64*j]);
        w12r = vec_ld(352,&w[64*j]);
        w12i = vec_ld(368,&w[64*j]);
        d0 = vec_perm(y4,y4,vmaskx);
        d1 = vec_perm(y12,y12,vmaskx);
        d2 = vec_perm(y20,y20,vmaskx);
        d3 = vec_perm(y28,y28,vmaskx);
        y4 = vec_mul (y4,w4r);
        y4 = vec_madd(d0,w4i,y4);
        y12 = vec_mul (y12,w12r);
        y12 = vec_madd(d1,w12i,y12);
        y20 = vec_mul (y20,w12r);
        y20 = vec_nmsub(d2,w12i,y20);
        y28 = vec_mul (y28,w4r);
        y28 = vec_nmsub(d3,w4i,y28);
        vec_st(y4,I4,&out[2*j]);
        vec_st(y12,I12,&out[2*j]);
        vec_st(y20,I20,&out[2*j]);
        vec_st(y28,I28,&out[2*j]);

        a1 = vec_sub(b10,b70);
        b1 = vec_add(b10,b70);
        a2 = vec_sub(b20,b60);
        b2 = vec_add(b20,b60);
        a3 = vec_sub(b30,b50);
        b3 = vec_add(b30,b50);

        z11 = vec_madd(a2,vC4,b00);
        z31 = vec_nmsub(a2,vC4,b00);
        z12 = vec_madd(b2,vC4,b40);
        z12 = vec_perm(z12,z12,vmaskx);
        z32 = vec_nmsub(b2,vC4,b40);
        z32 = vec_perm(z32,z32,vmaskx);
        t11 = vec_madd(a3,vT2,a1);
        t31 = vec_nmsub(a1,vT2,a3);
        t12 = vec_madd(b1,vT2,b3);
        t12 = vec_perm(t12,t12,vmaskx);
        t32 = vec_nmsub(b3,vT2,b1);
        t32 = vec_perm(t32,t32,vmaskx);

        v11 = vec_madd(t11,vC2,z11);
        v71 = vec_nmsub(t11,vC2,z11);
        v12 = vec_madd(t12,vC2,z12);
        v72 = vec_nmsub(t12,vC2,z12);

        y2 = vec_nmsub(v12,vSIGN,v11);
        y30 = vec_madd(v12,vSIGN,v11);
        y14 = vec_madd(v72,vSIGN,v71);
        y18 = vec_nmsub(v72,vSIGN,v71);

        w2r = vec_ld( 32,&w[64*j]);
        w2i = vec_ld( 48,&w[64*j]);
        w14r = vec_ld(416,&w[64*j]);
        w14i = vec_ld(432,&w[64*j]);
        d0 = vec_perm(y2,y2,vmaskx);
        d1 = vec_perm(y14,y14,vmaskx);
        d2 = vec_perm(y18,y18,vmaskx);
        d3 = vec_perm(y30,y30,vmaskx);
        y2 = vec_mul (y2,w2r);
        y2 = vec_madd(d0,w2i,y2);
        y14 = vec_mul (y14,w14r);
        y14 = vec_madd(d1,w14i,y14);
        y18 = vec_mul (y18,w14r);
        y18 = vec_nmsub(d2,w14i,y18);
        y30 = vec_mul (y30,w2r);
        y30 = vec_nmsub(d3,w2i,y30);
        vec_st(y2,I2,&out[2*j]);
        vec_st(y14,I14,&out[2*j]);
        vec_st(y18,I18,&out[2*j]);
        vec_st(y30,I30,&out[2*j]);

        v31 = vec_nmsub(t31,vC2,z31);
        v51 = vec_madd(t31,vC2,z31);
        v32 = vec_nmsub(t32,vC2,z32);
        v52 = vec_madd(t32,vC2,z32);

        y6 = vec_madd(v32,vSIGN,v31);
        y26 = vec_nmsub(v32,vSIGN,v31);
        y10 = vec_nmsub(v52,vSIGN,v51);
        y22 = vec_madd(v52,vSIGN,v51);

        w6r = vec_ld(160,&w[64*j]);
        w6i = vec_ld(176,&w[64*j]);
        w10r = vec_ld(288,&w[64*j]);
        w10i = vec_ld(304,&w[64*j]);
        d0 = vec_perm(y6,y6,vmaskx);
        d1 = vec_perm(y10,y10,vmaskx);
        d2 = vec_perm(y22,y22,vmaskx);
        d3 = vec_perm(y26,y26,vmaskx);
        y6 = vec_mul (y6,w6r);
        y6 = vec_madd(d0,w6i,y6);
        y10 = vec_mul (y10,w10r);
        y10 = vec_madd(d1,w10i,y10);
        y22 = vec_mul (y22,w10r);
        y22 = vec_nmsub(d2,w10i,y22);
        y26 = vec_mul (y26,w6r);
        y26 = vec_nmsub(d3,w6i,y26);
        vec_st(y6,I6,&out[2*j]);
        vec_st(y10,I10,&out[2*j]);
        vec_st(y22,I22,&out[2*j]);
        vec_st(y26,I26,&out[2*j]);

        b40 = vec_sub(x4,x20);
        b120 = vec_sub(x12,x28);
        a11 = vec_sub(b40,b120);
        b11 = vec_add(b40,b120);

        b20 = vec_sub(x2,x18);
        b140 = vec_sub(x14,x30);
        a21 = vec_sub(b20,b140);
        b21 = vec_add(b20,b140);

        b60 = vec_sub(x6,x22);
        b100 = vec_sub(x10,x26);
        a31 = vec_sub(b60,b100);
        b31 = vec_add(b60,b100);

        b00 = vec_sub(x0,x16);
        b80 = vec_sub(x8,x24);

        b10 = vec_sub(x1,x17);
        b150 = vec_sub(x15,x31);
        a41 = vec_sub(b10,b150);
        b41 = vec_add(b10,b150);

        b70 = vec_sub(x7,x23);
        b90 = vec_sub(x9,x25);
        a51 = vec_sub(b70,b90);
        b51 = vec_add(b70,b90);

        b30 = vec_sub(x3,x19);
        b130 = vec_sub(x13,x29);
        a61 = vec_sub(b30,b130);
        b61 = vec_add(b30,b130);

        b50 = vec_sub(x5,x21);
        b110 = vec_sub(x11,x27);
        a71 = vec_sub(b50,b110);
        b71 = vec_add(b50,b110);

        v111 = vec_madd(a11,vC4,b00);
        z111 = vec_nmsub(a11,vC4,b00);
        v112 = vec_madd(a31,vT2,a21);
        z112 = vec_nmsub(a21,vT2,a31);
        xx0 = vec_add(a41,a51);
        xx1 = vec_sub(a41,a51);
        v113 = vec_madd(xx0,vC4,a61);
        z113 = vec_nmsub(xx0,vC4,a61);
        v114 = vec_madd(xx1,vC4,a71);
        z114 = vec_nmsub(xx1,vC4,a71);

        xx0 = vec_madd(v112,vC2,v111);
        xx1 = vec_nmsub(v112,vC2,v111);
        xx2 = vec_madd(v114,vT3,v113);
        xx3 = vec_nmsub(v113,vT3,v114);
        v11 = vec_madd(xx2,vC3,xx0);
        v21 = vec_nmsub(xx2,vC3,xx0);
        v31 = vec_madd(xx3,vC3,xx1);
        v41 = vec_nmsub(xx3,vC3,xx1);

        xx0 = vec_madd(z112,vC2,z111);
        xx1 = vec_nmsub(z112,vC2,z111);
        xx2 = vec_madd(z113,vT1,z114);
        xx3 = vec_nmsub(z114,vT1,z113);
        v51 = vec_nmsub(xx2,vC1,xx1);
        v61 = vec_madd(xx2,vC1,xx1);
        v71 = vec_nmsub(xx3,vC1,xx0);
        v81 = vec_madd(xx3,vC1,xx0);

        v121 = vec_madd(b11,vC4,b80);
        v121 = vec_perm(v121,v121,vmaskx);
        z121 = vec_nmsub(b11,vC4,b80);
        z121 = vec_perm(z121,z121,vmaskx);
        v122 = vec_madd(b21,vT2,b31);
        v122 = vec_perm(v122,v122,vmaskx);
        z122 = vec_nmsub(b31,vT2,b21);
        z122 = vec_perm(z122,z122,vmaskx);

        xx0 = vec_add(b41,b51);
        xx1 = vec_sub(b41,b51);
        v123 = vec_madd(xx0,vC4,b71);
        v123 = vec_perm(v123,v123,vmaskx);
        z123 = vec_nmsub(xx0,vC4,b71);
        z123 = vec_perm(z123,z123,vmaskx);
        v124 = vec_nmsub(xx1,vC4,b61);
        v124 = vec_perm(v124,v124,vmaskx);
        z124 = vec_madd(xx1,vC4,b61);
        z124 = vec_perm(z124,z124,vmaskx);

        xx0 = vec_madd(v122,vC2,v121);
        xx1 = vec_nmsub(v122,vC2,v121);
        xx2 = vec_madd(v124,vT3,v123);
        xx3 = vec_nmsub(v123,vT3,v124);
        v12 = vec_madd(xx2,vC3,xx0);
        v22 = vec_nmsub(xx2,vC3,xx0);
        v32 = vec_madd(xx3,vC3,xx1);
        v42 = vec_nmsub(xx3,vC3,xx1);

        xx0 = vec_madd(z122,vC2,z121);
        xx1 = vec_nmsub(z122,vC2,z121);
        xx2 = vec_madd(z123,vT1,z124);
        xx3 = vec_nmsub(z124,vT1,z123);
        v52 = vec_nmsub(xx2,vC1,xx1);
        v62 = vec_madd(xx2,vC1,xx1);
        v72 = vec_nmsub(xx3,vC1,xx0);
        v82 = vec_madd(xx3,vC1,xx0);

        w1r = vec_ld(  0,&w[64*j]);
        w1i = vec_ld( 16,&w[64*j]);
        w3r = vec_ld( 64,&w[64*j]);
        w3i = vec_ld( 80,&w[64*j]);
        y1 = vec_nmsub(v12,vSIGN,v11);
        y3 = vec_madd(v52,vSIGN,v51);
        y29 = vec_nmsub(v52,vSIGN,v51);
        y31 = vec_madd(v12,vSIGN,v11);
        x1 = vec_perm(y1,y1,vmaskx);
        x3 = vec_perm(y3,y3,vmaskx);
        x29 = vec_perm(y29,y29,vmaskx);
        x31 = vec_perm(y31,y31,vmaskx);
        y1 = vec_mul (y1,w1r);
        y1 = vec_madd(x1,w1i,y1);
        y3 = vec_mul (y3,w3r);
        y3 = vec_madd(x3,w3i,y3);
        y29 = vec_mul (y29,w3r);
        y29 = vec_nmsub(x29,w3i,y29);
        y31 = vec_mul (y31,w1r);
        y31 = vec_nmsub(x31,w1i,y31);
        vec_st(y1,I1,&out[2*j]);
        vec_st(y3,I3,&out[2*j]);
        vec_st(y29,I29,&out[2*j]);
        vec_st(y31,I31,&out[2*j]);

        w5r = vec_ld(128,&w[64*j]);
        w5i = vec_ld(144,&w[64*j]);
        w7r = vec_ld(192,&w[64*j]);
        w7i = vec_ld(208,&w[64*j]);
        y5 = vec_nmsub(v72,vSIGN,v71);
        y7 = vec_madd(v32,vSIGN,v31);
        y25 = vec_nmsub(v32,vSIGN,v31);
        y27 = vec_madd(v72,vSIGN,v71);
        x5 = vec_perm(y5,y5,vmaskx);
        x7 = vec_perm(y7,y7,vmaskx);
        x25 = vec_perm(y25,y25,vmaskx);
        x27 = vec_perm(y27,y27,vmaskx);
        y5 = vec_mul (y5,w5r);
        y5 = vec_madd(x5,w5i,y5);
        y7 = vec_mul (y7,w7r);
        y7 = vec_madd(x7,w7i,y7);
        y25 = vec_mul (y25,w7r);
        y25 = vec_nmsub(x25,w7i,y25);
        y27 = vec_mul (y27,w5r);
        y27 = vec_nmsub(x27,w5i,y27);
        vec_st(y5,I5,&out[2*j]);
        vec_st(y7,I7,&out[2*j]);
        vec_st(y25,I25,&out[2*j]);
        vec_st(y27,I27,&out[2*j]);

        w9r = vec_ld(256,&w[64*j]);
        w9i = vec_ld(272,&w[64*j]);
        w11r = vec_ld(320,&w[64*j]);
        w11i = vec_ld(336,&w[64*j]);
        y9 = vec_nmsub(v42,vSIGN,v41);
        y11 = vec_madd(v82,vSIGN,v81);
        y21 = vec_nmsub(v82,vSIGN,v81);
        y23 = vec_madd(v42,vSIGN,v41);
        x9 = vec_perm(y9,y9,vmaskx);
        x11 = vec_perm(y11,y11,vmaskx);
        x21 = vec_perm(y21,y21,vmaskx);
        x23 = vec_perm(y23,y23,vmaskx);
        y9 = vec_mul (y9,w9r);
        y9 = vec_madd(x9,w9i,y9);
        y11 = vec_mul (y11,w11r);
        y11 = vec_madd(x11,w11i,y11);
        y21 = vec_mul (y21,w11r);
        y21 = vec_nmsub(x21,w11i,y21);
        y23 = vec_mul (y23,w9r);
        y23 = vec_nmsub(x23,w9i,y23);
        vec_st(y9,I9,&out[2*j]);
        vec_st(y11,I11,&out[2*j]);
        vec_st(y21,I21,&out[2*j]);
        vec_st(y23,I23,&out[2*j]);

        w13r = vec_ld(384,&w[64*j]);
        w13i = vec_ld(400,&w[64*j]);
        w15r = vec_ld(448,&w[64*j]);
        w15i = vec_ld(464,&w[64*j]);
        y13 = vec_nmsub(v62,vSIGN,v61);
        y15 = vec_madd(v22,vSIGN,v21);
        y17 = vec_nmsub(v22,vSIGN,v21);
        y19 = vec_madd(v62,vSIGN,v61);
        x13 = vec_perm(y13,y13,vmaskx);
        x15 = vec_perm(y15,y15,vmaskx);
        x17 = vec_perm(y17,y17,vmaskx);
        x19 = vec_perm(y19,y19,vmaskx);
        y13 = vec_mul (y13,w13r);
        y13 = vec_madd(x13,w13i,y13);
        y15 = vec_mul (y15,w15r);
        y15 = vec_madd(x15,w15i,y15);
        y17 = vec_mul (y17,w15r);
        y17 = vec_nmsub(x17,w15i,y17);
        y19 = vec_mul (y19,w13r);
        y19 = vec_nmsub(x19,w13i,y19);
        vec_st(y13,I13,&out[2*j]);
        vec_st(y15,I15,&out[2*j]);
        vec_st(y17,I17,&out[2*j]);
        vec_st(y19,I19,&out[2*j]);
#if 0
     for (i=0; i<32; i++) {
       printf("%10.6f %10.6f ", out[2*j+64*i], out[2*j+64*i+1]);
     }
     printf("\n");
     for (i=0; i<32; i++) {
       printf("%10.6f %10.6f ", out[2*j+64*i+2], out[2*j+64*i+3]);
     }
     printf("\n");
#endif
    }
}

void fft_32_s2(int isign, float *in, float *out)
{
    int i, j, jj;
    int J0, J1, J2, J3, J4, J5, J6, J7, J8, J9;
    int J10, J11, J12, J13, J14, J15, J16, J17, J18, J19;
    int J20, J21, J22, J23, J24, J25, J26, J27, J28, J29;
    int J30, J31;
    vector float x0, x1, x2, x3, x4, x5, x6, x7, x8, x9;
    vector float x10, x11, x12, x13, x14, x15, x16, x17, x18, x19;
    vector float x20, x21, x22, x23, x24, x25, x26, x27, x28, x29;
    vector float x30, x31;
    vector float y0,y1,y2,y3,y4,y5,y6,y7,y8,y9;
    vector float y10,y11,y12,y13,y14,y15,y16,y17,y18,y19;
    vector float y20,y21,y22,y23,y24,y25,y26,y27,y28,y29;
    vector float y30,y31;
    vector float xx0,xx1,xx2,xx3,xx4,xx5,xx6,xx7,xx8,xx9;
    vector float xx10,xx11,xx12,xx13,xx14,xx15;
    vector float a0,a1,a2,a3,a01,a11,a21,a31,a41,a51,a61,a71;
    vector float b0,b1,b2,b3,b01,b11,b21,b31,b41,b51,b61,b71;
    vector float a00,a10,a20,a30,a40,a50,a60,a70;
    vector float b00,b10,b20,b30,b40,b50,b60,b70,b80,b90;
    vector float b100,b110,b120,b130,b140,b150;
    vector float z11,z31,z12,z32,t11,t31,t12,t32;
    vector float z111,z112,z113,z114,z121,z122,z123,z124;
    vector float v11,v21,v31,v41,v51,v61,v71,v81;
    vector float v12,v22,v32,v42,v52,v62,v72,v82;
    vector float v111,v112,v113,v114,v121,v122,v123,v124;
    vector float bb00,bb10,bb20,bb30,bb40,bb50,bb60,bb70;
    vector float vSIGN;
    vector float vSIGN1= {-1.0, 1.0,-1.0, 1.0};
    vector float vSIGN2= { 1.0,-1.0, 1.0,-1.0};
    vector float vSCALE;
    vector float vC1 = {0.98078528040323044913, 0.98078528040323044913, 0.98078528040323044913, 0.98078528040323044913};
    vector float vC2 = {0.92387953251128675613, 0.92387953251128675613, 0.92387953251128675613, 0.92387953251128675613};
    vector float vC3 = {0.83146961230254523708, 0.83146961230254523708, 0.83146961230254523708, 0.83146961230254523708};
    vector float vC4 = {0.70710678118654752440, 0.70710678118654752440, 0.70710678118654752440, 0.70710678118654752440};
    vector float vT1 = {0.19891236737965800691, 0.19891236737965800691, 0.19891236737965800691, 0.19891236737965800691};
    vector float vT2 = {0.41421356237309504880, 0.41421356237309504880, 0.41421356237309504880, 0.41421356237309504880};
    vector float vT3 = {0.66817863791929892000, 0.66817863791929892000, 0.66817863791929892000, 0.66817863791929892000};
    vector unsigned char vmaskx = {0x04, 0x05, 0x06, 0x07, 0x00, 0x01, 0x02, 0x03, 0x1C, 0x1D, 0x1E, 0x1F, 0x18, 0x19, 0x1A, 0x1B};
    vector unsigned char vmaskf = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17};
    vector unsigned char vmaskl = {0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};
    vector unsigned char vmasky = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F};

    if (isign >= 0) {
        vSIGN = vSIGN1;
    } else {
        vSIGN = vSIGN2;
    }

    J0 = 0;
    J1 = J0 + 32*sizeof(float)*2;
    J2 = J1 + 32*sizeof(float)*2;
    J3 = J2 + 32*sizeof(float)*2;
    J4 = J3 + 32*sizeof(float)*2;
    J5 = J4 + 32*sizeof(float)*2;
    J6 = J5 + 32*sizeof(float)*2;
    J7 = J6 + 32*sizeof(float)*2;
    J8 = J7 + 32*sizeof(float)*2;
    J9 = J8 + 32*sizeof(float)*2;
    J10 = J9 + 32*sizeof(float)*2;
    J11 = J10 + 32*sizeof(float)*2;
    J12 = J11 + 32*sizeof(float)*2;
    J13 = J12 + 32*sizeof(float)*2;
    J14 = J13 + 32*sizeof(float)*2;
    J15 = J14 + 32*sizeof(float)*2;
    J16 = J15 + 32*sizeof(float)*2;
    J17 = J16 + 32*sizeof(float)*2;
    J18 = J17 + 32*sizeof(float)*2;
    J19 = J18 + 32*sizeof(float)*2;
    J20 = J19 + 32*sizeof(float)*2;
    J21 = J20 + 32*sizeof(float)*2;
    J22 = J21 + 32*sizeof(float)*2;
    J23 = J22 + 32*sizeof(float)*2;
    J24 = J23 + 32*sizeof(float)*2;
    J25 = J24 + 32*sizeof(float)*2;
    J26 = J25 + 32*sizeof(float)*2;
    J27 = J26 + 32*sizeof(float)*2;
    J28 = J27 + 32*sizeof(float)*2;
    J29 = J28 + 32*sizeof(float)*2;
    J30 = J29 + 32*sizeof(float)*2;
    J31 = J30 + 32*sizeof(float)*2;

    for (j = 0; j < 15; j+=2) {
        jj = j+1;

        y0 = vec_ld(  0,&in[64*j]);        //x(0:1,j)
        y1 = vec_ld(  0,&in[64*jj]);        //x(0:1,j+1)
        y16 = vec_ld(128,&in[64*j]);        //x(16:17,j)
        y17 = vec_ld(128,&in[64*jj]);        //x(16:17,j+1)
        x0 = vec_perm(y0,y1,vmaskf);
        x16 = vec_perm(y16,y17,vmaskf);
        xx0 = vec_add(x0,x16);
        b00 = vec_sub(x0,x16);
        x1 = vec_perm(y0,y1,vmaskl);
        x17 = vec_perm(y16,y17,vmaskl);
        xx1 = vec_add(x1,x17);
        b10 = vec_sub(x1,x17);

        y2 = vec_ld( 16,&in[64*j]);        //x(2:3,j)
        y3 = vec_ld( 16,&in[64*jj]);        //x(2:3,j+1)
        y18 = vec_ld(144,&in[64*j]);        //x(18:19,j)
        y19 = vec_ld(144,&in[64*jj]);        //x(18:19,j+1)
        x2 = vec_perm(y2,y3,vmaskf);
        x18 = vec_perm(y18,y19,vmaskf);
        xx2 = vec_add(x2,x18);
        b20 = vec_sub(x2,x18);
        x3 = vec_perm(y2,y3,vmaskl);
        x19 = vec_perm(y18,y19,vmaskl);
        xx3 = vec_add(x3,x19);
        b30 = vec_sub(x3,x19);

        y4 = vec_ld( 32,&in[64*j]);        //x(4:5,j)
        y5 = vec_ld( 32,&in[64*jj]);        //x(4:5,j+1)
        y20 = vec_ld(160,&in[64*j]);        //x(20:21,j)
        y21 = vec_ld(160,&in[64*jj]);        //x(20:21,j+1)
        x4 = vec_perm(y4,y5,vmaskf);
        x20 = vec_perm(y20,y21,vmaskf);
        xx4 = vec_add(x4,x20);
        b40 = vec_sub(x4,x20);
        x5 = vec_perm(y4,y5,vmaskl);
        x21 = vec_perm(y20,y21,vmaskl);
        xx5 = vec_add(x5,x21);
        b50 = vec_sub(x5,x21);

        y6 = vec_ld( 48,&in[64*j]);        //x(6:7,j)
        y7 = vec_ld( 48,&in[64*jj]);        //x(6:7,j+1)
        y22 = vec_ld(176,&in[64*j]);        //x(22:23,j)
        y23 = vec_ld(176,&in[64*jj]);        //x(22:23,j+1)
        x6 = vec_perm(y6,y7,vmaskf);
        x22 = vec_perm(y22,y23,vmaskf);
        xx6 = vec_add(x6,x22);
        b60 = vec_sub(x6,x22);
        x7 = vec_perm(y6,y7,vmaskl);
        x23 = vec_perm(y22,y23,vmaskl);
        xx7 = vec_add(x7,x23);
        b70 = vec_sub(x7,x23);

        y8 = vec_ld( 64,&in[64*j]);        //x(8:9,j)
        y9 = vec_ld( 64,&in[64*jj]);        //x(8:9,j+1)
        y24 = vec_ld(192,&in[64*j]);        //x(24:25,j)
        y25 = vec_ld(192,&in[64*jj]);        //x(24:25,j+1)
        x8 = vec_perm(y8,y9,vmaskf);
        x24 = vec_perm(y24,y25,vmaskf);
        xx8 = vec_add(x8,x24);
        b80 = vec_sub(x8,x24);
        x9 = vec_perm(y8,y9,vmaskl);
        x25 = vec_perm(y24,y25,vmaskl);
        xx9 = vec_add(x9,x25);
        b90 = vec_sub(x9,x25);

        y10 = vec_ld( 80,&in[64*j]);        //x(10:11,j)
        y11 = vec_ld( 80,&in[64*jj]);        //x(10:11,j+1)
        y26 = vec_ld(208,&in[64*j]);        //x(26:27,j)
        y27 = vec_ld(208,&in[64*jj]);        //x(26:27,j+1)
        x10 = vec_perm(y10,y11,vmaskf);
        x26 = vec_perm(y26,y27,vmaskf);
        xx10 = vec_add(x10,x26);
        b100 = vec_sub(x10,x26);
        x11 = vec_perm(y10,y11,vmaskl);
        x27 = vec_perm(y26,y27,vmaskl);
        xx11 = vec_add(x11,x27);
        b110 = vec_sub(x11,x27);

        y12 = vec_ld( 96,&in[64*j]);        //x(12:13,j)
        y13 = vec_ld( 96,&in[64*jj]);        //x(12:13,j+1)
        y28 = vec_ld(224,&in[64*j]);        //x(28:29,j)
        y29 = vec_ld(224,&in[64*jj]);        //x(28:29,j+1)
        x12 = vec_perm(y12,y13,vmaskf);
        x28 = vec_perm(y28,y29,vmaskf);
        xx12 = vec_add(x12,x28);
        b120 = vec_sub(x12,x28);
        x13 = vec_perm(y12,y13,vmaskl);
        x29 = vec_perm(y28,y29,vmaskl);
        xx13 = vec_add(x13,x29);
        b130 = vec_sub(x13,x29);

        y14 = vec_ld(112,&in[64*j]);        //x(14:15,j)
        y15 = vec_ld(112,&in[64*jj]);        //x(14:15,j+1)
        y30 = vec_ld(240,&in[64*j]);        //x(30:31,j)
        y31 = vec_ld(240,&in[64*jj]);        //x(30:31,j+1)
        x14 = vec_perm(y14,y15,vmaskf);
        x30 = vec_perm(y30,y31,vmaskf);
        xx14 = vec_add(x14,x30);
        b140 = vec_sub(x14,x30);
        x15 = vec_perm(y14,y15,vmaskl);
        x31 = vec_perm(y30,y31,vmaskl);
        xx15 = vec_add(x15,x31);
        b150 = vec_sub(x15,x31);

        a00 = vec_add(xx0,xx8);
        bb00 = vec_sub(xx0,xx8);
        a40 = vec_add(xx4,xx12);
        bb40 = vec_sub(xx4,xx12);
        a0 = vec_add(a00,a40);
        b0 = vec_sub(a00,a40);

        a20 = vec_add(xx2,xx10);
        bb20 = vec_sub(xx2,xx10);
        a60 = vec_add(xx6,xx14);
        bb60 = vec_sub(xx6,xx14);
        a2 = vec_add(a20,a60);
        b2 = vec_sub(a20,a60);
        a01 = vec_add(a0,a2);
        b01 = vec_sub(a0,a2);

        a10 = vec_add(xx1,xx9);
        bb10 = vec_sub(xx1,xx9);
        a70 = vec_add(xx7,xx15);
        bb70 = vec_sub(xx7,xx15);

        a30 = vec_add(xx3,xx11);
        bb30 = vec_sub(xx3,xx11);
        a50 = vec_add(xx5,xx13);
        bb50 = vec_sub(xx5,xx13);
        a1 = vec_add(a10,a50);
        b1 = vec_sub(a10,a50);
        a3 = vec_add(a30,a70);
        b3 = vec_sub(a30,a70);
        a11 = vec_add(a1,a3);
        b11 = vec_sub(a1,a3);
        b11 = vec_perm(b11,b11,vmaskx);

        y0 = vec_add(a01,a11);
        y16 = vec_sub(a01,a11);
        y8 = vec_nmsub(b11,vSIGN,b01);
        y24 = vec_madd(b11,vSIGN,b01);
        vec_st(y0,J0,&out[2*j]);
        vec_st(y8,J8,&out[2*j]);
        vec_st(y16,J16,&out[2*j]);
        vec_st(y24,J24,&out[2*j]);

        a11 = vec_sub(b1,b3);
        b11 = vec_add(b1,b3);

        v21 = vec_madd(a11,vC4,b0);
        v61 = vec_nmsub(a11,vC4,b0);
        v22 = vec_madd(b11,vC4,b2);
        v22 = vec_perm(v22,v22,vmaskx);
        v62 = vec_nmsub(b11,vC4,b2);
        v62 = vec_perm(v62,v62,vmaskx);

        y4 = vec_nmsub(v22,vSIGN,v21);
        y28 = vec_madd(v22,vSIGN,v21);
        y12 = vec_madd(v62,vSIGN,v61);
        y20 = vec_nmsub(v62,vSIGN,v61);
        vec_st(y4,J4,&out[2*j]);
        vec_st(y12,J12,&out[2*j]);
        vec_st(y20,J20,&out[2*j]);
        vec_st(y28,J28,&out[2*j]);

        a1 = vec_sub(bb10,bb70);
        b1 = vec_add(bb10,bb70);
        a2 = vec_sub(bb20,bb60);
        b2 = vec_add(bb20,bb60);
        a3 = vec_sub(bb30,bb50);
        b3 = vec_add(bb30,bb50);

        z11 = vec_madd(a2,vC4,bb00);
        z31 = vec_nmsub(a2,vC4,bb00);
        z12 = vec_madd(b2,vC4,bb40);
        z12 = vec_perm(z12,z12,vmaskx);
        z32 = vec_nmsub(b2,vC4,bb40);
        z32 = vec_perm(z32,z32,vmaskx);
        t11 = vec_madd(a3,vT2,a1);
        t31 = vec_nmsub(a1,vT2,a3);
        t12 = vec_madd(b1,vT2,b3);
        t12 = vec_perm(t12,t12,vmaskx);
        t32 = vec_nmsub(b3,vT2,b1);
        t32 = vec_perm(t32,t32,vmaskx);

        v11 = vec_madd(t11,vC2,z11);
        v71 = vec_nmsub(t11,vC2,z11);
        v12 = vec_madd(t12,vC2,z12);
        v72 = vec_nmsub(t12,vC2,z12);

        y2 = vec_nmsub(v12,vSIGN,v11);
        y30 = vec_madd(v12,vSIGN,v11);
        y14 = vec_madd(v72,vSIGN,v71);
        y18 = vec_nmsub(v72,vSIGN,v71);
        vec_st(y2,J2,&out[2*j]);
        vec_st(y14,J14,&out[2*j]);
        vec_st(y18,J18,&out[2*j]);
        vec_st(y30,J30,&out[2*j]);

        v31 = vec_nmsub(t31,vC2,z31);
        v51 = vec_madd(t31,vC2,z31);
        v32 = vec_nmsub(t32,vC2,z32);
        v52 = vec_madd(t32,vC2,z32);

        y6 = vec_madd(v32,vSIGN,v31);
        y26 = vec_nmsub(v32,vSIGN,v31);
        y10 = vec_nmsub(v52,vSIGN,v51);
        y22 = vec_madd(v52,vSIGN,v51);
        vec_st(y6,J6,&out[2*j]);
        vec_st(y10,J10,&out[2*j]);
        vec_st(y22,J22,&out[2*j]);
        vec_st(y26,J26,&out[2*j]);

        a11 = vec_sub(b40,b120);
        b11 = vec_add(b40,b120);

        a21 = vec_sub(b20,b140);
        b21 = vec_add(b20,b140);

        a31 = vec_sub(b60,b100);
        b31 = vec_add(b60,b100);

        a41 = vec_sub(b10,b150);
        b41 = vec_add(b10,b150);

        a51 = vec_sub(b70,b90);
        b51 = vec_add(b70,b90);

        a61 = vec_sub(b30,b130);
        b61 = vec_add(b30,b130);

        a71 = vec_sub(b50,b110);
        b71 = vec_add(b50,b110);

        v111 = vec_madd(a11,vC4,b00);
        z111 = vec_nmsub(a11,vC4,b00);
        v112 = vec_madd(a31,vT2,a21);
        z112 = vec_nmsub(a21,vT2,a31);
        xx0 = vec_add(a41,a51);
        xx1 = vec_sub(a41,a51);
        v113 = vec_madd(xx0,vC4,a61);
        z113 = vec_nmsub(xx0,vC4,a61);
        v114 = vec_madd(xx1,vC4,a71);
        z114 = vec_nmsub(xx1,vC4,a71);

        v121 = vec_madd(b11,vC4,b80);
        v121 = vec_perm(v121,v121,vmaskx);
        z121 = vec_nmsub(b11,vC4,b80);
        z121 = vec_perm(z121,z121,vmaskx);
        v122 = vec_madd(b21,vT2,b31);
        v122 = vec_perm(v122,v122,vmaskx);
        z122 = vec_nmsub(b31,vT2,b21);
        z122 = vec_perm(z122,z122,vmaskx);

        xx0 = vec_add(b41,b51);
        xx1 = vec_sub(b41,b51);
        v123 = vec_madd(xx0,vC4,b71);
        v123 = vec_perm(v123,v123,vmaskx);
        z123 = vec_nmsub(xx0,vC4,b71);
        z123 = vec_perm(z123,z123,vmaskx);
        v124 = vec_nmsub(xx1,vC4,b61);
        v124 = vec_perm(v124,v124,vmaskx);
        z124 = vec_madd(xx1,vC4,b61);
        z124 = vec_perm(z124,z124,vmaskx);

        xx0 = vec_madd(v112,vC2,v111);
        xx2 = vec_madd(v114,vT3,v113);
        v11 = vec_madd(xx2,vC3,xx0);
        v21 = vec_nmsub(xx2,vC3,xx0);

        xx0 = vec_madd(v122,vC2,v121);
        xx2 = vec_madd(v124,vT3,v123);
        v12 = vec_madd(xx2,vC3,xx0);
        v22 = vec_nmsub(xx2,vC3,xx0);

        y1 = vec_nmsub(v12,vSIGN,v11);
        y31 = vec_madd(v12,vSIGN,v11);
        y15 = vec_madd(v22,vSIGN,v21);
        y17 = vec_nmsub(v22,vSIGN,v21);
        vec_st(y1,J1,&out[2*j]);
        vec_st(y15,J15,&out[2*j]);
        vec_st(y17,J17,&out[2*j]);
        vec_st(y31,J31,&out[2*j]);

        xx1 = vec_nmsub(v112,vC2,v111);
        xx3 = vec_nmsub(v113,vT3,v114);
        v31 = vec_madd(xx3,vC3,xx1);
        v41 = vec_nmsub(xx3,vC3,xx1);

        xx1 = vec_nmsub(v122,vC2,v121);
        xx3 = vec_nmsub(v123,vT3,v124);
        v32 = vec_madd(xx3,vC3,xx1);
        v42 = vec_nmsub(xx3,vC3,xx1);

        y7 = vec_madd(v32,vSIGN,v31);
        y25 = vec_nmsub(v32,vSIGN,v31);
        y9 = vec_nmsub(v42,vSIGN,v41);
        y23 = vec_madd(v42,vSIGN,v41);
        vec_st(y7,J7,&out[2*j]);
        vec_st(y9,J9,&out[2*j]);
        vec_st(y23,J23,&out[2*j]);
        vec_st(y25,J25,&out[2*j]);

        xx1 = vec_nmsub(z112,vC2,z111);
        xx2 = vec_madd(z113,vT1,z114);
        v51 = vec_nmsub(xx2,vC1,xx1);
        v61 = vec_madd(xx2,vC1,xx1);

        xx1 = vec_nmsub(z122,vC2,z121);
        xx2 = vec_madd(z123,vT1,z124);
        v52 = vec_nmsub(xx2,vC1,xx1);
        v62 = vec_madd(xx2,vC1,xx1);

        y3 = vec_madd(v52,vSIGN,v51);
        y29 = vec_nmsub(v52,vSIGN,v51);
        y13 = vec_nmsub(v62,vSIGN,v61);
        y19 = vec_madd(v62,vSIGN,v61);
        vec_st(y3,J3,&out[2*j]);
        vec_st(y13,J13,&out[2*j]);
        vec_st(y19,J19,&out[2*j]);
        vec_st(y29,J29,&out[2*j]);

        xx0 = vec_madd(z112,vC2,z111);
        xx3 = vec_nmsub(z114,vT1,z113);
        v71 = vec_nmsub(xx3,vC1,xx0);
        v81 = vec_madd(xx3,vC1,xx0);

        xx0 = vec_madd(z122,vC2,z121);
        xx3 = vec_nmsub(z124,vT1,z123);
        v72 = vec_nmsub(xx3,vC1,xx0);
        v82 = vec_madd(xx3,vC1,xx0);

        y5 = vec_nmsub(v72,vSIGN,v71);
        y27 = vec_madd(v72,vSIGN,v71);
        y11 = vec_madd(v82,vSIGN,v81);
        y21 = vec_nmsub(v82,vSIGN,v81);
        vec_st(y5,J5,&out[2*j]);
        vec_st(y11,J11,&out[2*j]);
        vec_st(y21,J21,&out[2*j]);
        vec_st(y27,J27,&out[2*j]);
#if 0
     for (i=0; i<32; i++) {
       printf("%14.6f %14.6f ", out[2*j+64*i], out[2*j+64*i+1]);
     }
     printf("\n");
     for (i=0; i<32; i++) {
       printf("%14.6f %14.6f ", out[2*j+64*i+2], out[2*j+64*i+3]);
     }
     printf("\n");
#endif
    }

        j = 16;
        jj = j+1;

        y0 = vec_ld(  0,&in[64*j]);        //x(0:1,j)
        y1 = vec_ld(  0,&in[64*jj]);        //x(0:1,j+1)
        y16 = vec_ld(128,&in[64*j]);        //x(16:17,j)
        y17 = vec_ld(128,&in[64*jj]);        //x(16:17,j+1)
        x0 = vec_perm(y0,y1,vmaskf);
        x16 = vec_perm(y16,y17,vmaskf);
        xx0 = vec_add(x0,x16);
        b00 = vec_sub(x0,x16);
        x1 = vec_perm(y0,y1,vmaskl);
        x17 = vec_perm(y16,y17,vmaskl);
        xx1 = vec_add(x1,x17);
        b10 = vec_sub(x1,x17);

        y2 = vec_ld( 16,&in[64*j]);        //x(2:3,j)
        y3 = vec_ld( 16,&in[64*jj]);        //x(2:3,j+1)
        y18 = vec_ld(144,&in[64*j]);        //x(18:19,j)
        y19 = vec_ld(144,&in[64*jj]);        //x(18:19,j+1)
        x2 = vec_perm(y2,y3,vmaskf);
        x18 = vec_perm(y18,y19,vmaskf);
        xx2 = vec_add(x2,x18);
        b20 = vec_sub(x2,x18);
        x3 = vec_perm(y2,y3,vmaskl);
        x19 = vec_perm(y18,y19,vmaskl);
        xx3 = vec_add(x3,x19);
        b30 = vec_sub(x3,x19);

        y4 = vec_ld( 32,&in[64*j]);        //x(4:5,j)
        y5 = vec_ld( 32,&in[64*jj]);        //x(4:5,j+1)
        y20 = vec_ld(160,&in[64*j]);        //x(20:21,j)
        y21 = vec_ld(160,&in[64*jj]);        //x(20:21,j+1)
        x4 = vec_perm(y4,y5,vmaskf);
        x20 = vec_perm(y20,y21,vmaskf);
        xx4 = vec_add(x4,x20);
        b40 = vec_sub(x4,x20);
        x5 = vec_perm(y4,y5,vmaskl);
        x21 = vec_perm(y20,y21,vmaskl);
        xx5 = vec_add(x5,x21);
        b50 = vec_sub(x5,x21);

        y6 = vec_ld( 48,&in[64*j]);        //x(6:7,j)
        y7 = vec_ld( 48,&in[64*jj]);        //x(6:7,j+1)
        y22 = vec_ld(176,&in[64*j]);        //x(22:23,j)
        y23 = vec_ld(176,&in[64*jj]);        //x(22:23,j+1)
        x6 = vec_perm(y6,y7,vmaskf);
        x22 = vec_perm(y22,y23,vmaskf);
        xx6 = vec_add(x6,x22);
        b60 = vec_sub(x6,x22);
        x7 = vec_perm(y6,y7,vmaskl);
        x23 = vec_perm(y22,y23,vmaskl);
        xx7 = vec_add(x7,x23);
        b70 = vec_sub(x7,x23);

        y8 = vec_ld( 64,&in[64*j]);        //x(8:9,j)
        y9 = vec_ld( 64,&in[64*jj]);        //x(8:9,j+1)
        y24 = vec_ld(192,&in[64*j]);        //x(24:25,j)
        y25 = vec_ld(192,&in[64*jj]);        //x(24:25,j+1)
        x8 = vec_perm(y8,y9,vmaskf);
        x24 = vec_perm(y24,y25,vmaskf);
        xx8 = vec_add(x8,x24);
        b80 = vec_sub(x8,x24);
        x9 = vec_perm(y8,y9,vmaskl);
        x25 = vec_perm(y24,y25,vmaskl);
        xx9 = vec_add(x9,x25);
        b90 = vec_sub(x9,x25);

        y10 = vec_ld( 80,&in[64*j]);        //x(10:11,j)
        y11 = vec_ld( 80,&in[64*jj]);        //x(10:11,j+1)
        y26 = vec_ld(208,&in[64*j]);        //x(26:27,j)
        y27 = vec_ld(208,&in[64*jj]);        //x(26:27,j+1)
        x10 = vec_perm(y10,y11,vmaskf);
        x26 = vec_perm(y26,y27,vmaskf);
        xx10 = vec_add(x10,x26);
        b100 = vec_sub(x10,x26);
        x11 = vec_perm(y10,y11,vmaskl);
        x27 = vec_perm(y26,y27,vmaskl);
        xx11 = vec_add(x11,x27);
        b110 = vec_sub(x11,x27);

        y12 = vec_ld( 96,&in[64*j]);        //x(12:13,j)
        y13 = vec_ld( 96,&in[64*jj]);        //x(12:13,j+1)
        y28 = vec_ld(224,&in[64*j]);        //x(28:29,j)
        y29 = vec_ld(224,&in[64*jj]);        //x(28:29,j+1)
        x12 = vec_perm(y12,y13,vmaskf);
        x28 = vec_perm(y28,y29,vmaskf);
        xx12 = vec_add(x12,x28);
        b120 = vec_sub(x12,x28);
        x13 = vec_perm(y12,y13,vmaskl);
        x29 = vec_perm(y28,y29,vmaskl);
        xx13 = vec_add(x13,x29);
        b130 = vec_sub(x13,x29);

        y14 = vec_ld(112,&in[64*j]);        //x(14:15,j)
        y15 = vec_ld(112,&in[64*jj]);        //x(14:15,j+1)
        y30 = vec_ld(240,&in[64*j]);        //x(30:31,j)
        y31 = vec_ld(240,&in[64*jj]);        //x(30:31,j+1)
        x14 = vec_perm(y14,y15,vmaskf);
        x30 = vec_perm(y30,y31,vmaskf);
        xx14 = vec_add(x14,x30);
        b140 = vec_sub(x14,x30);
        x15 = vec_perm(y14,y15,vmaskl);
        x31 = vec_perm(y30,y31,vmaskl);
        xx15 = vec_add(x15,x31);
        b150 = vec_sub(x15,x31);

        a00 = vec_add(xx0,xx8);
        bb00 = vec_sub(xx0,xx8);
        a40 = vec_add(xx4,xx12);
        bb40 = vec_sub(xx4,xx12);
        a0 = vec_add(a00,a40);
        b0 = vec_sub(a00,a40);

        a20 = vec_add(xx2,xx10);
        bb20 = vec_sub(xx2,xx10);
        a60 = vec_add(xx6,xx14);
        bb60 = vec_sub(xx6,xx14);
        a2 = vec_add(a20,a60);
        b2 = vec_sub(a20,a60);
        a01 = vec_add(a0,a2);
        b01 = vec_sub(a0,a2);

        a10 = vec_add(xx1,xx9);
        bb10 = vec_sub(xx1,xx9);
        a70 = vec_add(xx7,xx15);
        bb70 = vec_sub(xx7,xx15);

        a30 = vec_add(xx3,xx11);
        bb30 = vec_sub(xx3,xx11);
        a50 = vec_add(xx5,xx13);
        bb50 = vec_sub(xx5,xx13);
        a1 = vec_add(a10,a50);
        b1 = vec_sub(a10,a50);
        a3 = vec_add(a30,a70);
        b3 = vec_sub(a30,a70);
        a11 = vec_add(a1,a3);
        b11 = vec_sub(a1,a3);
        b11 = vec_perm(b11,b11,vmaskx);

        y0 = vec_add(a01,a11);
        y16 = vec_sub(a01,a11);
        y8 = vec_nmsub(b11,vSIGN,b01);
        y24 = vec_madd(b11,vSIGN,b01);

        a11 = vec_sub(b1,b3);
        b11 = vec_add(b1,b3);

        v21 = vec_madd(a11,vC4,b0);
        v61 = vec_nmsub(a11,vC4,b0);
        v22 = vec_madd(b11,vC4,b2);
        v22 = vec_perm(v22,v22,vmaskx);
        v62 = vec_nmsub(b11,vC4,b2);
        v62 = vec_perm(v62,v62,vmaskx);

        y4 = vec_nmsub(v22,vSIGN,v21);
        y28 = vec_madd(v22,vSIGN,v21);
        y12 = vec_madd(v62,vSIGN,v61);
        y20 = vec_nmsub(v62,vSIGN,v61);

        a1 = vec_sub(bb10,bb70);
        b1 = vec_add(bb10,bb70);
        a2 = vec_sub(bb20,bb60);
        b2 = vec_add(bb20,bb60);
        a3 = vec_sub(bb30,bb50);
        b3 = vec_add(bb30,bb50);

        z11 = vec_madd(a2,vC4,bb00);
        z31 = vec_nmsub(a2,vC4,bb00);
        z12 = vec_madd(b2,vC4,bb40);
        z12 = vec_perm(z12,z12,vmaskx);
        z32 = vec_nmsub(b2,vC4,bb40);
        z32 = vec_perm(z32,z32,vmaskx);
        t11 = vec_madd(a3,vT2,a1);
        t31 = vec_nmsub(a1,vT2,a3);
        t12 = vec_madd(b1,vT2,b3);
        t12 = vec_perm(t12,t12,vmaskx);
        t32 = vec_nmsub(b3,vT2,b1);
        t32 = vec_perm(t32,t32,vmaskx);

        v11 = vec_madd(t11,vC2,z11);
        v71 = vec_nmsub(t11,vC2,z11);
        v12 = vec_madd(t12,vC2,z12);
        v72 = vec_nmsub(t12,vC2,z12);

        y2 = vec_nmsub(v12,vSIGN,v11);
        y30 = vec_madd(v12,vSIGN,v11);
        y14 = vec_madd(v72,vSIGN,v71);
        y18 = vec_nmsub(v72,vSIGN,v71);

        v31 = vec_nmsub(t31,vC2,z31);
        v51 = vec_madd(t31,vC2,z31);
        v32 = vec_nmsub(t32,vC2,z32);
        v52 = vec_madd(t32,vC2,z32);

        y6 = vec_madd(v32,vSIGN,v31);
        y26 = vec_nmsub(v32,vSIGN,v31);
        y10 = vec_nmsub(v52,vSIGN,v51);
        y22 = vec_madd(v52,vSIGN,v51);

        a11 = vec_sub(b40,b120);
        b11 = vec_add(b40,b120);

        a21 = vec_sub(b20,b140);
        b21 = vec_add(b20,b140);

        a31 = vec_sub(b60,b100);
        b31 = vec_add(b60,b100);

        a41 = vec_sub(b10,b150);
        b41 = vec_add(b10,b150);

        a51 = vec_sub(b70,b90);
        b51 = vec_add(b70,b90);

        a61 = vec_sub(b30,b130);
        b61 = vec_add(b30,b130);

        a71 = vec_sub(b50,b110);
        b71 = vec_add(b50,b110);

        v111 = vec_madd(a11,vC4,b00);
        z111 = vec_nmsub(a11,vC4,b00);
        v112 = vec_madd(a31,vT2,a21);
        z112 = vec_nmsub(a21,vT2,a31);
        xx0 = vec_add(a41,a51);
        xx1 = vec_sub(a41,a51);
        v113 = vec_madd(xx0,vC4,a61);
        z113 = vec_nmsub(xx0,vC4,a61);
        v114 = vec_madd(xx1,vC4,a71);
        z114 = vec_nmsub(xx1,vC4,a71);

        v121 = vec_madd(b11,vC4,b80);
        v121 = vec_perm(v121,v121,vmaskx);
        z121 = vec_nmsub(b11,vC4,b80);
        z121 = vec_perm(z121,z121,vmaskx);
        v122 = vec_madd(b21,vT2,b31);
        v122 = vec_perm(v122,v122,vmaskx);
        z122 = vec_nmsub(b31,vT2,b21);
        z122 = vec_perm(z122,z122,vmaskx);

        xx0 = vec_add(b41,b51);
        xx1 = vec_sub(b41,b51);
        v123 = vec_madd(xx0,vC4,b71);
        v123 = vec_perm(v123,v123,vmaskx);
        z123 = vec_nmsub(xx0,vC4,b71);
        z123 = vec_perm(z123,z123,vmaskx);
        v124 = vec_nmsub(xx1,vC4,b61);
        v124 = vec_perm(v124,v124,vmaskx);
        z124 = vec_madd(xx1,vC4,b61);
        z124 = vec_perm(z124,z124,vmaskx);

        xx0 = vec_madd(v112,vC2,v111);
        xx2 = vec_madd(v114,vT3,v113);
        v11 = vec_madd(xx2,vC3,xx0);
        v21 = vec_nmsub(xx2,vC3,xx0);

        xx0 = vec_madd(v122,vC2,v121);
        xx2 = vec_madd(v124,vT3,v123);
        v12 = vec_madd(xx2,vC3,xx0);
        v22 = vec_nmsub(xx2,vC3,xx0);

        y1 = vec_nmsub(v12,vSIGN,v11);
        y31 = vec_madd(v12,vSIGN,v11);
        x0 = vec_perm(y0,y1,vmasky);
        x1 = vec_perm(y1,y2,vmasky);
        x30= vec_perm(y30,y31,vmasky);
        x31= vec_perm(y31,y0,vmasky);
        vec_st(x0,J0,&out[2*j]);
        vec_st(x1,J1,&out[2*j]);
        vec_st(x30,J30,&out[2*j]);
        vec_st(x31,J31,&out[2*j]);

        y15 = vec_madd(v22,vSIGN,v21);
        y17 = vec_nmsub(v22,vSIGN,v21);
        x14= vec_perm(y14,y15,vmasky);
        x15= vec_perm(y15,y16,vmasky);
        x16= vec_perm(y16,y17,vmasky);
        x17= vec_perm(y17,y18,vmasky);
        vec_st(x14,J14,&out[2*j]);
        vec_st(x15,J15,&out[2*j]);
        vec_st(x16,J16,&out[2*j]);
        vec_st(x17,J17,&out[2*j]);

        xx1 = vec_nmsub(v112,vC2,v111);
        xx3 = vec_nmsub(v113,vT3,v114);
        v31 = vec_madd(xx3,vC3,xx1);
        v41 = vec_nmsub(xx3,vC3,xx1);

        xx1 = vec_nmsub(v122,vC2,v121);
        xx3 = vec_nmsub(v123,vT3,v124);
        v32 = vec_madd(xx3,vC3,xx1);
        v42 = vec_nmsub(xx3,vC3,xx1);

        y7 = vec_madd(v32,vSIGN,v31);
        y25 = vec_nmsub(v32,vSIGN,v31);
        x6 = vec_perm(y6,y7,vmasky);
        x7 = vec_perm(y7,y8,vmasky);
        x24= vec_perm(y24,y25,vmasky);
        x25= vec_perm(y25,y26,vmasky);
        vec_st(x6,J6,&out[2*j]);
        vec_st(x7,J7,&out[2*j]);
        vec_st(x24,J24,&out[2*j]);
        vec_st(x25,J25,&out[2*j]);

        y9 = vec_nmsub(v42,vSIGN,v41);
        y23 = vec_madd(v42,vSIGN,v41);
        x8 = vec_perm(y8,y9,vmasky);
        x9 = vec_perm(y9,y10,vmasky);
        x22= vec_perm(y22,y23,vmasky);
        x23= vec_perm(y23,y24,vmasky);
        vec_st(x8,J8,&out[2*j]);
        vec_st(x9,J9,&out[2*j]);
        vec_st(x22,J22,&out[2*j]);
        vec_st(x23,J23,&out[2*j]);

        xx1 = vec_nmsub(z112,vC2,z111);
        xx2 = vec_madd(z113,vT1,z114);
        v51 = vec_nmsub(xx2,vC1,xx1);
        v61 = vec_madd(xx2,vC1,xx1);

        xx1 = vec_nmsub(z122,vC2,z121);
        xx2 = vec_madd(z123,vT1,z124);
        v52 = vec_nmsub(xx2,vC1,xx1);
        v62 = vec_madd(xx2,vC1,xx1);

        y3 = vec_madd(v52,vSIGN,v51);
        y29 = vec_nmsub(v52,vSIGN,v51);
        x2 = vec_perm(y2,y3,vmasky);
        x3 = vec_perm(y3,y4,vmasky);
        x28= vec_perm(y28,y29,vmasky);
        x29= vec_perm(y29,y30,vmasky);
        vec_st(x2,J2,&out[2*j]);
        vec_st(x3,J3,&out[2*j]);
        vec_st(x28,J28,&out[2*j]);
        vec_st(x29,J29,&out[2*j]);

        y13 = vec_nmsub(v62,vSIGN,v61);
        y19 = vec_madd(v62,vSIGN,v61);
        x12= vec_perm(y12,y13,vmasky);
        x13= vec_perm(y13,y14,vmasky);
        x18= vec_perm(y18,y19,vmasky);
        x19= vec_perm(y19,y20,vmasky);
        vec_st(x12,J12,&out[2*j]);
        vec_st(x13,J13,&out[2*j]);
        vec_st(x18,J18,&out[2*j]);
        vec_st(x19,J19,&out[2*j]);

        xx0 = vec_madd(z112,vC2,z111);
        xx3 = vec_nmsub(z114,vT1,z113);
        v71 = vec_nmsub(xx3,vC1,xx0);
        v81 = vec_madd(xx3,vC1,xx0);

        xx0 = vec_madd(z122,vC2,z121);
        xx3 = vec_nmsub(z124,vT1,z123);
        v72 = vec_nmsub(xx3,vC1,xx0);
        v82 = vec_madd(xx3,vC1,xx0);

        y5 = vec_nmsub(v72,vSIGN,v71);
        y27 = vec_madd(v72,vSIGN,v71);
        x4 = vec_perm(y4,y5,vmasky);
        x5 = vec_perm(y5,y6,vmasky);
        x26= vec_perm(y26,y27,vmasky);
        x27= vec_perm(y27,y28,vmasky);
        vec_st(x4,J4,&out[2*j]);
        vec_st(x5,J5,&out[2*j]);
        vec_st(x26,J26,&out[2*j]);
        vec_st(x27,J27,&out[2*j]);

        y11 = vec_madd(v82,vSIGN,v81);
        y21 = vec_nmsub(v82,vSIGN,v81);
        x10= vec_perm(y10,y11,vmasky);
        x11= vec_perm(y11,y12,vmasky);
        x20= vec_perm(y20,y21,vmasky);
        x21= vec_perm(y21,y22,vmasky);
        vec_st(x10,J10,&out[2*j]);
        vec_st(x11,J11,&out[2*j]);
        vec_st(x20,J20,&out[2*j]);
        vec_st(x21,J21,&out[2*j]);
#if 0
     for (i=0; i<32; i++) {
       printf("%14.6f %14.6f ", out[2*j+64*i], out[2*j+64*i+1]);
     }
     printf("\n");
     for (i=0; i<32; i++) {
       printf("%14.6f %14.6f ", out[2*j+64*i+2], out[2*j+64*i+3]);
     }
     printf("\n");
#endif

    for (j = 18; j < 31; j+=2 ) {
        jj = j+1;

        y0 = vec_ld(  0,&in[64*j]);        //x(0:1,j)
        y1 = vec_ld(  0,&in[64*jj]);        //x(0:1,j+1)
        y16 = vec_ld(128,&in[64*j]);        //x(16:17,j)
        y17 = vec_ld(128,&in[64*jj]);        //x(16:17,j+1)
        x0 = vec_perm(y0,y1,vmaskf);
        x16 = vec_perm(y16,y17,vmaskf);
        xx0 = vec_add(x0,x16);
        b00 = vec_sub(x0,x16);
        x1 = vec_perm(y0,y1,vmaskl);
        x17 = vec_perm(y16,y17,vmaskl);
        xx1 = vec_add(x1,x17);
        b10 = vec_sub(x1,x17);

        y2 = vec_ld( 16,&in[64*j]);        //x(2:3,j)
        y3 = vec_ld( 16,&in[64*jj]);        //x(2:3,j+1)
        y18 = vec_ld(144,&in[64*j]);        //x(18:19,j)
        y19 = vec_ld(144,&in[64*jj]);        //x(18:19,j+1)
        x2 = vec_perm(y2,y3,vmaskf);
        x18 = vec_perm(y18,y19,vmaskf);
        xx2 = vec_add(x2,x18);
        b20 = vec_sub(x2,x18);
        x3 = vec_perm(y2,y3,vmaskl);
        x19 = vec_perm(y18,y19,vmaskl);
        xx3 = vec_add(x3,x19);
        b30 = vec_sub(x3,x19);

        y4 = vec_ld( 32,&in[64*j]);        //x(4:5,j)
        y5 = vec_ld( 32,&in[64*jj]);        //x(4:5,j+1)
        y20 = vec_ld(160,&in[64*j]);        //x(20:21,j)
        y21 = vec_ld(160,&in[64*jj]);        //x(20:21,j+1)
        x4 = vec_perm(y4,y5,vmaskf);
        x20 = vec_perm(y20,y21,vmaskf);
        xx4 = vec_add(x4,x20);
        b40 = vec_sub(x4,x20);
        x5 = vec_perm(y4,y5,vmaskl);
        x21 = vec_perm(y20,y21,vmaskl);
        xx5 = vec_add(x5,x21);
        b50 = vec_sub(x5,x21);

        y6 = vec_ld( 48,&in[64*j]);        //x(6:7,j)
        y7 = vec_ld( 48,&in[64*jj]);        //x(6:7,j+1)
        y22 = vec_ld(176,&in[64*j]);        //x(22:23,j)
        y23 = vec_ld(176,&in[64*jj]);        //x(22:23,j+1)
        x6 = vec_perm(y6,y7,vmaskf);
        x22 = vec_perm(y22,y23,vmaskf);
        xx6 = vec_add(x6,x22);
        b60 = vec_sub(x6,x22);
        x7 = vec_perm(y6,y7,vmaskl);
        x23 = vec_perm(y22,y23,vmaskl);
        xx7 = vec_add(x7,x23);
        b70 = vec_sub(x7,x23);

        y8 = vec_ld( 64,&in[64*j]);        //x(8:9,j)
        y9 = vec_ld( 64,&in[64*jj]);        //x(8:9,j+1)
        y24 = vec_ld(192,&in[64*j]);        //x(24:25,j)
        y25 = vec_ld(192,&in[64*jj]);        //x(24:25,j+1)
        x8 = vec_perm(y8,y9,vmaskf);
        x24 = vec_perm(y24,y25,vmaskf);
        xx8 = vec_add(x8,x24);
        b80 = vec_sub(x8,x24);
        x9 = vec_perm(y8,y9,vmaskl);
        x25 = vec_perm(y24,y25,vmaskl);
        xx9 = vec_add(x9,x25);
        b90 = vec_sub(x9,x25);

        y10 = vec_ld( 80,&in[64*j]);        //x(10:11,j)
        y11 = vec_ld( 80,&in[64*jj]);        //x(10:11,j+1)
        y26 = vec_ld(208,&in[64*j]);        //x(26:27,j)
        y27 = vec_ld(208,&in[64*jj]);        //x(26:27,j+1)
        x10 = vec_perm(y10,y11,vmaskf);
        x26 = vec_perm(y26,y27,vmaskf);
        xx10 = vec_add(x10,x26);
        b100 = vec_sub(x10,x26);
        x11 = vec_perm(y10,y11,vmaskl);
        x27 = vec_perm(y26,y27,vmaskl);
        xx11 = vec_add(x11,x27);
        b110 = vec_sub(x11,x27);

        y12 = vec_ld( 96,&in[64*j]);        //x(12:13,j)
        y13 = vec_ld( 96,&in[64*jj]);        //x(12:13,j+1)
        y28 = vec_ld(224,&in[64*j]);        //x(28:29,j)
        y29 = vec_ld(224,&in[64*jj]);        //x(28:29,j+1)
        x12 = vec_perm(y12,y13,vmaskf);
        x28 = vec_perm(y28,y29,vmaskf);
        xx12 = vec_add(x12,x28);
        b120 = vec_sub(x12,x28);
        x13 = vec_perm(y12,y13,vmaskl);
        x29 = vec_perm(y28,y29,vmaskl);
        xx13 = vec_add(x13,x29);
        b130 = vec_sub(x13,x29);

        y14 = vec_ld(112,&in[64*j]);        //x(14:15,j)
        y15 = vec_ld(112,&in[64*jj]);        //x(14:15,j+1)
        y30 = vec_ld(240,&in[64*j]);        //x(30:31,j)
        y31 = vec_ld(240,&in[64*jj]);        //x(30:31,j+1)
        x14 = vec_perm(y14,y15,vmaskf);
        x30 = vec_perm(y30,y31,vmaskf);
        xx14 = vec_add(x14,x30);
        b140 = vec_sub(x14,x30);
        x15 = vec_perm(y14,y15,vmaskl);
        x31 = vec_perm(y30,y31,vmaskl);
        xx15 = vec_add(x15,x31);
        b150 = vec_sub(x15,x31);

        a00 = vec_add(xx0,xx8);
        bb00 = vec_sub(xx0,xx8);
        a40 = vec_add(xx4,xx12);
        bb40 = vec_sub(xx4,xx12);
        a0 = vec_add(a00,a40);
        b0 = vec_sub(a00,a40);

        a20 = vec_add(xx2,xx10);
        bb20 = vec_sub(xx2,xx10);
        a60 = vec_add(xx6,xx14);
        bb60 = vec_sub(xx6,xx14);
        a2 = vec_add(a20,a60);
        b2 = vec_sub(a20,a60);
        a01 = vec_add(a0,a2);
        b01 = vec_sub(a0,a2);

        a10 = vec_add(xx1,xx9);
        bb10 = vec_sub(xx1,xx9);
        a70 = vec_add(xx7,xx15);
        bb70 = vec_sub(xx7,xx15);

        a30 = vec_add(xx3,xx11);
        bb30 = vec_sub(xx3,xx11);
        a50 = vec_add(xx5,xx13);
        bb50 = vec_sub(xx5,xx13);
        a1 = vec_add(a10,a50);
        b1 = vec_sub(a10,a50);
        a3 = vec_add(a30,a70);
        b3 = vec_sub(a30,a70);
        a11 = vec_add(a1,a3);
        b11 = vec_sub(a1,a3);
        b11 = vec_perm(b11,b11,vmaskx);

        y0 = vec_add(a01,a11);
        y16 = vec_sub(a01,a11);
        y8 = vec_nmsub(b11,vSIGN,b01);
        y24 = vec_madd(b11,vSIGN,b01);
        vec_st(y0,J31,&out[2*j]);
        vec_st(y8,J7,&out[2*j]);
        vec_st(y16,J15,&out[2*j]);
        vec_st(y24,J23,&out[2*j]);

        a11 = vec_sub(b1,b3);
        b11 = vec_add(b1,b3);

        v21 = vec_madd(a11,vC4,b0);
        v61 = vec_nmsub(a11,vC4,b0);
        v22 = vec_madd(b11,vC4,b2);
        v22 = vec_perm(v22,v22,vmaskx);
        v62 = vec_nmsub(b11,vC4,b2);
        v62 = vec_perm(v62,v62,vmaskx);

        y4 = vec_nmsub(v22,vSIGN,v21);
        y28 = vec_madd(v22,vSIGN,v21);
        y12 = vec_madd(v62,vSIGN,v61);
        y20 = vec_nmsub(v62,vSIGN,v61);
        vec_st(y4,J3,&out[2*j]);
        vec_st(y12,J11,&out[2*j]);
        vec_st(y20,J19,&out[2*j]);
        vec_st(y28,J27,&out[2*j]);

        a1 = vec_sub(bb10,bb70);
        b1 = vec_add(bb10,bb70);
        a2 = vec_sub(bb20,bb60);
        b2 = vec_add(bb20,bb60);
        a3 = vec_sub(bb30,bb50);
        b3 = vec_add(bb30,bb50);

        z11 = vec_madd(a2,vC4,bb00);
        z31 = vec_nmsub(a2,vC4,bb00);
        z12 = vec_madd(b2,vC4,bb40);
        z12 = vec_perm(z12,z12,vmaskx);
        z32 = vec_nmsub(b2,vC4,bb40);
        z32 = vec_perm(z32,z32,vmaskx);
        t11 = vec_madd(a3,vT2,a1);
        t31 = vec_nmsub(a1,vT2,a3);
        t12 = vec_madd(b1,vT2,b3);
        t12 = vec_perm(t12,t12,vmaskx);
        t32 = vec_nmsub(b3,vT2,b1);
        t32 = vec_perm(t32,t32,vmaskx);

        v11 = vec_madd(t11,vC2,z11);
        v71 = vec_nmsub(t11,vC2,z11);
        v12 = vec_madd(t12,vC2,z12);
        v72 = vec_nmsub(t12,vC2,z12);

        y2 = vec_nmsub(v12,vSIGN,v11);
        y30 = vec_madd(v12,vSIGN,v11);
        y14 = vec_madd(v72,vSIGN,v71);
        y18 = vec_nmsub(v72,vSIGN,v71);
        vec_st(y2,J1,&out[2*j]);
        vec_st(y14,J13,&out[2*j]);
        vec_st(y18,J17,&out[2*j]);
        vec_st(y30,J29,&out[2*j]);

        v31 = vec_nmsub(t31,vC2,z31);
        v51 = vec_madd(t31,vC2,z31);
        v32 = vec_nmsub(t32,vC2,z32);
        v52 = vec_madd(t32,vC2,z32);

        y6 = vec_madd(v32,vSIGN,v31);
        y26 = vec_nmsub(v32,vSIGN,v31);
        y10 = vec_nmsub(v52,vSIGN,v51);
        y22 = vec_madd(v52,vSIGN,v51);
        vec_st(y6,J5,&out[2*j]);
        vec_st(y10,J9,&out[2*j]);
        vec_st(y22,J21,&out[2*j]);
        vec_st(y26,J25,&out[2*j]);

        a11 = vec_sub(b40,b120);
        b11 = vec_add(b40,b120);

        a21 = vec_sub(b20,b140);
        b21 = vec_add(b20,b140);

        a31 = vec_sub(b60,b100);
        b31 = vec_add(b60,b100);

        a41 = vec_sub(b10,b150);
        b41 = vec_add(b10,b150);

        a51 = vec_sub(b70,b90);
        b51 = vec_add(b70,b90);

        a61 = vec_sub(b30,b130);
        b61 = vec_add(b30,b130);

        a71 = vec_sub(b50,b110);
        b71 = vec_add(b50,b110);

        v111 = vec_madd(a11,vC4,b00);
        z111 = vec_nmsub(a11,vC4,b00);
        v112 = vec_madd(a31,vT2,a21);
        z112 = vec_nmsub(a21,vT2,a31);
        xx0 = vec_add(a41,a51);
        xx1 = vec_sub(a41,a51);
        v113 = vec_madd(xx0,vC4,a61);
        z113 = vec_nmsub(xx0,vC4,a61);
        v114 = vec_madd(xx1,vC4,a71);
        z114 = vec_nmsub(xx1,vC4,a71);

        v121 = vec_madd(b11,vC4,b80);
        v121 = vec_perm(v121,v121,vmaskx);
        z121 = vec_nmsub(b11,vC4,b80);
        z121 = vec_perm(z121,z121,vmaskx);
        v122 = vec_madd(b21,vT2,b31);
        v122 = vec_perm(v122,v122,vmaskx);
        z122 = vec_nmsub(b31,vT2,b21);
        z122 = vec_perm(z122,z122,vmaskx);

        xx0 = vec_add(b41,b51);
        xx1 = vec_sub(b41,b51);
        v123 = vec_madd(xx0,vC4,b71);
        v123 = vec_perm(v123,v123,vmaskx);
        z123 = vec_nmsub(xx0,vC4,b71);
        z123 = vec_perm(z123,z123,vmaskx);
        v124 = vec_nmsub(xx1,vC4,b61);
        v124 = vec_perm(v124,v124,vmaskx);
        z124 = vec_madd(xx1,vC4,b61);
        z124 = vec_perm(z124,z124,vmaskx);

        xx0 = vec_madd(v112,vC2,v111);
        xx2 = vec_madd(v114,vT3,v113);
        v11 = vec_madd(xx2,vC3,xx0);
        v21 = vec_nmsub(xx2,vC3,xx0);

        xx0 = vec_madd(v122,vC2,v121);
        xx2 = vec_madd(v124,vT3,v123);
        v12 = vec_madd(xx2,vC3,xx0);
        v22 = vec_nmsub(xx2,vC3,xx0);

        y1 = vec_nmsub(v12,vSIGN,v11);
        y31 = vec_madd(v12,vSIGN,v11);
        y15 = vec_madd(v22,vSIGN,v21);
        y17 = vec_nmsub(v22,vSIGN,v21);
        vec_st(y1,J0,&out[2*j]);
        vec_st(y15,J14,&out[2*j]);
        vec_st(y17,J16,&out[2*j]);
        vec_st(y31,J30,&out[2*j]);

        xx1 = vec_nmsub(v112,vC2,v111);
        xx3 = vec_nmsub(v113,vT3,v114);
        v31 = vec_madd(xx3,vC3,xx1);
        v41 = vec_nmsub(xx3,vC3,xx1);

        xx1 = vec_nmsub(v122,vC2,v121);
        xx3 = vec_nmsub(v123,vT3,v124);
        v32 = vec_madd(xx3,vC3,xx1);
        v42 = vec_nmsub(xx3,vC3,xx1);

        y7 = vec_madd(v32,vSIGN,v31);
        y25 = vec_nmsub(v32,vSIGN,v31);
        y9 = vec_nmsub(v42,vSIGN,v41);
        y23 = vec_madd(v42,vSIGN,v41);
        vec_st(y7,J6,&out[2*j]);
        vec_st(y9,J8,&out[2*j]);
        vec_st(y23,J22,&out[2*j]);
        vec_st(y25,J24,&out[2*j]);

        xx1 = vec_nmsub(z112,vC2,z111);
        xx2 = vec_madd(z113,vT1,z114);
        v51 = vec_nmsub(xx2,vC1,xx1);
        v61 = vec_madd(xx2,vC1,xx1);

        xx1 = vec_nmsub(z122,vC2,z121);
        xx2 = vec_madd(z123,vT1,z124);
        v52 = vec_nmsub(xx2,vC1,xx1);
        v62 = vec_madd(xx2,vC1,xx1);

        y3 = vec_madd(v52,vSIGN,v51);
        y29 = vec_nmsub(v52,vSIGN,v51);
        y13 = vec_nmsub(v62,vSIGN,v61);
        y19 = vec_madd(v62,vSIGN,v61);
        vec_st(y3,J2,&out[2*j]);
        vec_st(y13,J12,&out[2*j]);
        vec_st(y19,J18,&out[2*j]);
        vec_st(y29,J28,&out[2*j]);

        xx0 = vec_madd(z112,vC2,z111);
        xx3 = vec_nmsub(z114,vT1,z113);
        v71 = vec_nmsub(xx3,vC1,xx0);
        v81 = vec_madd(xx3,vC1,xx0);

        xx0 = vec_madd(z122,vC2,z121);
        xx3 = vec_nmsub(z124,vT1,z123);
        v72 = vec_nmsub(xx3,vC1,xx0);
        v82 = vec_madd(xx3,vC1,xx0);

        y5 = vec_nmsub(v72,vSIGN,v71);
        y27 = vec_madd(v72,vSIGN,v71);
        y11 = vec_madd(v82,vSIGN,v81);
        y21 = vec_nmsub(v82,vSIGN,v81);
        vec_st(y5,J4,&out[2*j]);
        vec_st(y11,J10,&out[2*j]);
        vec_st(y21,J20,&out[2*j]);
        vec_st(y27,J26,&out[2*j]);
#if 0
     for (i=0; i<32; i++) {
       printf("%14.6f %14.6f ", out[2*j+64*i], out[2*j+64*i+1]);
     }
     printf("\n");
     for (i=0; i<32; i++) {
       printf("%14.6f %14.6f ", out[2*j+64*i+2], out[2*j+64*i+3]);
     }
     printf("\n");
#endif
    }
}

void fft(unsigned int N, float (*x_r), float (*x_i), float (*X_R), float (*X_I),float **XX, float **x, float **X)
{
    unsigned int i, j;
    vector float xr, xi, x1, x2;
    vector unsigned char vmask1 = {0x00, 0x01, 0x02, 0x03, 0x08, 0x09, 0x0A, 0x0B, 0x10, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1A, 0x1B};
    vector unsigned char vmask2 = {0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F, 0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F};
    // TODO: Check performance whether we need more unroll instead of auto-unroll by compiler.
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
        vec_st(x1, 0, &aux1[i*8]);
        vec_st(x2,16, &aux1[i*8]);
    }

    /* Calculate FFT by a recursion. */
    fft_32_s1(1, 1.0, aux1, aux2, WTF);
    fft_32_s2(1, aux2, aux1);

    for (i=0;i<N/4;i++) {
        x1 = vec_ld( 0, &aux1[i*8]);
        x2 = vec_ld(16, &aux1[i*8]);
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


/* IFFT */
void ifft(unsigned int N, float (*x_r), float (*x_i), float (*X_R), float (*X_I), float **XX, float **x, float **X)
{
    unsigned int i, j;
    vector float xr, xi, x1, x2;
    vector unsigned char vmask1 = {0x00, 0x01, 0x02, 0x03, 0x08, 0x09, 0x0A, 0x0B, 0x10, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1A, 0x1B};
    vector unsigned char vmask2 = {0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F, 0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F};
    // TODO: Check performance whether we need more unroll instead of auto-unroll by compiler.
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
        vec_st(x1, 0, &aux1[i*8]);
        vec_st(x2,16, &aux1[i*8]);
    }

    /* Calculate FFT by a recursion. */
    fft_32_s1(-1, 1.0/(NFFT), aux1, aux2, WFT);
    fft_32_s2(-1, aux2, aux1);

    for (i=0;i<N/4;i++) {
        x1 = vec_ld( 0, &aux1[i*8]);
        x2 = vec_ld(16, &aux1[i*8]);
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
#else
float **XX;
float **x;
float **X;

int get_mem2F(float ***array2F, int rows, int columns)
{
    int i;

    if((*array2F = (float**)calloc(rows, sizeof(float*))) == NULL)
        exit(-1);

    if(((*array2F)[0] = (float*)calloc(columns*rows, sizeof(float))) == NULL)
        exit(-1);

    for(i=1;i<rows;i++)
        (*array2F)[i] = (*array2F)[i-1] + columns ;

    return rows*columns;
}


void free_mem2F(float **array2F)
{
    if (array2F)
    {
        if (array2F[0])
            free (array2F[0]);
        else
        {
            printf("free_mem2F: trying to free unused memory\n");
            exit(-1);
        }

        free (array2F);
    }
    else
    {
        printf("free_mem2F: trying to free unused memory\n");
        exit(-1);
    }

}

int fft_init(unsigned int N, float ***XX, float ***x, float ***X)
{
    int size=0;
    /* Declare a pointer to scratch space. */
    size += get_mem2F(XX, N, 2);
    size += get_mem2F(x, N, 2);
    size += get_mem2F(X, N, 2);

    return size;
}

void fft_quit(float **XX, float **x, float **X)
{
    /* Free memory. */
    free_mem2F(XX);
    free_mem2F(x);
    free_mem2F(X);
}

/* FFT recursion */
void fft_rec(unsigned int N, int offset, int delta,
             float **x, float **X, float **XX)
{
    int N2 = N/2;            /* half the number of points in FFT */
    int k;                   /* generic index */
    float cs, sn;            /* cosine and sine */
    int k00, k01, k10, k11;  /* indices for butterflies */
    float tmp0, tmp1;        /* temporary storage */

    if(N != 2)  /* Perform recursive step. */
    {
        /* Calculate two (N/2)-point DFT's. */
        fft_rec(N2, offset, 2*delta, x, XX, X);
        fft_rec(N2, offset+delta, 2*delta, x, XX, X);

        /* Combine the two (N/2)-point DFT's into one N-point DFT. */
        for(k=0; k<N2; k++)
        {
            k00 = offset + k*delta;    k01 = k00 + N2*delta;
            k10 = offset + 2*k*delta;  k11 = k10 + delta;
            cs = (float)cos((float)TWO_PI*k/(float)N); sn = (float)sin((float)TWO_PI*k/(float)N);
            tmp0 = cs * XX[k11][0] + sn * XX[k11][1];
            tmp1 = cs * XX[k11][1] - sn * XX[k11][0];
            X[k01][0] = XX[k10][0] - tmp0;
            X[k01][1] = XX[k10][1] - tmp1;
            X[k00][0] = XX[k10][0] + tmp0;
            X[k00][1] = XX[k10][1] + tmp1;
        }
/*        if (N == 32) {
          for(k=0; k<1024; k++) {
            printf("%10.6f %10.6f", X[k][0], X[k][1]);
            if (k%16 == 15) printf("\n");
          }
        }
*/
    }
    else  /* Perform 2-point DFT. */
    {
        k00 = offset; k01 = k00 + delta;
        X[k01][0] = x[k00][0] - x[k01][0];
        X[k01][1] = x[k00][1] - x[k01][1];
        X[k00][0] = x[k00][0] + x[k01][0];
        X[k00][1] = x[k00][1] + x[k01][1];
    }
}

void fft(unsigned int N, float (*x_r), float (*x_i), float (*X_R), float (*X_I),float **XX, float **x, float **X)
{
    unsigned int i;
    for (i=0;i<N;i++) {
        x[i][0]=x_r[i];
        x[i][1]=x_i[i];
    }

    /* Calculate FFT by a recursion. */
    fft_rec(N, 0, 1, x, X, XX);

    for (i=0;i<N;i++) {
        X_R[i]=X[i][0];
        X_I[i]=X[i][1];
    }
}


/* IFFT */
void ifft(unsigned int N, float (*x_r), float (*x_i), float (*X_R), float (*X_I), float **XX, float **x, float **X)
{
    int N2 = N/2;       /* half the number of points in IFFT */
    int i;              /* generic index */
    float tmp0, tmp1;  /* temporary storage */

    fft( N, X_R, X_I, x_r, x_i, XX, x, X);  // dxy, change the parameter;

    x_r[0] = x_r[0]/N;    x_i[0] = x_i[0]/N;
    x_r[N2] = x_r[N2]/N;  x_i[N2] = x_i[N2]/N;

    for(i=1; i<N2; i++)
    {
        tmp0 = x_r[i]/N;       tmp1 = x_i[i]/N;
        x_r[i] = x_r[N-i]/N;   x_i[i] = x_i[N-i]/N;
        x_r[N-i] = tmp0;       x_i[N-i] = tmp1;
    }
}
#endif


#if 0
int fft_init(int N)
{
    int size=0;
    /* Declare a pointer to scratch space. */
    size += get_mem2F(&XX, N, 2);
    size += get_mem2F(&x, N, 2);
    size += get_mem2F(&X, N, 2);

    return size;
}

void fft_quit()
{
    /* Free memory. */
    free_mem2F(XX);
    free_mem2F(x);
    free_mem2F(X);
}

/* FFT recursion */
void fft_rec(int N, int offset, int delta,
             float **x, float **X, float **XX)
{
    int N2 = N/2;            /* half the number of points in FFT */
    int k;                   /* generic index */
    float cs, sn;            /* cosine and sine */
    int k00, k01, k10, k11;  /* indices for butterflies */
    float tmp0, tmp1;        /* temporary storage */

    if(N != 2)  /* Perform recursive step. */
    {
        /* Calculate two (N/2)-point DFT's. */
        fft_rec(N2, offset, 2*delta, x, XX, X);
        fft_rec(N2, offset+delta, 2*delta, x, XX, X);

        /* Combine the two (N/2)-point DFT's into one N-point DFT. */
        for(k=0; k<N2; k++)
        {
            k00 = offset + k*delta;    k01 = k00 + N2*delta;
            k10 = offset + 2*k*delta;  k11 = k10 + delta;
            cs = (float)cos((float)TWO_PI*k/(float)N); sn = (float)sin((float)TWO_PI*k/(float)N);
            tmp0 = cs * XX[k11][0] + sn * XX[k11][1];
            tmp1 = cs * XX[k11][1] - sn * XX[k11][0];
            X[k01][0] = XX[k10][0] - tmp0;
            X[k01][1] = XX[k10][1] - tmp1;
            X[k00][0] = XX[k10][0] + tmp0;
            X[k00][1] = XX[k10][1] + tmp1;
        }
    }
    else  /* Perform 2-point DFT. */
    {
        k00 = offset; k01 = k00 + delta;
        X[k01][0] = x[k00][0] - x[k01][0];
        X[k01][1] = x[k00][1] - x[k01][1];
        X[k00][0] = x[k00][0] + x[k01][0];
        X[k00][1] = x[k00][1] + x[k01][1];
    }
}

void fft(int N, float (*x_r), float (*x_i), float (*X_R), float (*X_I))
{
    int i;
    for (i=0;i<N;i++) {
        x[i][0]=x_r[i];
        x[i][1]=x_i[i];
    }

    /* Calculate FFT by a recursion. */
    fft_rec(N, 0, 1, x, X, XX);

    for (i=0;i<N;i++) {
        X_R[i]=X[i][0];
        X_I[i]=X[i][1];
    }
}


/* IFFT */
void ifft(int N, float (*x_r), float (*x_i), float (*X_R), float (*X_I))
{
    int N2 = N/2;       /* half the number of points in IFFT */
    int i;              /* generic index */
    float tmp0, tmp1;  /* temporary storage */

    fft( N, X_R, X_I, x_r, x_i);  // dxy, change the parameter;

    x_r[0] = x_r[0]/N;    x_i[0] = x_i[0]/N;
    x_r[N2] = x_r[N2]/N;  x_i[N2] = x_i[N2]/N;

    for(i=1; i<N2; i++)
    {
        tmp0 = x_r[i]/N;       tmp1 = x_i[i]/N;
        x_r[i] = x_r[N-i]/N;   x_i[i] = x_i[N-i]/N;
        x_r[N-i] = tmp0;       x_i[N-i] = tmp1;
    }
}
#endif


