/* ----------------------------------------------------------------------------
    IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.
  

   File Name: phy_ul_demodulation.c

   Function:  demodulation in the receiver

   Change Activity:
   Date             Description of Change                            By
   -----------      ---------------------                            --------


   ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "phy_ul_rx_interface.h"
#include "phy_ul_rx_common.h"
#include "phy_ul_demodulation.h"
#include "flog.h"

#ifdef SSE2OPT
#include <xmmintrin.h>
#include <emmintrin.h>
#endif

#ifdef VSXOPT
#include <altivec.h>
#endif

#define TSUBCARRIER_NUM 72 //total subcarriers in each slot
#define DSUBCARRIER_NUM 48 //total data subcarriers in each slot
#define PSUBCARRIER_NUM 24 //total pilot subcarrier in each slot
#define SOFTBIT_NUM_MAX 288 //48*6 max softbit in each slot
 
#ifdef VSXOPT
// it is assumed all data are well aligned
// all kind of sizes are a multiple of 4
// x = x + y
static void vector_region_add(float *x, const float *y, int len){
	int i;
	vector float vx, vy;
	for(i=0;i<len;i+=4){
		vx = vec_ld(0, &x[i]);
		vy = vec_ld(0, &y[i]);
		vx = vec_add(vx, vy);
		vec_st(vx, 0, &x[i]);
	}
}

// phy_ul_demodulation_xxx is used by phy_ul_demodulation_single & phy_ul_demodulation
// subcarrier, hest input
// softbit is returned
static void phy_ul_demodulation_qpsk34(const float *subcarrier_r,
                                        const float *subcarrier_i,
                                        const float *hest_r,
                                        const float *hest_i,
                                        int slot_num,
                                        const float *noise_power,
                                        float *softbit)
{
  int i, slot_idx,step;
  int k1,k2,k3,k4;
  float factor __attribute__((aligned(16)));

  // const scale factor for code type demodulation
  static const float scal_fact = -2.828427125; // -4.0/sqrt(2.0)
  vector float vhest_r1, vhest_i1;
  vector float vhest_r2, vhest_i2;
  vector float vhest_r3, vhest_i3;
  vector float vhest_r4, vhest_i4;
  vector float vsubc_r1, vsubc_i1;
  vector float vsubc_r2, vsubc_i2;
  vector float vsubc_r3, vsubc_i3;
  vector float vsubc_r4, vsubc_i4;
  vector float vseq_r1, vseq_i1;
  vector float vseq_r2, vseq_i2;
  vector float vseq_r3, vseq_i3;
  vector float vseq_r4, vseq_i4;
  vector float vfactor;

  // initilize offset of the 4 column falimilies
  // DSUBCARRIER_NUM is a multiple of 4;
  step = DSUBCARRIER_NUM/4;
  k1 = 0;
  k2 = k1 + step;
  k3 = k2 + step;
  k4 = k3 + step;

  for(slot_idx = 0; slot_idx < slot_num; slot_idx++)
  {
	  factor = scal_fact / noise_power[slot_idx];

	  // load/splat a scalar factor
	  vfactor = (vector float){factor, 0., 0., 0.};
	  vfactor = vec_splat(vfactor, 0);

	  //perform the demodulation slot by slot
	  //start from 4 parts.
	  for (i = 0; i < step/4; i++)
	  {
		  vhest_r1 = vec_ld(0, &hest_r[k1]);
		  vhest_i1 = vec_ld(0, &hest_i[k1]);
		  vhest_r2 = vec_ld(0, &hest_r[k2]);
		  vhest_i2 = vec_ld(0, &hest_i[k2]);
		  vhest_r3 = vec_ld(0, &hest_r[k3]);
		  vhest_i3 = vec_ld(0, &hest_i[k3]);
		  vhest_r4 = vec_ld(0, &hest_r[k4]);
		  vhest_i4 = vec_ld(0, &hest_i[k4]);

		  vsubc_r1 = vec_ld(0, &subcarrier_r[k1]);
		  vsubc_i1 = vec_ld(0, &subcarrier_i[k1]);
		  vsubc_r2 = vec_ld(0, &subcarrier_r[k2]);
		  vsubc_i2 = vec_ld(0, &subcarrier_i[k2]);
		  vsubc_r3 = vec_ld(0, &subcarrier_r[k3]);
		  vsubc_i3 = vec_ld(0, &subcarrier_i[k3]);
		  vsubc_r4 = vec_ld(0, &subcarrier_r[k4]);
		  vsubc_i4 = vec_ld(0, &subcarrier_i[k4]);

		  vseq_r1 = vec_mul(vsubc_r1, vhest_r1);
		  vseq_r2 = vec_mul(vsubc_r2, vhest_r2);
		  vseq_r3 = vec_mul(vsubc_r3, vhest_r3);
		  vseq_r4 = vec_mul(vsubc_r4, vhest_r4);

		  vseq_i1 = vec_mul(vsubc_i1, vhest_r1);
		  vseq_i2 = vec_mul(vsubc_i2, vhest_r2);
		  vseq_i3 = vec_mul(vsubc_i3, vhest_r3);
		  vseq_i4 = vec_mul(vsubc_i4, vhest_r4);

		  vseq_r1 = vec_madd(vsubc_i1, vhest_i1, vseq_r1);
		  vseq_r2 = vec_madd(vsubc_i2, vhest_i2, vseq_r2);
		  vseq_r3 = vec_madd(vsubc_i3, vhest_i3, vseq_r3);
		  vseq_r4 = vec_madd(vsubc_i4, vhest_i4, vseq_r4);

		  vseq_i1 = vec_nmsub(vsubc_r1, vhest_i1, vseq_i1);
		  vseq_i2 = vec_nmsub(vsubc_r2, vhest_i2, vseq_i2);
		  vseq_i3 = vec_nmsub(vsubc_r3, vhest_i3, vseq_i3);
		  vseq_i4 = vec_nmsub(vsubc_r4, vhest_i4, vseq_i4);

		  // shuffle through vector merge
		  vsubc_r1 = vec_mergeh(vseq_r1, vseq_i1);
		  vsubc_r2 = vec_mergeh(vseq_r2, vseq_i2);
		  vsubc_r3 = vec_mergeh(vseq_r3, vseq_i3);
		  vsubc_r4 = vec_mergeh(vseq_r4, vseq_i4);

		  vsubc_i1 = vec_mergel(vseq_r1, vseq_i1);
		  vsubc_i2 = vec_mergel(vseq_r2, vseq_i2);
		  vsubc_i3 = vec_mergel(vseq_r3, vseq_i3);
		  vsubc_i4 = vec_mergel(vseq_r4, vseq_i4);

		  vseq_r1 = vec_mul(vsubc_r1, vfactor);
		  vseq_r2 = vec_mul(vsubc_r2, vfactor);
		  vseq_r3 = vec_mul(vsubc_r3, vfactor);
		  vseq_r4 = vec_mul(vsubc_r4, vfactor);

		  vseq_i1 = vec_mul(vsubc_i1, vfactor);
		  vseq_i2 = vec_mul(vsubc_i2, vfactor);
		  vseq_i3 = vec_mul(vsubc_i3, vfactor);
		  vseq_i4 = vec_mul(vsubc_i4, vfactor);

		  vec_st(vseq_r1, 0, &softbit[2*k1]);
		  vec_st(vseq_i1, 0, &softbit[2*k1+4]);
		  vec_st(vseq_r2, 0, &softbit[2*k2]);
		  vec_st(vseq_i2, 0, &softbit[2*k2+4]);
		  vec_st(vseq_r3, 0, &softbit[2*k3]);
		  vec_st(vseq_i3, 0, &softbit[2*k3+4]);
		  vec_st(vseq_r4, 0, &softbit[2*k4]);
		  vec_st(vseq_i4, 0, &softbit[2*k4+4]);
		  k1 = k1 + 4;
		  k2 = k2 + 4;
		  k3 = k3 + 4;
		  k4 = k4 + 4;
	  }
	  // adjust index
	k1 = k4;
	k2 = k1 + step;
	k3 = k2 + step;
	k4 = k3 + step;
  }
}

static void phy_ul_demodulation_qam16_34(const float *subcarrier_r,
                                        const float *subcarrier_i,
                                        const float *hest_r,
                                        const float *hest_i,
                                        int slot_num,
                                        const float *noise_power,
                                        float *softbit)
{
  int i, slot_idx,step;
  int k1,k2,k3,k4;
  float factor __attribute__((aligned(16)));

  // const scale factor for code type demodulation
  static const float tmp_scal[5] __attribute__((aligned(16))) = {
	  0.632455532, // 2.0/sqrt(10.0)
	  0.632455532, // 2.0/sqrt(10.0)
	  0.632455532, // 2.0/sqrt(10.0)
	  0.632455532, // 2.0/sqrt(10.0)
	 -1.264911064  //-4.0/sqrt(10.0)
  };

  vector float vhest_r1, vhest_i1;
  vector float vhest_r2, vhest_i2;
  vector float vhest_r3, vhest_i3;
  vector float vhest_r4, vhest_i4;
  vector float vsubc_r1, vsubc_i1;
  vector float vsubc_r2, vsubc_i2;
  vector float vsubc_r3, vsubc_i3;
  vector float vsubc_r4, vsubc_i4;
  vector float vseq_r1, vseq_i1;
  vector float vseq_r2, vseq_i2;
  vector float vseq_r3, vseq_i3;
  vector float vseq_r4, vseq_i4;
  vector float vx_1, vx_2, vx_3, vx_4;
  vector float vfactor, vtmp;

  // initilize offset of the 4 column falimilies
  // DSUBCARRIER_NUM is a multiple of 4;
  step = DSUBCARRIER_NUM/4;
  k1 = 0;
  k2 = k1 + step;
  k3 = k2 + step;
  k4 = k3 + step;

  vtmp = vec_ld(0, tmp_scal);
  for(slot_idx = 0; slot_idx < slot_num; slot_idx++)
  {
	  factor = tmp_scal[4] / noise_power[slot_idx];

	  // load/splat a scalar factor
	  vfactor = (vector float){factor, 0., 0., 0.};
	  vfactor = vec_splat(vfactor, 0);

	  //perform the demodulation slot by slot
	  //start from 4 parts.
	  for (i = 0; i < step/4; i++)
	  {
		  vhest_r1 = vec_ld(0, &hest_r[k1]);
		  vhest_i1 = vec_ld(0, &hest_i[k1]);
		  vhest_r2 = vec_ld(0, &hest_r[k2]);
		  vhest_i2 = vec_ld(0, &hest_i[k2]);
		  vhest_r3 = vec_ld(0, &hest_r[k3]);
		  vhest_i3 = vec_ld(0, &hest_i[k3]);
		  vhest_r4 = vec_ld(0, &hest_r[k4]);
		  vhest_i4 = vec_ld(0, &hest_i[k4]);

		  vsubc_r1 = vec_ld(0, &subcarrier_r[k1]);
		  vsubc_i1 = vec_ld(0, &subcarrier_i[k1]);
		  vsubc_r2 = vec_ld(0, &subcarrier_r[k2]);
		  vsubc_i2 = vec_ld(0, &subcarrier_i[k2]);
		  vsubc_r3 = vec_ld(0, &subcarrier_r[k3]);
		  vsubc_i3 = vec_ld(0, &subcarrier_i[k3]);
		  vsubc_r4 = vec_ld(0, &subcarrier_r[k4]);
		  vsubc_i4 = vec_ld(0, &subcarrier_i[k4]);

// complex(subcarrier)*conj(complex(vhest))
		  vseq_r1 = vec_mul(vsubc_r1, vhest_r1);
		  vseq_r2 = vec_mul(vsubc_r2, vhest_r2);
		  vseq_r3 = vec_mul(vsubc_r3, vhest_r3);
		  vseq_r4 = vec_mul(vsubc_r4, vhest_r4);

		  vseq_i1 = vec_mul(vsubc_i1, vhest_r1);
		  vseq_i2 = vec_mul(vsubc_i2, vhest_r2);
		  vseq_i3 = vec_mul(vsubc_i3, vhest_r3);
		  vseq_i4 = vec_mul(vsubc_i4, vhest_r4);

		  vseq_r1 = vec_madd(vsubc_i1, vhest_i1, vseq_r1);
		  vseq_r2 = vec_madd(vsubc_i2, vhest_i2, vseq_r2);
		  vseq_r3 = vec_madd(vsubc_i3, vhest_i3, vseq_r3);
		  vseq_r4 = vec_madd(vsubc_i4, vhest_i4, vseq_r4);

		  vseq_i1 = vec_nmsub(vsubc_r1, vhest_i1, vseq_i1);
		  vseq_i2 = vec_nmsub(vsubc_r2, vhest_i2, vseq_i2);
		  vseq_i3 = vec_nmsub(vsubc_r3, vhest_i3, vseq_i3);
		  vseq_i4 = vec_nmsub(vsubc_r4, vhest_i4, vseq_i4);

// ||complex(vhest)||
		  vhest_i1 = vec_mul(vhest_i1, vhest_i1);
		  vhest_i2 = vec_mul(vhest_i2, vhest_i2);
		  vhest_i3 = vec_mul(vhest_i3, vhest_i3);
		  vhest_i4 = vec_mul(vhest_i4, vhest_i4);

		  vhest_r1 = vec_madd(vhest_r1, vhest_r1, vhest_i1);
		  vhest_r2 = vec_madd(vhest_r2, vhest_r2, vhest_i2);
		  vhest_r3 = vec_madd(vhest_r3, vhest_r3, vhest_i3);
		  vhest_r4 = vec_madd(vhest_r4, vhest_r4, vhest_i4);

// get scale factor
		  vhest_r1 = vec_mul(vhest_r1, vtmp);
		  vhest_r2 = vec_mul(vhest_r2, vtmp);
		  vhest_r3 = vec_mul(vhest_r3, vtmp);
		  vhest_r4 = vec_mul(vhest_r4, vtmp);

// scale complex(symbol_eq)
		  vseq_r1 = vec_mul(vseq_r1, vfactor);
		  vseq_r2 = vec_mul(vseq_r2, vfactor);
		  vseq_r3 = vec_mul(vseq_r3, vfactor);
		  vseq_r4 = vec_mul(vseq_r4, vfactor);

		  vseq_i1 = vec_mul(vseq_i1, vfactor);
		  vseq_i2 = vec_mul(vseq_i2, vfactor);
		  vseq_i3 = vec_mul(vseq_i3, vfactor);
		  vseq_i4 = vec_mul(vseq_i4, vfactor);
// 
		  vhest_r1 = vec_mul(vhest_r1, vfactor);
		  vhest_r2 = vec_mul(vhest_r2, vfactor);
		  vhest_r3 = vec_mul(vhest_r3, vfactor);
		  vhest_r4 = vec_mul(vhest_r4, vfactor);

		  vx_1 = vec_abs(vseq_r1);
		  vx_2 = vec_abs(vseq_r2);
		  vx_3 = vec_abs(vseq_r3);
		  vx_4 = vec_abs(vseq_r4);

// reuse vsubc_rx
		  vsubc_r1 = vec_add(vhest_r1, vx_1);
		  vsubc_r2 = vec_add(vhest_r2, vx_2);
		  vsubc_r3 = vec_add(vhest_r3, vx_3);
		  vsubc_r4 = vec_add(vhest_r4, vx_4);

		  vx_1 = vec_abs(vseq_i1);
		  vx_2 = vec_abs(vseq_i2);
		  vx_3 = vec_abs(vseq_i3);
		  vx_4 = vec_abs(vseq_i4);

// reuse vsubc_ix
		  vsubc_i1 = vec_add(vhest_r1, vx_1);
		  vsubc_i2 = vec_add(vhest_r2, vx_2);
		  vsubc_i3 = vec_add(vhest_r3, vx_3);
		  vsubc_i4 = vec_add(vhest_r4, vx_4);

		  // transpose the matrix with vector merge high/low
		  // vseq_r, vsubc_r, vseq_i, vsubc_i
		  vhest_r1 = vec_mergeh(vseq_r1, vseq_i1);
		  vhest_i1 = vec_mergel(vseq_r1, vseq_i1);
		  vseq_r1 = vec_mergeh(vsubc_r1, vsubc_i1);
		  vseq_i1 = vec_mergel(vsubc_r1, vsubc_i1);

		  vsubc_r1 = vec_mergeh(vhest_r1, vseq_r1);
		  vsubc_i1 = vec_mergel(vhest_r1, vseq_r1);
		  vseq_r1 = vec_mergeh(vhest_i1, vseq_i1);
		  vseq_i1 = vec_mergel(vhest_i1, vseq_i1);

		  vec_st(vsubc_r1, 0, &softbit[4*k1]);
		  vec_st(vsubc_i1, 0, &softbit[4*k1+4]);
		  vec_st(vseq_r1, 0, &softbit[4*k1+8]);
		  vec_st(vseq_i1, 0, &softbit[4*k1+12]);

		  vhest_r2 = vec_mergeh(vseq_r2, vseq_i2);
		  vhest_i2 = vec_mergel(vseq_r2, vseq_i2);
		  vseq_r2 = vec_mergeh(vsubc_r2, vsubc_i2);
		  vseq_i2 = vec_mergel(vsubc_r2, vsubc_i2);

		  vsubc_r2 = vec_mergeh(vhest_r2, vseq_r2);
		  vsubc_i2 = vec_mergel(vhest_r2, vseq_r2);
		  vseq_r2 = vec_mergeh(vhest_i2, vseq_i2);
		  vseq_i2 = vec_mergel(vhest_i2, vseq_i2);

		  vec_st(vsubc_r2, 0, &softbit[4*k2]);
		  vec_st(vsubc_i2, 0, &softbit[4*k2+4]);
		  vec_st(vseq_r2, 0, &softbit[4*k2+8]);
		  vec_st(vseq_i2, 0, &softbit[4*k2+12]);

		  vhest_r3 = vec_mergeh(vseq_r3, vseq_i3);
		  vhest_i3 = vec_mergel(vseq_r3, vseq_i3);
		  vseq_r3 = vec_mergeh(vsubc_r3, vsubc_i3);
		  vseq_i3 = vec_mergel(vsubc_r3, vsubc_i3);

		  vsubc_r3 = vec_mergeh(vhest_r3, vseq_r3);
		  vsubc_i3 = vec_mergel(vhest_r3, vseq_r3);
		  vseq_r3 = vec_mergeh(vhest_i3, vseq_i3);
		  vseq_i3 = vec_mergel(vhest_i3, vseq_i3);

		  vec_st(vsubc_r3, 0, &softbit[4*k3]);
		  vec_st(vsubc_i3, 0, &softbit[4*k3+4]);
		  vec_st(vseq_r3, 0, &softbit[4*k3+8]);
		  vec_st(vseq_i3, 0, &softbit[4*k3+12]);

		  vhest_r4 = vec_mergeh(vseq_r4, vseq_i4);
		  vhest_i4 = vec_mergel(vseq_r4, vseq_i4);
		  vseq_r4 = vec_mergeh(vsubc_r4, vsubc_i4);
		  vseq_i4 = vec_mergel(vsubc_r4, vsubc_i4);

		  vsubc_r4 = vec_mergeh(vhest_r4, vseq_r4);
		  vsubc_i4 = vec_mergel(vhest_r4, vseq_r4);
		  vseq_r4 = vec_mergeh(vhest_i4, vseq_i4);
		  vseq_i4 = vec_mergel(vhest_i4, vseq_i4);

		  vec_st(vsubc_r4, 0, &softbit[4*k4]);
		  vec_st(vsubc_i4, 0, &softbit[4*k4+4]);
		  vec_st(vseq_r4, 0, &softbit[4*k4+8]);
		  vec_st(vseq_i4, 0, &softbit[4*k4+12]);

		  k1 = k1 + 4;
		  k2 = k2 + 4;
		  k3 = k3 + 4;
		  k4 = k4 + 4;
	  }
	  // adjust index
	k1 = k4;
	k2 = k1 + step;
	k3 = k2 + step;
	k4 = k3 + step;
  }
}

static void phy_ul_demodulation_qam64_34(const float *subcarrier_r,
                                        const float *subcarrier_i,
                                        const float *hest_r,
                                        const float *hest_i,
                                        int slot_num,
                                        const float *noise_power,
                                        float *softbit)
{
  int i, slot_idx,step;
  int k1,k2,k3,k4;
  float factor __attribute__((aligned(16)));

  // const scale factor for code type demodulation
  static const float tmp_scal[5] __attribute__((aligned(16))) = {
	  0.3086067, // 2.0/sqrt(42.0)
	  0.3086067, // 2.0/sqrt(42.0)
	  0.3086067, // 2.0/sqrt(42.0)
	  0.3086067, // 2.0/sqrt(42.0)
	  -0.6172134 // -4.0/sqrt(42.0)
  };

  vector float vhest_r1, vhest_i1;
  vector float vhest_r2, vhest_i2;
  vector float vhest_r3, vhest_i3;
  vector float vhest_r4, vhest_i4;
  vector float vsubc_r1, vsubc_i1;
  vector float vsubc_r2, vsubc_i2;
  vector float vsubc_r3, vsubc_i3;
  vector float vsubc_r4, vsubc_i4;
  vector float vseq_r1, vseq_i1;
  vector float vseq_r2, vseq_i2;
  vector float vseq_r3, vseq_i3;
  vector float vseq_r4, vseq_i4;
  vector float vx_h1, vx_l1;
  vector float vx_h2, vx_l2;
  vector float vx_h3, vx_l3;
  vector float vx_h4, vx_l4;
  vector float vfactor, vtmp;

// declared as static
  static vector unsigned char pat1 = (vector unsigned char){\
	  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,\
		  0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
  static vector unsigned char pat2 = (vector unsigned char){\
	  0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,\
		  0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f};
  static vector unsigned char pat3 = (vector unsigned char){\
	  0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,\
		  0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f};


  // initilize offset of the 4 column falimilies
  // DSUBCARRIER_NUM is a multiple of 4;
  step = DSUBCARRIER_NUM/4;
  k1 = 0;
  k2 = k1 + step;
  k3 = k2 + step;
  k4 = k3 + step;

  vtmp = vec_ld(0, tmp_scal);
  for(slot_idx = 0; slot_idx < slot_num; slot_idx++)
  {
	  factor = tmp_scal[4] / noise_power[slot_idx];

	  // load/splat a scalar factor
	  vfactor = (vector float){factor, 0., 0., 0.};
	  vfactor = vec_splat(vfactor, 0);

	  //perform the demodulation slot by slot
	  //start from 4 parts.
	  for (i = 0; i < step/4; i++)
	  {
		  vhest_r1 = vec_ld(0, &hest_r[k1]);
		  vhest_i1 = vec_ld(0, &hest_i[k1]);
		  vhest_r2 = vec_ld(0, &hest_r[k2]);
		  vhest_i2 = vec_ld(0, &hest_i[k2]);
		  vhest_r3 = vec_ld(0, &hest_r[k3]);
		  vhest_i3 = vec_ld(0, &hest_i[k3]);
		  vhest_r4 = vec_ld(0, &hest_r[k4]);
		  vhest_i4 = vec_ld(0, &hest_i[k4]);

		  vsubc_r1 = vec_ld(0, &subcarrier_r[k1]);
		  vsubc_i1 = vec_ld(0, &subcarrier_i[k1]);
		  vsubc_r2 = vec_ld(0, &subcarrier_r[k2]);
		  vsubc_i2 = vec_ld(0, &subcarrier_i[k2]);
		  vsubc_r3 = vec_ld(0, &subcarrier_r[k3]);
		  vsubc_i3 = vec_ld(0, &subcarrier_i[k3]);
		  vsubc_r4 = vec_ld(0, &subcarrier_r[k4]);
		  vsubc_i4 = vec_ld(0, &subcarrier_i[k4]);

// complex(subcarrier)*conj(complex(vhest))
		  vseq_r1 = vec_mul(vsubc_r1, vhest_r1);
		  vseq_r2 = vec_mul(vsubc_r2, vhest_r2);
		  vseq_r3 = vec_mul(vsubc_r3, vhest_r3);
		  vseq_r4 = vec_mul(vsubc_r4, vhest_r4);

		  vseq_i1 = vec_mul(vsubc_i1, vhest_r1);
		  vseq_i2 = vec_mul(vsubc_i2, vhest_r2);
		  vseq_i3 = vec_mul(vsubc_i3, vhest_r3);
		  vseq_i4 = vec_mul(vsubc_i4, vhest_r4);

		  vseq_r1 = vec_madd(vsubc_i1, vhest_i1, vseq_r1);
		  vseq_r2 = vec_madd(vsubc_i2, vhest_i2, vseq_r2);
		  vseq_r3 = vec_madd(vsubc_i3, vhest_i3, vseq_r3);
		  vseq_r4 = vec_madd(vsubc_i4, vhest_i4, vseq_r4);

		  vseq_i1 = vec_nmsub(vsubc_r1, vhest_i1, vseq_i1);
		  vseq_i2 = vec_nmsub(vsubc_r2, vhest_i2, vseq_i2);
		  vseq_i3 = vec_nmsub(vsubc_r3, vhest_i3, vseq_i3);
		  vseq_i4 = vec_nmsub(vsubc_r4, vhest_i4, vseq_i4);

// ||complex(vhest)||
		  vhest_i1 = vec_mul(vhest_i1, vhest_i1);
		  vhest_i2 = vec_mul(vhest_i2, vhest_i2);
		  vhest_i3 = vec_mul(vhest_i3, vhest_i3);
		  vhest_i4 = vec_mul(vhest_i4, vhest_i4);

		  vhest_r1 = vec_madd(vhest_r1, vhest_r1, vhest_i1);
		  vhest_r2 = vec_madd(vhest_r2, vhest_r2, vhest_i2);
		  vhest_r3 = vec_madd(vhest_r3, vhest_r3, vhest_i3);
		  vhest_r4 = vec_madd(vhest_r4, vhest_r4, vhest_i4);

// get scale factor
		  vhest_r1 = vec_mul(vhest_r1, vtmp);
		  vhest_r2 = vec_mul(vhest_r2, vtmp);
		  vhest_r3 = vec_mul(vhest_r3, vtmp);
		  vhest_r4 = vec_mul(vhest_r4, vtmp);

// scale complex(symbol_eq)
		  vseq_r1 = vec_mul(vseq_r1, vfactor);
		  vseq_r2 = vec_mul(vseq_r2, vfactor);
		  vseq_r3 = vec_mul(vseq_r3, vfactor);
		  vseq_r4 = vec_mul(vseq_r4, vfactor);

		  vseq_i1 = vec_mul(vseq_i1, vfactor);
		  vseq_i2 = vec_mul(vseq_i2, vfactor);
		  vseq_i3 = vec_mul(vseq_i3, vfactor);
		  vseq_i4 = vec_mul(vseq_i4, vfactor);

// temp*factor
		  vhest_i1 = vec_mul(vhest_r1, vfactor);
		  vhest_i2 = vec_mul(vhest_r2, vfactor);
		  vhest_i3 = vec_mul(vhest_r3, vfactor);
		  vhest_i4 = vec_mul(vhest_r4, vfactor);

// 2*temp*factor
		  vhest_r1 = vec_add(vhest_i1, vhest_i1);
		  vhest_r2 = vec_add(vhest_i2, vhest_i2);
		  vhest_r3 = vec_add(vhest_i3, vhest_i3);
		  vhest_r4 = vec_add(vhest_i4, vhest_i4);

// reuse vsubc_rx
		  vsubc_r1 = vec_add(vhest_r1, vec_abs(vseq_r1));
		  vsubc_r2 = vec_add(vhest_r2, vec_abs(vseq_r2));
		  vsubc_r3 = vec_add(vhest_r3, vec_abs(vseq_r3));
		  vsubc_r4 = vec_add(vhest_r4, vec_abs(vseq_r4));

// reuse vsubc_ix
		  vsubc_i1 = vec_add(vhest_r1, vec_abs(vseq_i1));
		  vsubc_i2 = vec_add(vhest_r2, vec_abs(vseq_i2));
		  vsubc_i3 = vec_add(vhest_r3, vec_abs(vseq_i3));
		  vsubc_i4 = vec_add(vhest_r4, vec_abs(vseq_i4));

// first reuse vhest_rx
		  vhest_r1 = vec_add(vhest_i1, vec_abs(vsubc_r1));
		  vhest_r2 = vec_add(vhest_i2, vec_abs(vsubc_r2));
		  vhest_r3 = vec_add(vhest_i3, vec_abs(vsubc_r3));
		  vhest_r4 = vec_add(vhest_i4, vec_abs(vsubc_r4));

// then reuse vhest_ix
		  vhest_i1 = vec_add(vhest_i1, vec_abs(vsubc_i1));
		  vhest_i2 = vec_add(vhest_i2, vec_abs(vsubc_i2));
		  vhest_i3 = vec_add(vhest_i3, vec_abs(vsubc_i3));
		  vhest_i4 = vec_add(vhest_i4, vec_abs(vsubc_i4));

		  // transpose the matrix
		  // vseq_r, vsubc_r, vhest_r, vseq_i, vsubc_i, vhest_i
		  vx_h1 = vec_mergeh(vseq_r1, vsubc_r1);
		  vx_h2 = vec_mergeh(vhest_r1, vseq_i1);
		  vx_h3 = vec_mergeh(vsubc_i1, vhest_i1);
		  vx_l1 = vec_mergel(vseq_r1, vsubc_r1);
		  vx_l2 = vec_mergel(vhest_r1, vseq_i1);
		  vx_l3 = vec_mergel(vsubc_i1, vhest_i1);
		  // vx_h1, vx_h2, vx_h3
		  vsubc_r1 = vec_perm(vx_h1, vx_h2, pat1);
		  vseq_r1 = vec_perm(vx_h3, vx_h1, pat2);
		  vhest_r1 = vec_perm(vx_h2, vx_h3, pat3);

		  vsubc_i1 = vec_perm(vx_l1, vx_l2, pat1);
		  vseq_i1 = vec_perm(vx_l3, vx_l1, pat2);
		  vhest_i1 = vec_perm(vx_l2, vx_l3, pat3);

		  vec_st(vsubc_r1, 0, &softbit[6*k1]);
		  vec_st(vseq_r1, 0, &softbit[6*k1+4]);
		  vec_st(vhest_r1, 0, &softbit[6*k1+8]);
		  vec_st(vsubc_i1, 0, &softbit[6*k1+12]);
		  vec_st(vseq_i1, 0, &softbit[6*k1+16]);
		  vec_st(vhest_i1, 0, &softbit[6*k1+20]);

		  // vseq_r, vsubc_r, vhest_r, vseq_i, vsubc_i, vhest_i
		  vx_h1 = vec_mergeh(vseq_r2, vsubc_r2);
		  vx_h2 = vec_mergeh(vhest_r2, vseq_i2);
		  vx_h3 = vec_mergeh(vsubc_i2, vhest_i2);
		  vx_l1 = vec_mergel(vseq_r2, vsubc_r2);
		  vx_l2 = vec_mergel(vhest_r2, vseq_i2);
		  vx_l3 = vec_mergel(vsubc_i2, vhest_i2);
		  // vx_h1, vx_h2, vx_h3
		  vsubc_r2 = vec_perm(vx_h1, vx_h2, pat1);
		  vseq_r2 = vec_perm(vx_h3, vx_h1, pat2);
		  vhest_r2 = vec_perm(vx_h2, vx_h3, pat3);

		  vsubc_i2 = vec_perm(vx_l1, vx_l2, pat1);
		  vseq_i2 = vec_perm(vx_l3, vx_l1, pat2);
		  vhest_i2 = vec_perm(vx_l2, vx_l3, pat3);

		  vec_st(vsubc_r2, 0, &softbit[6*k2]);
		  vec_st(vseq_r2, 0, &softbit[6*k2+4]);
		  vec_st(vhest_r2, 0, &softbit[6*k2+8]);
		  vec_st(vsubc_i2, 0, &softbit[6*k2+12]);
		  vec_st(vseq_i2, 0, &softbit[6*k2+16]);
		  vec_st(vhest_i2, 0, &softbit[6*k2+20]);

		  // vseq_r, vsubc_r, vhest_r, vseq_i, vsubc_i, vhest_i
		  vx_h1 = vec_mergeh(vseq_r3, vsubc_r3);
		  vx_h2 = vec_mergeh(vhest_r3, vseq_i3);
		  vx_h3 = vec_mergeh(vsubc_i3, vhest_i3);
		  vx_l1 = vec_mergel(vseq_r3, vsubc_r3);
		  vx_l2 = vec_mergel(vhest_r3, vseq_i3);
		  vx_l3 = vec_mergel(vsubc_i3, vhest_i3);
		  // vx_h1, vx_h2, vx_h3
		  vsubc_r3 = vec_perm(vx_h1, vx_h2, pat1);
		  vseq_r3 = vec_perm(vx_h3, vx_h1, pat2);
		  vhest_r3 = vec_perm(vx_h2, vx_h3, pat3);

		  vsubc_i3 = vec_perm(vx_l1, vx_l2, pat1);
		  vseq_i3 = vec_perm(vx_l3, vx_l1, pat2);
		  vhest_i3 = vec_perm(vx_l2, vx_l3, pat3);

		  vec_st(vsubc_r3, 0, &softbit[6*k3]);
		  vec_st(vseq_r3, 0, &softbit[6*k3+4]);
		  vec_st(vhest_r3, 0, &softbit[6*k3+8]);
		  vec_st(vsubc_i3, 0, &softbit[6*k3+12]);
		  vec_st(vseq_i3, 0, &softbit[6*k3+16]);
		  vec_st(vhest_i3, 0, &softbit[6*k3+20]);

		  // vseq_r, vsubc_r, vhest_r, vseq_i, vsubc_i, vhest_i
		  vx_h1 = vec_mergeh(vseq_r4, vsubc_r4);
		  vx_h2 = vec_mergeh(vhest_r4, vseq_i4);
		  vx_h3 = vec_mergeh(vsubc_i4, vhest_i4);
		  vx_l1 = vec_mergel(vseq_r4, vsubc_r4);
		  vx_l2 = vec_mergel(vhest_r4, vseq_i4);
		  vx_l3 = vec_mergel(vsubc_i4, vhest_i4);
		  // vx_h1, vx_h2, vx_h3
		  vsubc_r4 = vec_perm(vx_h1, vx_h2, pat1);
		  vseq_r4 = vec_perm(vx_h3, vx_h1, pat2);
		  vhest_r4 = vec_perm(vx_h2, vx_h3, pat3);

		  vsubc_i4 = vec_perm(vx_l1, vx_l2, pat1);
		  vseq_i4 = vec_perm(vx_l3, vx_l1, pat2);
		  vhest_i4 = vec_perm(vx_l2, vx_l3, pat3);

		  vec_st(vsubc_r4, 0, &softbit[6*k4]);
		  vec_st(vseq_r4, 0, &softbit[6*k4+4]);
		  vec_st(vhest_r4, 0, &softbit[6*k4+8]);
		  vec_st(vsubc_i4, 0, &softbit[6*k4+12]);
		  vec_st(vseq_i4, 0, &softbit[6*k4+16]);
		  vec_st(vhest_i4, 0, &softbit[6*k4+20]);

		  k1 = k1 + 4;
		  k2 = k2 + 4;
		  k3 = k3 + 4;
		  k4 = k4 + 4;
	  }
	  // adjust index
	k1 = k4;
	k2 = k1 + step;
	k3 = k2 + step;
	k4 = k3 + step;
  }
}

//------------------------------------------------------------------------------------------
int32_t phy_ul_noise_est(const struct union_burst_ie *burstpara,
                         const float *pilot_data_r,
                         const float *pilot_data_i,
                         const float *pilot_est_r,
		         const float *pilot_est_i,
		         float *noise_est,
                         float *snr_est)
{
    int slot_idx;
    int slot_num;
    float noise_r;
    float noise_i;
    float signal_r;
    float signal_i;
    float noise_power;
    float signal_power;
    int i;

    slot_num = burstpara->slots_num;
    for(slot_idx = 0; slot_idx < slot_num; slot_idx++)
    {
        noise_power = 0;
        signal_power = 0;
        for (i = 0; i < PSUBCARRIER_NUM; i++)
        {
            noise_r = *(pilot_est_r + i) - *(pilot_data_r + i);
            noise_i = *(pilot_est_i + i) - *(pilot_data_i + i);
            signal_r = *(pilot_data_r + i);
            signal_i = *(pilot_data_i + i);
        
            noise_power += noise_r * noise_r + noise_i * noise_i;
            signal_power += signal_r * signal_r + signal_i * signal_i;
        }
        noise_power /= PSUBCARRIER_NUM;
         if (noise_power < 1e-7)
        {
            noise_power = 1.0F;
        }

        signal_power /= PSUBCARRIER_NUM;  

      
        noise_est[slot_idx] = noise_power;
        if (noise_power < 1e-7)
        {
            noise_power = 1.0F;
        }
        snr_est[slot_idx] = signal_power / noise_power;

        pilot_est_r += PSUBCARRIER_NUM;
        pilot_est_i += PSUBCARRIER_NUM;
        pilot_data_r += PSUBCARRIER_NUM;
        pilot_data_i += PSUBCARRIER_NUM;

    }
	
    return 0;
}							

int32_t phy_ul_demodulation(const struct union_burst_ie *burstpara,
                            const float *ant1_r,
                            const float *ant1_i,
                            const float *ant2_r,
                            const float *ant2_i,
                            const float *hest1_r,
                            const float *hest1_i,
                            const float *hest2_r,
                            const float *hest2_i,
                            float *noise_power1,
                            float *noise_power2,
                            float *softbit)

{
    const float *p_subcarrier_r;
    const float *p_subcarrier_i;
    const float *hest_r;
    const float *hest_i;
    // space that holds temporary softbit
    float *pool;

    int slot_idx;
    int slot_num;
    int softbit_num;
    int len;
//    static int max_num = 0;
	
    int i;
	
    // no check for release version
    //if(ant1_r == NULL || ant1_i == NULL)
    //{
    //    FLOG_ERROR("Input data for antenna 1 error in ul demodulation.\n");
    //    return ERROR_CODE;
    //}
    //    
    //if(ant2_r == NULL || ant2_i == NULL)
    //{
    //    FLOG_ERROR("Input data for antenna 2 error in ul demodulation.\n");
    //    return ERROR_CODE;
    //}

    //if(hest1_r == NULL || hest1_i == NULL)
    //{
    //    FLOG_ERROR("Input channel estimation for antenna 1 error in ul demodulation.\n");
    //    return ERROR_CODE;
    //}
    //    
    //if(hest2_r == NULL || hest2_i == NULL)
    //{
    //    FLOG_ERROR("Input channel estimation for antenna 2 error in ul demodulation.\n");
    //    return ERROR_CODE;
    //}

    //if(noise_power1 == NULL || noise_power2 == NULL)
    //{ 
    //    FLOG_ERROR("Input noise power is NULL.\n");
    //    return ERROR_CODE;
    //}
    //
    ////  FLOG_DEBUG("code _id = %d\n", burstpara->code_id);
    //    
    //if (burstpara->code_id > 6)
    //{
    //    FLOG_ERROR("The code type is not supported.\n");
    //    return ERROR_CODE;
    //}
    slot_num = burstpara->slots_num;
#if 0	
    if(max_num < slot_num){
	    if(max_num!=0) free(pool);
            
	    max_num = slot_num;
	    len = max_num*sizeof(float)*SOFTBIT_NUM_MAX;
	    //pool = (float *)malloc(len);
	    posix_memalign((void **)&pool, 16, len);
            //printf(" re-alloc pool %p \n", pool);
    }
#endif

/* added for mean N0 */
#ifdef _MEAN_NOISE_EST_ 
    float temp1 = 0;
    float temp2 = 0;

    for (i=0; i<slot_num; i++)
    {
        temp1 = temp1 + noise_power1[i];
        temp2 = temp2 + noise_power2[i];
    }

    temp1 = temp1 / slot_num;
    temp2 = temp2 / slot_num;
 // FLOG_DEBUG("noise_ant0 = %f, noise_ant1 = %f\n", temp1, temp2);

    for (i=0; i<slot_num; i++)
    {
        noise_power1[i] = temp1;
        noise_power2[i] = temp2;
    }

