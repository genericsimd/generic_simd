/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: phy_ul_channelest.c

   Function: This function estimates one channel impulse response h(k) on each subcarrier
             by using MMSE channel estimation method.

   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

// Note: if compiled for Power7, output h_r, h_i must be 16-byte aligned, 
// what's more, para->ofdma_nused_no_dc is a multiple of 4

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "phy_ul_channelest.h"
#include "phy_ul_rx_common.h"

#ifdef SSE2OPT
#include <xmmintrin.h>
#include <emmintrin.h>
#endif

#ifdef VSXOPT
#include <altivec.h>
#endif

#ifdef GSIMD
#include <gsimd.h>
#endif

#ifndef TILE_SIZE
#define TILE_SIZE 210
#endif

/**----------------------------------------------------------------------------
   Function:    phy_ul_single_chanlest()

   Description: Calculate the h(k) without Multiple-antenna.
                MMSE channel estimation method is used.

   Parameters:

                Input-  [const struct phy_ul_sys_para *para]  The pointer refer to the
                        struct of system parameters.
                        [const float *subcarderand_r]  The pointer refer to real part of
                        3 OFDMA symbols, each of 840 samples.
                        [const float *subcarderand_i]  The pointer refer to imaginary
                        part of 3 OFDMA symbols, each of 840 samples.

                Output- [float *h_r]  The pointer refer to the real part of
                        channel impulse response, 3*840 samples.
                        [float *h_i]  The pointer refer to the imaginary part
                        of channel impulse response, 3*840 samples.

   Return Value:
                0       Success
                150     Error
   ----------------------------------------------------------------------------
   LOG END TAG zYx         */
#if (defined SSE2OPT || defined VSXOPT || defined GSIMD)
/*Define the MMSE matrix for SSE optimization,
the matrix should be stored with column major and 16-byte aligned*/
// the matrix can be resued for VSX optimization,
static float hcoef_r[12][4] __attribute__((aligned(16))) = {
    {0.4823, 0.3578, 0.2311, 0.1061},
    {0.1061, 0.2311, 0.3578, 0.4823},
    {0.3874, 0.2636, 0.1386, 0.0166},
    {0.0166, 0.1386, 0.2636, 0.3874},

    {0.4350, 0.3108, 0.1849, 0.0163},
    {0.0613, 0.1849, 0.3108, 0.4350},
    {0.4350, 0.3108, 0.1849, 0.0163},
    {0.0613, 0.1849, 0.3108, 0.4350},

    {0.3874, 0.2636, 0.1386, 0.0166},
    {0.0166, 0.1386, 0.2636, 0.3874},
    {0.4823, 0.3578, 0.2311, 0.1061},
    {0.1061, 0.2311, 0.3578, 0.4823}
};

static float hcoef_i[12][4] __attribute__((aligned(16)))= {
    {0.0000, -0.0341, -0.0445, -0.0311},
    {0.0311, 0.0445, 0.0341, 0.0000},
    {0.0000, -0.0251, -0.0267, -0.0049},
    {0.0049, 0.0267, 0.0251, 0.0000},

    {0.0000, -0.0296, -0.0356, -0.0180},
    {0.0180, 0.0356, 0.0296, 0.0000},
    {0.0000, -0.0296, -0.0356, -0.0180},
    {0.0180, 0.0356, 0.0296, 0.0000},

    {0.0000, -0.0251, -0.0267, -0.0049},
    {0.0049, 0.0267, 0.0251, 0.000},
    {0.0000, -0.0341, -0.0445, -0.0311},
    {0.0311, 0.0445, 0.0341, 0.0000}

};

#else
/*Define the MMSE matrix*/
static float hcoef_r[12][4] = {
    {0.4823, 0.1061, 0.3874, 0.0166},
    {0.3578, 0.2311, 0.2636, 0.1386},
    {0.2311, 0.3578, 0.1386, 0.2636},
    {0.1061, 0.4823, 0.0166, 0.3874},
	
    {0.4350, 0.0613, 0.4350, 0.0613},
    {0.3108, 0.1849, 0.3108, 0.1849},
    {0.1849, 0.3108, 0.1849, 0.3108},
    {0.0163, 0.4350, 0.0163, 0.4350},
	
    {0.3874, 0.0166, 0.4823, 0.1061},
    {0.2636, 0.1386, 0.3578, 0.2311},
    {0.1386, 0.2636, 0.2311, 0.3578},
    {0.0166, 0.3874, 0.1061, 0.4823}
};

