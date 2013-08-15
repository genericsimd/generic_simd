/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */

#ifndef __COMPLEX_MATH_H__
#define __COMPLEX_MATH_H__
float complex_mul(float complex1_real,
                  float complex1_imag,
                  float complex2_real,
                  float complex2_imag,
                  float *result);

float complex_add(float complex1_real,
                  float complex1_imag,
                  float complex2_real,
                  float complex2_imag,
                  float *result);

float complex_sub(float complex1_real,
                  float complex1_imag,
                  float complex2_real,
                  float complex2_imag,
                  float *result);

float complex_div(float complex1_real,
                  float complex1_imag,
                  float complex2_real,
                  float complex2_imag,
                  float *result);
#endif