#endif
	
    switch (burstpara->code_id)
    {
        case CC_QPSK12: //QPSK
        case CC_QPSK34:
            softbit_num = DSUBCARRIER_NUM * 2;
            pool = softbit + softbit_num*slot_num; 
          //perform the demodulation for antenna #1
            phy_ul_demodulation_qpsk34(ant1_r, ant1_i, hest1_r, hest1_i, slot_num,
                noise_power1, softbit);
          //perform the demodulation for antenna #2
            phy_ul_demodulation_qpsk34(ant2_r, ant2_i, hest2_r, hest2_i, slot_num,
                noise_power2, pool);
            break;
        case CC_QAM16_12: //16-QAM
        case CC_QAM16_34:
            softbit_num = DSUBCARRIER_NUM * 4;
            pool = softbit + softbit_num*slot_num; 
            phy_ul_demodulation_qam16_34(ant1_r, ant1_i, hest1_r, hest1_i, slot_num,
                noise_power1, softbit);
            phy_ul_demodulation_qam16_34(ant2_r, ant2_i, hest2_r, hest2_i, slot_num,
                noise_power2, pool);
            break;
        case CC_QAM64_12: //64-QAM
        case CC_QAM64_23:
        case CC_QAM64_34:
            softbit_num = DSUBCARRIER_NUM * 6;
            pool = softbit + softbit_num*slot_num; 
            phy_ul_demodulation_qam64_34(ant1_r, ant1_i, hest1_r, hest1_i, slot_num,
                noise_power1, softbit);
            phy_ul_demodulation_qam64_34(ant2_r, ant2_i, hest2_r, hest2_i, slot_num,
                noise_power2, pool);
            break;
        default:
	    ;
            //softbit_num = DSUBCARRIER_NUM * 4;
    }
    // combine two antenna output
    vector_region_add(softbit, pool, slot_num*softbit_num);

    return 0;	
}

