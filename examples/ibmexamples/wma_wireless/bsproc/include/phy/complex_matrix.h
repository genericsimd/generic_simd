/* ----------------------------------------------------------------------------
   IBM Confidential

   IBM Wireless M2M Platform

   (C)Copyright IBM Corp. 2009, 2010, 2011

   All Rights Reserved.

   File Name: 

   Function: 

      ----------------------------------------------------------------------------
   PROLOG END TAG zYx                                                         */


#ifndef __COMPLEX_MATRIX_H__

#define __COMPLEX_MATRIX_H__

float complex_matrix_add(float *matrix1_real,

                         float *matrix1_imag,

                         float *matrix2_real,

                         float *matrix2_imag,

                         float *result_real,

                         float *result_imag);



float complex_matrix_conj(float *matrix_real,

                          float *matrix_imag,

                          float *result_real,

                          float *result_imag);



float complex_matrix_mul22(float *matrix1_real,

                           float *matrix1_imag,

                           float *matrix2_real,

                           float *matrix2_imag,

                           float *result_real,

                           float *result_imag);



float complex_matrix_mul21(float *matrix1_real,

                           float *matrix1_imag,

                           float *matrix2_real,

                           float *matrix2_imag,

                           float *result_real,

                           float *result_imag);

#endif