static float hcoef_i[12][4] = {
    {0.0000, 0.0311, 0.0000, 0.0049},
    {-0.0341, 0.0445, -0.0251, 0.0267},
    {-0.0445, 0.0341, -0.0267, 0.0251},
    {-0.0311, 0.0000, -0.0049, 0.0000},
	
    {0.0000, 0.0180, 0.0000, 0.0180},
    {-0.0296, 0.0356, -0.0296, 0.0356},
    {-0.0356, 0.0296, -0.0356, 0.0296},
    {-0.0180, 0.0000, -0.0180, 0.0000},
	
    {0.0000, 0.0049, 0.0000, 0.0311},
    {-0.0251, 0.0267, -0.0341, 0.0445},
    {-0.0267, 0.0251, -0.0445, 0.0341},
    {-0.0049, 0.0000, -0.0311, 0.0000}

};
#endif

#ifdef GSIMD
// the kernel function estimates MMSE channel in tile group
// argument matrix is a 12x4 column-major coefficient matrix
// vectorin is pest vector
// length is channel estimation stride
// vectorout is outputed channel estimation
static
int complex_matrix_vector_muladd(const float *vectorin_r, const float *vectorin_i,
        int length, float *vectorout_r, float *vectorout_i)
{
  svec4_f vx_r1, vx_i1;
  svec4_f vx_r2, vx_i2;
  svec4_f vx_r3, vx_i3;
  svec4_f vx_r4, vx_i4;

  // coefficient matrix is treated as 3 groups.
  svec4_f vcoef_r11, vcoef_i11;
  svec4_f vcoef_r12, vcoef_i12;
  svec4_f vcoef_r13, vcoef_i13;
  svec4_f vcoef_r14, vcoef_i14;
  svec4_f vcoef_r21, vcoef_i21;
  svec4_f vcoef_r22, vcoef_i22;
  svec4_f vcoef_r23, vcoef_i23;
  svec4_f vcoef_r24, vcoef_i24;
  svec4_f vcoef_r31, vcoef_i31;
  svec4_f vcoef_r32, vcoef_i32;
  svec4_f vcoef_r33, vcoef_i33;
  svec4_f vcoef_r34, vcoef_i34;

  svec4_f vchan_r11, vchan_i11;
  svec4_f vchan_r21, vchan_i21;
  svec4_f vchan_r31, vchan_i31;

  // expand element into a scale vector
  vx_r4 = *(svec4_f*)(vectorin_r);
  vx_i4 = *(svec4_f*)(vectorin_i);

  vx_r1 = vx_r4.broadcast(0);
  vx_r2 = vx_r4.broadcast(1);
  vx_r3 = vx_r4.broadcast(2);
  vx_r4 = vx_r4.broadcast(3);

  vx_i1 = vx_i4.broadcast(0);
  vx_i2 = vx_i4.broadcast(1);
  vx_i3 = vx_i4.broadcast(2);
  vx_i4 = vx_i4.broadcast(3);

  vcoef_r11 = *(svec4_f*)(hcoef_r[0]);
  vcoef_i11 = *(svec4_f*)(hcoef_i[0]);
  vcoef_r12 = *(svec4_f*)(hcoef_r[1]);
  vcoef_i12 = *(svec4_f*)(hcoef_i[1]);
  vcoef_r13 = *(svec4_f*)(hcoef_r[2]);
  vcoef_i13 = *(svec4_f*)(hcoef_i[2]);
  vcoef_r14 = *(svec4_f*)(hcoef_r[3]);
  vcoef_i14 = *(svec4_f*)(hcoef_i[3]);

  // the first part

  vcoef_r21 = *(svec4_f*)(hcoef_r[4]);
  vcoef_i21 = *(svec4_f*)(hcoef_i[4]);
  vcoef_r22 = *(svec4_f*)(hcoef_r[5]);
  vcoef_i22 = *(svec4_f*)(hcoef_i[5]);
  vcoef_r23 = *(svec4_f*)(hcoef_r[6]);
  vcoef_i23 = *(svec4_f*)(hcoef_i[6]);
  vcoef_r24 = *(svec4_f*)(hcoef_r[7]);
  vcoef_i24 = *(svec4_f*)(hcoef_i[7]);
  // the second part

  vcoef_r31 = *(svec4_f*)(hcoef_r[8]);
  vcoef_i31 = *(svec4_f*)(hcoef_i[8]);
  vcoef_r32 = *(svec4_f*)(hcoef_r[9]);
  vcoef_i32 = *(svec4_f*)(hcoef_i[9]);
  vcoef_r33 = *(svec4_f*)(hcoef_r[10]);
  vcoef_i33 = *(svec4_f*)(hcoef_i[10]);
  vcoef_r34 = *(svec4_f*)(hcoef_r[11]);
  vcoef_i34 = *(svec4_f*)(hcoef_i[11]);
  // the third part

  vchan_r11 = svec_mul(vcoef_r11, vx_r1);
  vchan_r11 = svec_madd(vcoef_r12, vx_r2, vchan_r11);
  vchan_r11 = svec_madd(vcoef_r13, vx_r3, vchan_r11);
  vchan_r11 = svec_madd(vcoef_r14, vx_r4, vchan_r11);

  vchan_i11 = svec_mul(vcoef_i11, vx_r1);
  vchan_i11 = svec_madd(vcoef_i12, vx_r2, vchan_i11);
  vchan_i11 = svec_madd(vcoef_i13, vx_r3, vchan_i11);
  vchan_i11 = svec_madd(vcoef_i14, vx_r4, vchan_i11);

  vchan_r11 = svec_nmsub(vcoef_i11, vx_i1, vchan_r11);
  vchan_r11 = svec_nmsub(vcoef_i12, vx_i2, vchan_r11);
  vchan_r11 = svec_nmsub(vcoef_i13, vx_i3, vchan_r11);
  vchan_r11 = svec_nmsub(vcoef_i14, vx_i4, vchan_r11);

  vchan_i11 = svec_madd(vcoef_r11, vx_i1, vchan_i11);
  vchan_i11 = svec_madd(vcoef_r12, vx_i2, vchan_i11);
  vchan_i11 = svec_madd(vcoef_r13, vx_i3, vchan_i11);
  vchan_i11 = svec_madd(vcoef_r14, vx_i4, vchan_i11);

  vchan_r21 = svec_mul(vcoef_r21, vx_r1);
  vchan_r21 = svec_madd(vcoef_r22, vx_r2, vchan_r21);
  vchan_r21 = svec_madd(vcoef_r23, vx_r3, vchan_r21);
  vchan_r21 = svec_madd(vcoef_r24, vx_r4, vchan_r21);

  vchan_i21 = svec_mul(vcoef_i21, vx_r1);
  vchan_i21 = svec_madd(vcoef_i22, vx_r2, vchan_i21);
  vchan_i21 = svec_madd(vcoef_i23, vx_r3, vchan_i21);
  vchan_i21 = svec_madd(vcoef_i24, vx_r4, vchan_i21);

  vchan_r21 = svec_nmsub(vcoef_i21, vx_i1, vchan_r21);
  vchan_r21 = svec_nmsub(vcoef_i22, vx_i2, vchan_r21);
  vchan_r21 = svec_nmsub(vcoef_i23, vx_i3, vchan_r21);
  vchan_r21 = svec_nmsub(vcoef_i24, vx_i4, vchan_r21);

  vchan_i21 = svec_madd(vcoef_r21, vx_i1, vchan_i21);
  vchan_i21 = svec_madd(vcoef_r22, vx_i2, vchan_i21);
  vchan_i21 = svec_madd(vcoef_r23, vx_i3, vchan_i21);
  vchan_i21 = svec_madd(vcoef_r24, vx_i4, vchan_i21);

  vchan_r31 = svec_mul(vcoef_r31, vx_r1);
  vchan_r31 = svec_madd(vcoef_r32, vx_r2, vchan_r31);
  vchan_r31 = svec_madd(vcoef_r33, vx_r3, vchan_r31);
  vchan_r31 = svec_madd(vcoef_r34, vx_r4, vchan_r31);

  vchan_i31 = svec_mul(vcoef_i31, vx_r1);
  vchan_i31 = svec_madd(vcoef_i32, vx_r2, vchan_i31);
  vchan_i31 = svec_madd(vcoef_i33, vx_r3, vchan_i31);
  vchan_i31 = svec_madd(vcoef_i34, vx_r4, vchan_i31);

  vchan_r31 = svec_nmsub(vcoef_i31, vx_i1, vchan_r31);
  vchan_r31 = svec_nmsub(vcoef_i32, vx_i2, vchan_r31);
  vchan_r31 = svec_nmsub(vcoef_i33, vx_i3, vchan_r31);
  vchan_r31 = svec_nmsub(vcoef_i34, vx_i4, vchan_r31);

  vchan_i31 = svec_madd(vcoef_r31, vx_i1, vchan_i31);
  vchan_i31 = svec_madd(vcoef_r32, vx_i2, vchan_i31);
  vchan_i31 = svec_madd(vcoef_r33, vx_i3, vchan_i31);
  vchan_i31 = svec_madd(vcoef_r34, vx_i4, vchan_i31);

  *(svec4_f*)(vectorout_r) = vchan_r11;
  *(svec4_f*)(vectorout_i) = vchan_i11;

  *(svec4_f*)(&vectorout_r[length]) = vchan_r21;
  *(svec4_f*)(&vectorout_i[length]) = vchan_i21;

  *(svec4_f*)(&vectorout_r[2*length]) = vchan_r31;
  *(svec4_f*)(&vectorout_i[2*length]) = vchan_i31;
  return 0;
}