int32_t phy_ul_demodulation_single(const struct union_burst_ie *burstpara,
                                   const float *ant1_r,
                                   const float *ant1_i,
                                   const float *hest1_r,
                                   const float *hest1_i,
                                   const float *noise_power1,
                                   float *softbit)
{
   // float ant1_softbit[SOFTBIT_NUM_MAX];
    const float *p_subcarrier_r;
    const float *p_subcarrier_i;
    const float *hest_r;
    const float *hest_i;

    int slot_idx;
    int slot_num;
    int softbit_num;
	
    int i;
	
  // no check for release version
    //if(ant1_r == NULL || ant1_i == NULL)
    //{
    //    FLOG_ERROR("Input data for antenna 1 error in demodulation single.\n");
    //    return ERROR_CODE;
    //}
    //    
    //if(hest1_r == NULL || hest1_i == NULL)
    //{
    //    FLOG_ERROR("Input channel estimation for antenna 1 error in demodulation single.\n");
    //    return ERROR_CODE;
    //}
    //        
    //if(noise_power1 == NULL)
    //{ 
    //    FLOG_ERROR("Input noise power is NULL.\n");
    //    return ERROR_CODE;
    //}

    //    
    //if (burstpara->code_id > 6)
    //{
    //    FLOG_ERROR("The code type is not supported.\n");
    //    return ERROR_CODE;
    //}

    slot_num = burstpara->slots_num;
	
    switch (burstpara->code_id)
    {
        case CC_QPSK12: //QPSK
        case CC_QPSK34:
            //softbit_num = DSUBCARRIER_NUM * 2;
            phy_ul_demodulation_qpsk34(ant1_r, ant1_i, hest1_r, hest1_i, slot_num,
                noise_power1, softbit);
            break;
        case CC_QAM16_12: //16-QAM
        case CC_QAM16_34:
            //softbit_num = DSUBCARRIER_NUM * 4;
            phy_ul_demodulation_qam16_34(ant1_r, ant1_i, hest1_r, hest1_i, slot_num,
                noise_power1, softbit);
            break;
        case CC_QAM64_12: //64-QAM
        case CC_QAM64_23:
        case CC_QAM64_34:
            //softbit_num = DSUBCARRIER_NUM * 6;
            phy_ul_demodulation_qam64_34(ant1_r, ant1_i, hest1_r, hest1_i, slot_num,
                noise_power1, softbit);
            break;
        default:
	    ;
            //softbit_num = DSUBCARRIER_NUM * 4;
    }

    return 0;	
}
#else

#ifdef SSE2OPT

static int32_t phy_ul_demodulation_slot(const struct union_burst_ie *burstpara,
                                        const float *p_subcarrier_r,
                                        const float *p_subcarrier_i,
                                        const float *hest_r,
                                        const float *hest_i,
                                        float noise_power,
                                        float *p_softbit)
{		

	int i;

	__m128 vsymbol_r;
	__m128 vsymbol_i;
	__m128 vsymbol_eq_r;
	__m128 vsymbol_eq_i;

	__m128 vchan_r;
	__m128 vchan_i;
	__m128 vchan_coef;

	__m128 vscal_fact;
	__m128 vfactor;
	__m128 vtemp;
	__m128 vmid_r, vmid_r_fabs;
	__m128 vmid_i, vmid_i_fabs;

	__m128 vsymbol_eq_r_fabs;
	__m128 vsymbol_eq_i_fabs;
	__m128 vsoftbit1_r, vsoftbit2_r,vsoftbit3_r;
	__m128 vsoftbit1_i, vsoftbit2_i,vsoftbit3_i;
	__m128 vshuffle_tmp1, vshuffle_tmp2, vshuffle_tmp3;

	__m128 vnoise_power = _mm_set1_ps(noise_power);

	const float *vp_subcarrier_r;
	const float *vp_subcarrier_i;
	const float *vhest_r;
	const float *vhest_i;
	__m128 *vp_softbit;

	__m128 vlp_subcarrier_r[DSUBCARRIER_NUM/4];
	__m128 vlp_subcarrier_i[DSUBCARRIER_NUM/4];
	__m128 vlhest_r[DSUBCARRIER_NUM/4];
	__m128 vlhest_i[DSUBCARRIER_NUM/4];
	__m128 vlp_softbit[DSUBCARRIER_NUM/4 * 6];

	//check data alignment
	if( (((unsigned long)p_subcarrier_r) & 0xF) == 0)
	{
		vp_subcarrier_r = p_subcarrier_r;
	}
	else
	{
		memcpy(vlp_subcarrier_r, p_subcarrier_r, DSUBCARRIER_NUM * sizeof(float) );
		vp_subcarrier_r = (float *)vlp_subcarrier_r;
	}

	if( (((unsigned long)p_subcarrier_i) & 0xF) == 0)
	{
		vp_subcarrier_i = p_subcarrier_i;
	}
	else
	{
		memcpy(vlp_subcarrier_i, p_subcarrier_i, DSUBCARRIER_NUM * sizeof(float) );
		vp_subcarrier_i = (float *)vlp_subcarrier_i;
	}

	if(( ((unsigned long)hest_r) & 0xF ) == 0)
	{
		vhest_r = hest_r;
	}
	else
	{
		memcpy(vlhest_r, hest_r, DSUBCARRIER_NUM * sizeof(float) );
		vhest_r = (float *)vlhest_r;
	}

	if(( ((unsigned long)hest_i) & 0xF) == 0)
	{
		vhest_i = hest_i;
	}
	else
	{
		memcpy(vlhest_i, hest_i, DSUBCARRIER_NUM * sizeof(float) );
		vhest_i = (float *)vlhest_i;
	}

	if( (((unsigned long)p_softbit) & 0xF) == 0)
	{
		vp_softbit = (__m128 *)p_softbit;
	}
	else
	{
		vp_softbit = vlp_softbit;
	}


	//demodulation
	switch(burstpara->code_id)
	{
		case CC_QPSK12: //QPSK
		case CC_QPSK34:
			vscal_fact = _mm_set1_ps(sqrt(2));
			vfactor = _mm_div_ps(_mm_div_ps(_mm_set1_ps(4.0f), vscal_fact), vnoise_power);

			for (i = 0; i < DSUBCARRIER_NUM; i+=4)
			{
				vchan_r = _mm_load_ps(vhest_r + i);
				vchan_i = _mm_load_ps(vhest_i + i);
				vsymbol_r = _mm_load_ps(vp_subcarrier_r + i);
				vsymbol_i = _mm_load_ps(vp_subcarrier_i + i);
				vchan_coef = _mm_add_ps(_mm_mul_ps(vchan_r,vchan_r), _mm_mul_ps(vchan_i,vchan_i));
				//equalization, symbol * conj(h)
				vsymbol_eq_r = _mm_add_ps(_mm_mul_ps(vsymbol_r, vchan_r), _mm_mul_ps( vsymbol_i, vchan_i));
				vsymbol_eq_i = _mm_sub_ps(_mm_mul_ps(vsymbol_i, vchan_r), _mm_mul_ps( vsymbol_r, vchan_i));

				vsoftbit1_r = _mm_mul_ps(_mm_sub_ps(_mm_setzero_ps(), vsymbol_eq_r), vfactor);
				vsoftbit1_i= _mm_mul_ps(_mm_sub_ps(_mm_setzero_ps(), vsymbol_eq_i), vfactor);

				vshuffle_tmp1  = _mm_shuffle_ps(vsoftbit1_r, vsoftbit1_i, 0x44);
				_mm_store_ps((float *)(vp_softbit++), _mm_shuffle_ps(vshuffle_tmp1, vshuffle_tmp1, 0xD8)); 

				vshuffle_tmp1  = _mm_shuffle_ps(vsoftbit1_r, vsoftbit1_i, 0xEE);
				_mm_store_ps((float *)(vp_softbit++), _mm_shuffle_ps(vshuffle_tmp1, vshuffle_tmp1, 0xD8)); 

			}
			if( (((unsigned long)p_softbit) & 0xF) != 0)
			{
				memcpy(p_softbit, vlp_softbit, DSUBCARRIER_NUM * 2 * sizeof(float) );
			}
			break;

		case CC_QAM16_12: //16-QAM
		case CC_QAM16_34:
			vscal_fact = _mm_set1_ps(sqrt(10));
			vfactor = _mm_div_ps(_mm_div_ps(_mm_set1_ps(4.0f), vscal_fact), vnoise_power);

			for (i = 0; i < DSUBCARRIER_NUM; i+=4)
			{
				vchan_r = _mm_load_ps(vhest_r + i);
				vchan_i = _mm_load_ps(vhest_i + i);
				vsymbol_r = _mm_load_ps(vp_subcarrier_r + i);
				vsymbol_i = _mm_load_ps(vp_subcarrier_i + i);
				vchan_coef = _mm_add_ps(_mm_mul_ps(vchan_r,vchan_r), _mm_mul_ps(vchan_i,vchan_i));
				//equalization, symbol * conj(h)
				vsymbol_eq_r = _mm_add_ps(_mm_mul_ps(vsymbol_r, vchan_r), _mm_mul_ps( vsymbol_i, vchan_i));
				vsymbol_eq_i = _mm_sub_ps(_mm_mul_ps(vsymbol_i, vchan_r), _mm_mul_ps( vsymbol_r, vchan_i));

				vsymbol_eq_r_fabs = _mm_max_ps(_mm_sub_ps(_mm_setzero_ps(), vsymbol_eq_r), vsymbol_eq_r);
				vsymbol_eq_i_fabs = _mm_max_ps(_mm_sub_ps(_mm_setzero_ps(), vsymbol_eq_i), vsymbol_eq_i);

				vtemp = _mm_mul_ps(_mm_div_ps(_mm_set1_ps(2.0f), vscal_fact), vchan_coef);

				vsoftbit1_r = _mm_mul_ps(_mm_sub_ps(_mm_setzero_ps(), vsymbol_eq_r), vfactor);
				vsoftbit2_r = _mm_mul_ps(_mm_sub_ps(vsymbol_eq_r_fabs, vtemp), vfactor);
				vsoftbit1_i= _mm_mul_ps(_mm_sub_ps(_mm_setzero_ps(), vsymbol_eq_i), vfactor);
				vsoftbit2_i = _mm_mul_ps(_mm_sub_ps(vsymbol_eq_i_fabs, vtemp), vfactor);

				vshuffle_tmp1  = _mm_shuffle_ps(vsoftbit1_r, vsoftbit2_r, 0x44);
				vshuffle_tmp2  = _mm_shuffle_ps(vsoftbit1_i, vsoftbit2_i, 0x44);
				_mm_store_ps((float *)(vp_softbit++), _mm_shuffle_ps(vshuffle_tmp1, vshuffle_tmp2, 0x88)); 
				_mm_store_ps((float *)(vp_softbit++), _mm_shuffle_ps(vshuffle_tmp1, vshuffle_tmp2, 0xDD)); 

				vshuffle_tmp1  = _mm_shuffle_ps(vsoftbit1_r, vsoftbit2_r, 0xEE);
				vshuffle_tmp2  = _mm_shuffle_ps(vsoftbit1_i, vsoftbit2_i, 0xEE);
				_mm_store_ps((float *)(vp_softbit++), _mm_shuffle_ps(vshuffle_tmp1, vshuffle_tmp2, 0x88)); 
				_mm_store_ps((float *)(vp_softbit++), _mm_shuffle_ps(vshuffle_tmp1, vshuffle_tmp2, 0xDD)); 


			}            
			if( (((unsigned long)p_softbit) & 0xF) != 0)
			{
				memcpy(p_softbit, vlp_softbit, DSUBCARRIER_NUM * 4 * sizeof(float) );
			}
			break;

		case CC_QAM64_12: //64-QAM
		case CC_QAM64_23:
		case CC_QAM64_34:
			vscal_fact = _mm_set1_ps(sqrt(42));
			vfactor = _mm_div_ps(_mm_div_ps(_mm_set1_ps(4.0f), vscal_fact), vnoise_power);

			for (i = 0; i < DSUBCARRIER_NUM; i+=4)
			{
				vchan_r = _mm_load_ps(vhest_r + i);
				vchan_i = _mm_load_ps(vhest_i + i);
				vsymbol_r = _mm_load_ps(vp_subcarrier_r + i);
				vsymbol_i = _mm_load_ps(vp_subcarrier_i + i);
				vchan_coef = _mm_add_ps(_mm_mul_ps(vchan_r,vchan_r), _mm_mul_ps(vchan_i,vchan_i));
				//equalization, symbol * conj(h)
				vsymbol_eq_r = _mm_add_ps(_mm_mul_ps(vsymbol_r, vchan_r), _mm_mul_ps( vsymbol_i, vchan_i));
				vsymbol_eq_i = _mm_sub_ps(_mm_mul_ps(vsymbol_i, vchan_r), _mm_mul_ps( vsymbol_r, vchan_i));

				vsymbol_eq_r_fabs = _mm_max_ps(_mm_sub_ps(_mm_setzero_ps(), vsymbol_eq_r), vsymbol_eq_r);
				vsymbol_eq_i_fabs = _mm_max_ps(_mm_sub_ps(_mm_setzero_ps(), vsymbol_eq_i), vsymbol_eq_i);

				vtemp = _mm_mul_ps(_mm_div_ps(_mm_set1_ps(2.0f), vscal_fact), vchan_coef);

				vmid_r =  _mm_sub_ps(vsymbol_eq_r_fabs, _mm_mul_ps(_mm_set1_ps(2.0f), vtemp));
				vmid_i =  _mm_sub_ps(vsymbol_eq_i_fabs, _mm_mul_ps(_mm_set1_ps(2.0f), vtemp));
				vmid_r_fabs = _mm_max_ps(_mm_sub_ps(_mm_setzero_ps(), vmid_r), vmid_r);
				vmid_i_fabs = _mm_max_ps(_mm_sub_ps(_mm_setzero_ps(), vmid_i), vmid_i);


				vsoftbit1_r = _mm_mul_ps(_mm_sub_ps(_mm_setzero_ps(), vsymbol_eq_r), vfactor);
				vsoftbit2_r = _mm_mul_ps(vmid_r, vfactor);
				vsoftbit3_r = _mm_mul_ps(_mm_sub_ps(vmid_r_fabs, vtemp), vfactor);
				vsoftbit1_i = _mm_mul_ps(_mm_sub_ps(_mm_setzero_ps(), vsymbol_eq_i), vfactor);
				vsoftbit2_i = _mm_mul_ps(vmid_i, vfactor);
				vsoftbit3_i = _mm_mul_ps(_mm_sub_ps(vmid_i_fabs, vtemp), vfactor);

				vshuffle_tmp1  = _mm_shuffle_ps(vsoftbit1_r, vsoftbit2_r, 0x44);
				vshuffle_tmp2  = _mm_shuffle_ps(vsoftbit3_r, vsoftbit1_i, 0x44);
				vshuffle_tmp3  = _mm_shuffle_ps(vsoftbit2_i, vsoftbit3_i, 0x44);
				_mm_store_ps((float *)(vp_softbit++), _mm_shuffle_ps(vshuffle_tmp1, vshuffle_tmp2, 0x88)); 
				_mm_store_ps((float *)(vp_softbit++), _mm_shuffle_ps(vshuffle_tmp3, vshuffle_tmp1, 0xD8)); 
				_mm_store_ps((float *)(vp_softbit++), _mm_shuffle_ps(vshuffle_tmp2, vshuffle_tmp3, 0xDD)); 

				vshuffle_tmp1  = _mm_shuffle_ps(vsoftbit1_r, vsoftbit2_r, 0xEE);
				vshuffle_tmp2  = _mm_shuffle_ps(vsoftbit3_r, vsoftbit1_i, 0xEE);
				vshuffle_tmp3  = _mm_shuffle_ps(vsoftbit2_i, vsoftbit3_i, 0xEE);
				_mm_store_ps((float *)(vp_softbit++), _mm_shuffle_ps(vshuffle_tmp1, vshuffle_tmp2, 0x88)); 
				_mm_store_ps((float *)(vp_softbit++), _mm_shuffle_ps(vshuffle_tmp3, vshuffle_tmp1, 0xD8)); 
				_mm_store_ps((float *)(vp_softbit++), _mm_shuffle_ps(vshuffle_tmp2, vshuffle_tmp3, 0xDD)); 


			}            

			if( (((unsigned long)p_softbit) & 0xF) != 0)
			{
				memcpy(p_softbit, vlp_softbit, DSUBCARRIER_NUM * 6 * sizeof(float) );
			}
			break;

		default:
			FLOG_INFO("This code type is not supported.\n");
			break;						
	}


	return 0;
}