#elif defined VSXOPT
// the kernel function estimates MMSE channel in tile group
// argument matrix is a 12x4 column-major coefficient matrix
// vectorin is pest vector
// length is channel estimation stride
// vectorout is outputed channel estimation

static
int complex_matrix_vector_muladd(const float *vectorin_r, const float *vectorin_i, 
		int length, float *vectorout_r, float *vectorout_i)
{
  vector float vx_r1, vx_i1;
  vector float vx_r2, vx_i2;
  vector float vx_r3, vx_i3;
  vector float vx_r4, vx_i4;

  // coefficient matrix is treated as 3 groups.
  vector float vcoef_r11, vcoef_i11;
  vector float vcoef_r12, vcoef_i12;
  vector float vcoef_r13, vcoef_i13;
  vector float vcoef_r14, vcoef_i14;
  vector float vcoef_r21, vcoef_i21;
  vector float vcoef_r22, vcoef_i22;
  vector float vcoef_r23, vcoef_i23;
  vector float vcoef_r24, vcoef_i24;
  vector float vcoef_r31, vcoef_i31;
  vector float vcoef_r32, vcoef_i32;
  vector float vcoef_r33, vcoef_i33;
  vector float vcoef_r34, vcoef_i34;

  vector float vchan_r11, vchan_i11;
  vector float vchan_r21, vchan_i21;
  vector float vchan_r31, vchan_i31;

  // expand element into a scale vector
  vx_r4 = vec_ld(0, vectorin_r);
  vx_i4 = vec_ld(0, vectorin_i);

  vx_r1 = vec_splat(vx_r4, 0);
  vx_r2 = vec_splat(vx_r4, 1);
  vx_r3 = vec_splat(vx_r4, 2);
  vx_r4 = vec_splat(vx_r4, 3);

  vx_i1 = vec_splat(vx_i4, 0);
  vx_i2 = vec_splat(vx_i4, 1);
  vx_i3 = vec_splat(vx_i4, 2);
  vx_i4 = vec_splat(vx_i4, 3);

  vcoef_r11 = vec_ld(0, hcoef_r[0]);
  vcoef_i11 = vec_ld(0, hcoef_i[0]);
  vcoef_r12 = vec_ld(0, hcoef_r[1]);
  vcoef_i12 = vec_ld(0, hcoef_i[1]);
  vcoef_r13 = vec_ld(0, hcoef_r[2]);
  vcoef_i13 = vec_ld(0, hcoef_i[2]);
  vcoef_r14 = vec_ld(0, hcoef_r[3]);
  vcoef_i14 = vec_ld(0, hcoef_i[3]);

  // the first part

  vcoef_r21 = vec_ld(0, hcoef_r[4]);
  vcoef_i21 = vec_ld(0, hcoef_i[4]);
  vcoef_r22 = vec_ld(0, hcoef_r[5]);
  vcoef_i22 = vec_ld(0, hcoef_i[5]);
  vcoef_r23 = vec_ld(0, hcoef_r[6]);
  vcoef_i23 = vec_ld(0, hcoef_i[6]);
  vcoef_r24 = vec_ld(0, hcoef_r[7]);
  vcoef_i24 = vec_ld(0, hcoef_i[7]);
  // the second part

  vcoef_r31 = vec_ld(0, hcoef_r[8]);
  vcoef_i31 = vec_ld(0, hcoef_i[8]);
  vcoef_r32 = vec_ld(0, hcoef_r[9]);
  vcoef_i32 = vec_ld(0, hcoef_i[9]);
  vcoef_r33 = vec_ld(0, hcoef_r[10]);
  vcoef_i33 = vec_ld(0, hcoef_i[10]);
  vcoef_r34 = vec_ld(0, hcoef_r[11]);
  vcoef_i34 = vec_ld(0, hcoef_i[11]);
  // the third part

  vchan_r11 = vec_mul(vcoef_r11, vx_r1);
  vchan_r11 = vec_madd(vcoef_r12, vx_r2, vchan_r11);
  vchan_r11 = vec_madd(vcoef_r13, vx_r3, vchan_r11);
  vchan_r11 = vec_madd(vcoef_r14, vx_r4, vchan_r11);

  vchan_i11 = vec_mul(vcoef_i11, vx_r1);
  vchan_i11 = vec_madd(vcoef_i12, vx_r2, vchan_i11);
  vchan_i11 = vec_madd(vcoef_i13, vx_r3, vchan_i11);
  vchan_i11 = vec_madd(vcoef_i14, vx_r4, vchan_i11);

  vchan_r11 = vec_nmsub(vcoef_i11, vx_i1, vchan_r11);
  vchan_r11 = vec_nmsub(vcoef_i12, vx_i2, vchan_r11);
  vchan_r11 = vec_nmsub(vcoef_i13, vx_i3, vchan_r11);
  vchan_r11 = vec_nmsub(vcoef_i14, vx_i4, vchan_r11);

  vchan_i11 = vec_madd(vcoef_r11, vx_i1, vchan_i11);
  vchan_i11 = vec_madd(vcoef_r12, vx_i2, vchan_i11);
  vchan_i11 = vec_madd(vcoef_r13, vx_i3, vchan_i11);
  vchan_i11 = vec_madd(vcoef_r14, vx_i4, vchan_i11);

  vchan_r21 = vec_mul(vcoef_r21, vx_r1);
  vchan_r21 = vec_madd(vcoef_r22, vx_r2, vchan_r21);
  vchan_r21 = vec_madd(vcoef_r23, vx_r3, vchan_r21);
  vchan_r21 = vec_madd(vcoef_r24, vx_r4, vchan_r21);

  vchan_i21 = vec_mul(vcoef_i21, vx_r1);
  vchan_i21 = vec_madd(vcoef_i22, vx_r2, vchan_i21);
  vchan_i21 = vec_madd(vcoef_i23, vx_r3, vchan_i21);
  vchan_i21 = vec_madd(vcoef_i24, vx_r4, vchan_i21);

  vchan_r21 = vec_nmsub(vcoef_i21, vx_i1, vchan_r21);
  vchan_r21 = vec_nmsub(vcoef_i22, vx_i2, vchan_r21);
  vchan_r21 = vec_nmsub(vcoef_i23, vx_i3, vchan_r21);
  vchan_r21 = vec_nmsub(vcoef_i24, vx_i4, vchan_r21);

  vchan_i21 = vec_madd(vcoef_r21, vx_i1, vchan_i21);
  vchan_i21 = vec_madd(vcoef_r22, vx_i2, vchan_i21);
  vchan_i21 = vec_madd(vcoef_r23, vx_i3, vchan_i21);
  vchan_i21 = vec_madd(vcoef_r24, vx_i4, vchan_i21);

  vchan_r31 = vec_mul(vcoef_r31, vx_r1);
  vchan_r31 = vec_madd(vcoef_r32, vx_r2, vchan_r31);
  vchan_r31 = vec_madd(vcoef_r33, vx_r3, vchan_r31);
  vchan_r31 = vec_madd(vcoef_r34, vx_r4, vchan_r31);

  vchan_i31 = vec_mul(vcoef_i31, vx_r1);
  vchan_i31 = vec_madd(vcoef_i32, vx_r2, vchan_i31);
  vchan_i31 = vec_madd(vcoef_i33, vx_r3, vchan_i31);
  vchan_i31 = vec_madd(vcoef_i34, vx_r4, vchan_i31);

  vchan_r31 = vec_nmsub(vcoef_i31, vx_i1, vchan_r31);
  vchan_r31 = vec_nmsub(vcoef_i32, vx_i2, vchan_r31);
  vchan_r31 = vec_nmsub(vcoef_i33, vx_i3, vchan_r31);
  vchan_r31 = vec_nmsub(vcoef_i34, vx_i4, vchan_r31);

  vchan_i31 = vec_madd(vcoef_r31, vx_i1, vchan_i31);
  vchan_i31 = vec_madd(vcoef_r32, vx_i2, vchan_i31);
  vchan_i31 = vec_madd(vcoef_r33, vx_i3, vchan_i31);
  vchan_i31 = vec_madd(vcoef_r34, vx_i4, vchan_i31);

  vec_st(vchan_r11, 0, vectorout_r);
  vec_st(vchan_i11, 0, vectorout_i);

  vec_st(vchan_r21, 0, &vectorout_r[length]);
  vec_st(vchan_i21, 0, &vectorout_i[length]);

  vec_st(vchan_r31, 0, &vectorout_r[2*length]);
  vec_st(vchan_i31, 0, &vectorout_i[2*length]);

  return 0;
}


#elif defined SSE2OPT
static
int complex_matrix_vector_mul(float *matrix_r, float *matrix_i, float *vectorin_r, float *vectorin_i,
                              float *vectorout_r, float *vectorout_i)
{
    __m128 mvec;
    __m128 mtemp;
    __m128 mmtx0, mmtx1, mmtx2, mmtx3;
    __m128 mresult_r;
    __m128 mresult_i;

    //clear the register
    mresult_r = _mm_setzero_ps();
    mresult_i = _mm_setzero_ps();

    //load the real part of the matrix
    mmtx0 = _mm_load_ps(matrix_r);
    mmtx1 = _mm_load_ps(matrix_r + 4);
    mmtx2 = _mm_load_ps(matrix_r + 8);
    mmtx3 = _mm_load_ps(matrix_r + 12);

    //matrix_r * vector_r
    mvec = _mm_load_ps(vectorin_r);
    mtemp = _mm_shuffle_ps(mvec, mvec, 0x00);
    mtemp = _mm_mul_ps(mtemp, mmtx0);
    mresult_r = _mm_add_ps(mresult_r, mtemp);

    mtemp = _mm_shuffle_ps(mvec, mvec, 0x55);
    mtemp = _mm_mul_ps(mtemp, mmtx1);
    mresult_r = _mm_add_ps(mresult_r, mtemp);

    mtemp = _mm_shuffle_ps(mvec, mvec, 0xAA);
    mtemp = _mm_mul_ps(mtemp, mmtx2);
    mresult_r = _mm_add_ps(mresult_r, mtemp);

    mtemp = _mm_shuffle_ps(mvec, mvec, 0xFF);
    mtemp = _mm_mul_ps(mtemp, mmtx3);
    mresult_r = _mm_add_ps(mresult_r, mtemp);

    //matrix_r * vector_i
    mvec = _mm_load_ps(vectorin_i);
    mtemp = _mm_shuffle_ps(mvec, mvec, 0x00);
    mtemp = _mm_mul_ps(mtemp, mmtx0);
    mresult_i = _mm_add_ps(mresult_i, mtemp);

    mtemp = _mm_shuffle_ps(mvec, mvec, 0x55);
    mtemp = _mm_mul_ps(mtemp, mmtx1);
    mresult_i = _mm_add_ps(mresult_i, mtemp);

    mtemp = _mm_shuffle_ps(mvec, mvec, 0xAA);
    mtemp = _mm_mul_ps(mtemp, mmtx2);
    mresult_i = _mm_add_ps(mresult_i, mtemp);

    mtemp = _mm_shuffle_ps(mvec, mvec, 0xFF);
    mtemp = _mm_mul_ps(mtemp, mmtx3);
    mresult_i = _mm_add_ps(mresult_i, mtemp);

    //load the img part of the matrix
    mmtx0 = _mm_load_ps(matrix_i);
    mmtx1 = _mm_load_ps(matrix_i + 4);
    mmtx2 = _mm_load_ps(matrix_i + 8);
    mmtx3 = _mm_load_ps(matrix_i + 12);

    //matrix_i * vector_r
    mvec = _mm_load_ps(vectorin_r);
    mtemp = _mm_shuffle_ps(mvec, mvec, 0x00);
    mtemp = _mm_mul_ps(mtemp, mmtx0);
    mresult_i = _mm_add_ps(mresult_i, mtemp);

    mtemp = _mm_shuffle_ps(mvec, mvec, 0x55);
    mtemp = _mm_mul_ps(mtemp, mmtx1);
    mresult_i = _mm_add_ps(mresult_i, mtemp);

    mtemp = _mm_shuffle_ps(mvec, mvec, 0xAA);
    mtemp = _mm_mul_ps(mtemp, mmtx2);
    mresult_i = _mm_add_ps(mresult_i, mtemp);

    mtemp = _mm_shuffle_ps(mvec, mvec, 0xFF);
    mtemp = _mm_mul_ps(mtemp, mmtx3);
    mresult_i = _mm_add_ps(mresult_i, mtemp);

    //matrix_i * vector_i
    mvec = _mm_load_ps(vectorin_i);
    mtemp = _mm_shuffle_ps(mvec, mvec, 0x00);
    mtemp = _mm_mul_ps(mtemp, mmtx0);
    mresult_r = _mm_sub_ps(mresult_r, mtemp);

    mtemp = _mm_shuffle_ps(mvec, mvec, 0x55);
    mtemp = _mm_mul_ps(mtemp, mmtx1);
    mresult_r = _mm_sub_ps(mresult_r, mtemp);

    mtemp = _mm_shuffle_ps(mvec, mvec, 0xAA);
    mtemp = _mm_mul_ps(mtemp, mmtx2);
    mresult_r = _mm_sub_ps(mresult_r, mtemp);

    mtemp = _mm_shuffle_ps(mvec, mvec, 0xFF);
    mtemp = _mm_mul_ps(mtemp, mmtx3);
    mresult_r = _mm_sub_ps(mresult_r, mtemp);

    //store the results, real and img parts
    _mm_store_ps(vectorout_r, mresult_r);
    _mm_store_ps(vectorout_i, mresult_i);

    return 0;
}