#else 

static int32_t phy_ul_demodulation_slot(const struct union_burst_ie *burstpara,
                                        const float *p_subcarrier_r,
                                        const float *p_subcarrier_i,
                                        const float *hest_r,
                                        const float *hest_i,
                                        float noise_power,
                                        float *p_softbit)
{		
    float symbol_r;
    float symbol_i;
    float symbol_eq_r;
    float symbol_eq_i;
	
    float chan_r;
    float chan_i;
    float chan_coef;
	
    float scal_fact;
    float factor;
    float temp;
    int i;

    //demodulation
    switch(burstpara->code_id)
    {
        case CC_QPSK12: //QPSK
        case CC_QPSK34:
            scal_fact = sqrt(2);
            factor = 4 / scal_fact / noise_power;
			
            for (i = 0; i < DSUBCARRIER_NUM; i++)
            {
                chan_r = *(hest_r + i);
                chan_i = *(hest_i + i);
                symbol_r = *(p_subcarrier_r + i);
                symbol_i = *(p_subcarrier_i + i);
                chan_coef = chan_r * chan_r + chan_i * chan_i;
                //equalization, symbol * conj(h)
                symbol_eq_r = symbol_r * chan_r + symbol_i * chan_i;
                symbol_eq_i = symbol_i * chan_r - symbol_r * chan_i;
		
                *p_softbit++ = -symbol_eq_r * factor;
                *p_softbit++ = -symbol_eq_i * factor;				
				
            }
            break;
			
        case CC_QAM16_12: //16-QAM
        case CC_QAM16_34:
            scal_fact = sqrt(10);
            factor = 4 / scal_fact /noise_power;
            for (i = 0; i < DSUBCARRIER_NUM; i++)
            {
                chan_r = *(hest_r + i);
                chan_i = *(hest_i + i);
                symbol_r = *(p_subcarrier_r + i);
                symbol_i = *(p_subcarrier_i + i);
                chan_coef = chan_r * chan_r + chan_i * chan_i;
                //equalization, symbol * conj(h)
                symbol_eq_r = symbol_r * chan_r + symbol_i * chan_i;
                symbol_eq_i = symbol_i * chan_r - symbol_r * chan_i;
				
                temp = 2 / scal_fact * chan_coef;
                *p_softbit++ = - symbol_eq_r * factor;
                *p_softbit++ = (fabs(symbol_eq_r) - temp) * factor;
                *p_softbit++ = -symbol_eq_i * factor;
                *p_softbit++ = (fabs(symbol_eq_i) - temp) * factor;			
            }
            break;
			
        case CC_QAM64_12: //64-QAM
        case CC_QAM64_23:
        case CC_QAM64_34:
            scal_fact = sqrt(42);
            factor = 4 / scal_fact /noise_power;
            for (i = 0; i < DSUBCARRIER_NUM; i++)
            {
                chan_r = *(hest_r + i);
                chan_i = *(hest_i + i);
                symbol_r = *(p_subcarrier_r + i);
                symbol_i = *(p_subcarrier_i + i);
                chan_coef = chan_r * chan_r + chan_i * chan_i;
                //equalization, symbol * conj(h)
                symbol_eq_r = symbol_r * chan_r + symbol_i * chan_i;
                symbol_eq_i = symbol_i * chan_r - symbol_r * chan_i;
				
                temp = 2 / scal_fact * chan_coef;
                *p_softbit++ = - symbol_eq_r * factor;
                *p_softbit++ = (fabs(symbol_eq_r) - 2 * temp) * factor;
                *p_softbit++ = (fabs(fabs(symbol_eq_r) - 2 * temp) - temp ) * factor;
                *p_softbit++ = - symbol_eq_i * factor;
                *p_softbit++ = (fabs(symbol_eq_i) - 2 * temp) * factor;
                *p_softbit++ = (fabs(fabs(symbol_eq_i) - 2 * temp) - temp ) * factor;		
            }
            break;
			
        default:
            FLOG_INFO("This code type is not supported.\n");
            break;						
    }

    return 0;
}

#endif

//------------------------------------------------------------------------------------------
int32_t phy_ul_noise_est(const struct union_burst_ie *burstpara,
                         const float *pilot_data_r,
                         const float *pilot_data_i,
                         const float *pilot_est_r,
		         const float *pilot_est_i,
		         float *noise_est,
                         float *snr_est)
{
    int slot_idx;
    int slot_num;
    float noise_r;
    float noise_i;
    float signal_r;
    float signal_i;
    float noise_power;
    float signal_power;
    int i;

    slot_num = burstpara->slots_num;
    for(slot_idx = 0; slot_idx < slot_num; slot_idx++)
    {
        noise_power = 0;
        signal_power = 0;
        for (i = 0; i < PSUBCARRIER_NUM; i++)
        {
            noise_r = *(pilot_est_r + i) - *(pilot_data_r + i);
            noise_i = *(pilot_est_i + i) - *(pilot_data_i + i);
            signal_r = *(pilot_data_r + i);
            signal_i = *(pilot_data_i + i);
        
            noise_power += noise_r * noise_r + noise_i * noise_i;
            signal_power += signal_r * signal_r + signal_i * signal_i;
        }
        noise_power /= PSUBCARRIER_NUM;
         if (noise_power < 1e-7)
        {
            noise_power = 1.0F;
        }

        signal_power /= PSUBCARRIER_NUM;  

      
        noise_est[slot_idx] = noise_power;
        if (noise_power < 1e-7)
        {
            noise_power = 1.0F;
        }
        snr_est[slot_idx] = signal_power / noise_power;

        pilot_est_r += PSUBCARRIER_NUM;
        pilot_est_i += PSUBCARRIER_NUM;
        pilot_data_r += PSUBCARRIER_NUM;
        pilot_data_i += PSUBCARRIER_NUM;

    }
	
    return 0;
}							
int32_t phy_ul_demodulation(const struct union_burst_ie *burstpara,
                            const float *ant1_r,
                            const float *ant1_i,
                            const float *ant2_r,
                            const float *ant2_i,
                            const float *hest1_r,
                            const float *hest1_i,
                            const float *hest2_r,
                            const float *hest2_i,
                            float *noise_power1,
                            float *noise_power2,
                            float *softbit)