#else
static
int complex_vector_muladd(float *a_r, float *a_i, float *b_r, float *b_i, float *c_r, float *c_i)
{
    unsigned int i;

    *c_r = 0;
    *c_i = 0;

    for(i = 0; i < 4; i++)
    {
        *c_r += a_r[i] * b_r[i] - a_i[i] * b_i[i];
        *c_i += a_r[i] * b_i[i] + a_i[i] * b_r[i];
    }

    return 0;
}
#endif

int32_t phy_ul_single_chanlest(const struct phy_ul_rx_syspara *para,
                               const float *subcarderand_r,
                               const float *subcarderand_i,
                               float *h_r,
                               float *h_i)
{
    float pilot_input;
    unsigned int tile_num;
    unsigned int subcar_num;
    unsigned int symbol_length_1;
    unsigned int symbol_length_2;
    unsigned int tile_idx;
    unsigned int subcar_idx;
    
    float pest_r[4] __attribute__((aligned(16)));
    float pest_i[4] __attribute__((aligned(16)));

    /* system parameters */
    tile_num = para->numoftiles;
    subcar_num = para->ofdma_nused_no_dc; 

    /* initialize some parameters */
    pilot_input = 1.0; //trnsmitted value on pilot position
    symbol_length_1 = subcar_num; /*length of suncars for single symbol*/
    symbol_length_2 = subcar_num * 2; /* length of subcars for two symbols */

    if (subcarderand_r == NULL || subcarderand_i == NULL)
    {
        printf("E001_chanlest: the pointer refer to input buffer is null!\n");
        return ERROR_CODE;
    }

    if (h_r == NULL || h_i == NULL)
    {
        printf("E002_chanlest: the pointer refer to output buffer is null!\n");
        return ERROR_CODE;
    }


    for (tile_idx = 0; tile_idx < 189; tile_idx++)
    {
	subcar_idx = tile_idx << 2;

        /*LS channel esimation in pilot position*/
        pest_r[0] = subcarderand_r[subcar_idx] / pilot_input;
        pest_i[0] = subcarderand_i[subcar_idx] / pilot_input;
        pest_r[1] = subcarderand_r[subcar_idx + 3] / pilot_input;
        pest_i[1] = subcarderand_i[subcar_idx + 3] / pilot_input;
        pest_r[2] = subcarderand_r[subcar_idx + symbol_length_2] / pilot_input;
        pest_i[2] = subcarderand_i[subcar_idx + symbol_length_2] / pilot_input;
        pest_r[3] = subcarderand_r[subcar_idx + 3 + symbol_length_2] / pilot_input;
        pest_i[3] = subcarderand_i[subcar_idx + 3 + symbol_length_2] / pilot_input;

#ifdef GSIMD
    complex_matrix_vector_muladd(pest_r, pest_i, subcar_num, &h_r[subcar_idx], &h_i[subcar_idx]);
#elif defined VSXOPT
	complex_matrix_vector_muladd(pest_r, pest_i, subcar_num, &h_r[subcar_idx], &h_i[subcar_idx]);
#elif defined SSE2OPT
	/*Using SSE to perform matrix and vector multi*/
        complex_matrix_vector_mul(hcoef_r[0], hcoef_i[0], pest_r, pest_i,
                                  &h_r[subcar_idx],
                                  &h_i[subcar_idx]);

        complex_matrix_vector_mul(hcoef_r[4], hcoef_i[4], pest_r, pest_i,
                                  &h_r[subcar_idx  + symbol_length_1],
                                  &h_i[subcar_idx  + symbol_length_1]);

        complex_matrix_vector_mul(hcoef_r[8], hcoef_i[8], pest_r, pest_i,
                                  &h_r[subcar_idx  + symbol_length_2],
                                  &h_i[subcar_idx  + symbol_length_2]);

#else
        int i;
        /*MMSE channel estimation in each tile, hcoef * pest*/
        for(i = 0; i < 12; i++)
        {
            complex_vector_muladd(hcoef_r[i], hcoef_i[i], pest_r, pest_i,
                                  &h_r[subcar_idx  + i / 4 * symbol_length_1 + i % 4],
                                  &h_i[subcar_idx  + i / 4 * symbol_length_1 + i % 4]);
        }

#endif
    }
    return SUCCESS_CODE;
}