{
    float ant1_softbit[SOFTBIT_NUM_MAX];
    float ant2_softbit[SOFTBIT_NUM_MAX];
    const float *p_subcarrier_r;
    const float *p_subcarrier_i;
    const float *hest_r;
    const float *hest_i;

    int slot_idx;
    int slot_num;
    int softbit_num;
	
    int i;
	
    if(ant1_r == NULL || ant1_i == NULL)
    {
        FLOG_ERROR("Input data for antenna 1 error in ul demodulation.\n");
        return ERROR_CODE;
    }
	
    if(ant2_r == NULL || ant2_i == NULL)
    {
        FLOG_ERROR("Input data for antenna 2 error in ul demodulation.\n");
        return ERROR_CODE;
    }

    if(hest1_r == NULL || hest1_i == NULL)
    {
        FLOG_ERROR("Input channel estimation for antenna 1 error in ul demodulation.\n");
        return ERROR_CODE;
    }
	
    if(hest2_r == NULL || hest2_i == NULL)
    {
        FLOG_ERROR("Input channel estimation for antenna 2 error in ul demodulation.\n");
        return ERROR_CODE;
    }

    if(noise_power1 == NULL || noise_power2 == NULL)
    { 
        FLOG_ERROR("Input noise power is NULL.\n");
        return ERROR_CODE;
    }
    
  //  FLOG_DEBUG("code _id = %d\n", burstpara->code_id);
	
    if (burstpara->code_id > 6)
    {
        FLOG_ERROR("The code type is not supported.\n");
        return ERROR_CODE;
    }

	
    switch (burstpara->code_id)
    {
        case CC_QPSK12: //QPSK
        case CC_QPSK34:
            softbit_num = DSUBCARRIER_NUM * 2;
            break;
        case CC_QAM16_12: //16-QAM
        case CC_QAM16_34:
            softbit_num = DSUBCARRIER_NUM * 4;
            break;
        case CC_QAM64_12: //64-QAM
        case CC_QAM64_23:
        case CC_QAM64_34:
            softbit_num = DSUBCARRIER_NUM * 6;
            break;
        default:
            softbit_num = DSUBCARRIER_NUM * 4;
    }
	
    slot_num = burstpara->slots_num;
/* added for mean N0 */
#ifdef _MEAN_NOISE_EST_ 
    float temp1 = 0;
    float temp2 = 0;

    for (i=0; i<slot_num; i++)
    {
        temp1 = temp1 + noise_power1[i];
        temp2 = temp2 + noise_power2[i];
    }

    temp1 = temp1 / slot_num;
    temp2 = temp2 / slot_num;
 // FLOG_DEBUG("noise_ant0 = %f, noise_ant1 = %f\n", temp1, temp2);

    for (i=0; i<slot_num; i++)
    {
        noise_power1[i] = temp1;
        noise_power2[i] = temp2;
    }

#endif

    for(slot_idx = 0; slot_idx < slot_num; slot_idx++)
    {
        //perform the demodulation slot by slot
        //demodulation for antenna #1
        p_subcarrier_r = ant1_r + DSUBCARRIER_NUM * slot_idx; 
        p_subcarrier_i = ant1_i + DSUBCARRIER_NUM * slot_idx; 
        hest_r = hest1_r + DSUBCARRIER_NUM * slot_idx; 
        hest_i = hest1_i + DSUBCARRIER_NUM * slot_idx;
        phy_ul_demodulation_slot(burstpara, p_subcarrier_r, p_subcarrier_i, 
                                 hest_r, hest_i, noise_power1[slot_idx], ant1_softbit);

        //demodulation for antenna #2
        p_subcarrier_r = ant2_r + DSUBCARRIER_NUM * slot_idx; 
        p_subcarrier_i = ant2_i + DSUBCARRIER_NUM * slot_idx; 
        hest_r = hest2_r + DSUBCARRIER_NUM * slot_idx; 
        hest_i = hest2_i + DSUBCARRIER_NUM * slot_idx;
        phy_ul_demodulation_slot(burstpara, p_subcarrier_r, p_subcarrier_i,
                                 hest_r, hest_i, noise_power2[slot_idx], ant2_softbit);
        //MRC
        for (i = 0; i < softbit_num; i++)
        {
            *softbit++ = ant1_softbit[i] + ant2_softbit[i];
        }
	
    }	

    return 0;	
}

int32_t phy_ul_demodulation_single(const struct union_burst_ie *burstpara,
                                   const float *ant1_r,
                                   const float *ant1_i,
                                   const float *hest1_r,
                                   const float *hest1_i,
                                   const float *noise_power1,
                                   float *softbit)
{
   // float ant1_softbit[SOFTBIT_NUM_MAX];
    const float *p_subcarrier_r;
    const float *p_subcarrier_i;
    const float *hest_r;
    const float *hest_i;

    int slot_idx;
    int slot_num;
    int softbit_num;
	
  //  int i;
	
    if(ant1_r == NULL || ant1_i == NULL)
    {
        FLOG_ERROR("Input data for antenna 1 error in demodulation single.\n");
        return ERROR_CODE;
    }
	
    if(hest1_r == NULL || hest1_i == NULL)
    {
        FLOG_ERROR("Input channel estimation for antenna 1 error in demodulation single.\n");
        return ERROR_CODE;
    }
	    
    if(noise_power1 == NULL)
    { 
        FLOG_ERROR("Input noise power is NULL.\n");
        return ERROR_CODE;
    }

	
    if (burstpara->code_id > 6)
    {
        FLOG_ERROR("The code type is not supported.\n");
        return ERROR_CODE;
    }


	
    switch (burstpara->code_id)
    {
        case CC_QPSK12: //QPSK
        case CC_QPSK34:
            softbit_num = DSUBCARRIER_NUM * 2;
            break;
        case CC_QAM16_12: //16-QAM
        case CC_QAM16_34:
            softbit_num = DSUBCARRIER_NUM * 4;
            break;
        case CC_QAM64_12: //64-QAM
        case CC_QAM64_23:
        case CC_QAM64_34:
            softbit_num = DSUBCARRIER_NUM * 6;
            break;
        default:
            softbit_num = DSUBCARRIER_NUM * 4;
    }
	
    slot_num = burstpara->slots_num;
	
    for(slot_idx = 0; slot_idx < slot_num; slot_idx++)
    {
        //perform the demodulation slot by slot
        //demodulation for antenna #1
        p_subcarrier_r = ant1_r + DSUBCARRIER_NUM * slot_idx; 
        p_subcarrier_i = ant1_i + DSUBCARRIER_NUM * slot_idx; 
        hest_r = hest1_r + DSUBCARRIER_NUM * slot_idx; 
        hest_i = hest1_i + DSUBCARRIER_NUM * slot_idx;
        phy_ul_demodulation_slot(burstpara, p_subcarrier_r, p_subcarrier_i, 
                                 hest_r, hest_i, noise_power1[slot_idx], softbit+softbit_num*slot_idx);

    }	


    return 0;	
}
#endif
