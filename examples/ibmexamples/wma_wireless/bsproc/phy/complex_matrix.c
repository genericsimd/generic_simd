#include <stdio.h>
#include "complex_matrix.h"
#include "complex_math.h"

float complex_matrix_add(float *matrix1_real,
                         float *matrix1_imag,
                         float *matrix2_real,
                         float *matrix2_imag,
                         float *result_real,
                         float *result_imag)
{
    int i;
    for (i=0;i<4;i++){
        result_real[i] = matrix1_real[i]+matrix2_real[i];
        result_imag[i] = matrix1_imag[i]+matrix2_imag[i];
    }

    return 0;
}


/* float complex_matrix_conj(float *matrix_real,
                          float *matrix_imag,
                          float *result_real,
                          float *result_imag)
{
    int i;
    for (i=0;i<4;i++){
        result_real[i] = matrix_real[i];
        result_imag[i] = -matrix_imag[i];
    }

    return 0;
} */

float complex_matrix_conj(float *matrix_real,
                          float *matrix_imag,
                          float *result_real,
                          float *result_imag)
{
    
    result_real[0] = matrix_real[0];
    result_imag[0] = -matrix_imag[0];
    
	result_real[1] = matrix_real[2];
    result_imag[1] = -matrix_imag[2];
	
	result_real[2] = matrix_real[1];
    result_imag[2] = -matrix_imag[1];
	
	result_real[3] = matrix_real[3];
    result_imag[3] = -matrix_imag[3];

    return 0;
}



float complex_matrix_mul22(float *matrix1_real,
                           float *matrix1_imag,
                           float *matrix2_real,
                           float *matrix2_imag,
                           float *result_real,
                           float *result_imag)
{
    float temp1[2], temp2[2];
    float temp1_r, temp1_i, temp2_r, temp2_i;

    complex_mul(matrix1_real[0],matrix1_imag[0], matrix2_real[0], matrix2_imag[0], temp1);
    complex_mul(matrix1_real[1],matrix1_imag[1], matrix2_real[2], matrix2_imag[2], temp2);
    temp1_r = temp1[0];
    temp1_i = temp1[1];
    temp2_r = temp2[0];
    temp2_i = temp2[1];
    result_real[0] = temp1_r + temp2_r;
    result_imag[0] = temp1_i + temp2_i;

    complex_mul(matrix1_real[0],matrix1_imag[0], matrix2_real[1], matrix2_imag[1], temp1);
    complex_mul(matrix1_real[1],matrix1_imag[1], matrix2_real[3], matrix2_imag[3], temp2);
    temp1_r = temp1[0];
    temp1_i = temp1[1];
    temp2_r = temp2[0];
    temp2_i = temp2[1];
    result_real[1] = temp1_r + temp2_r;
    result_imag[1] = temp1_i + temp2_i;

    complex_mul(matrix1_real[2],matrix1_imag[2], matrix2_real[0], matrix2_imag[0], temp1);
    complex_mul(matrix1_real[3],matrix1_imag[3], matrix2_real[2], matrix2_imag[2], temp2);
    temp1_r = temp1[0];
    temp1_i = temp1[1];
    temp2_r = temp2[0];
    temp2_i = temp2[1];
    result_real[2] = temp1_r + temp2_r;
    result_imag[2] = temp1_i + temp2_i;

    complex_mul(matrix1_real[2],matrix1_imag[2], matrix2_real[1], matrix2_imag[1], temp1);
    complex_mul(matrix1_real[3],matrix1_imag[3], matrix2_real[3], matrix2_imag[3], temp2);
    temp1_r = temp1[0];
    temp1_i = temp1[1];
    temp2_r = temp2[0];
    temp2_i = temp2[1];
    result_real[3] = temp1_r + temp2_r;
    result_imag[3] = temp1_i + temp2_i;

    return 0;
}


float complex_matrix_mul21(float *matrix1_real,
                           float *matrix1_imag,
                           float *matrix2_real,
                           float *matrix2_imag,
                           float *result_real,
                           float *result_imag)
{
    float temp1[2], temp2[2];
    float temp1_r, temp1_i, temp2_r, temp2_i;

    complex_mul(matrix1_real[0],matrix1_imag[0], matrix2_real[0], matrix2_imag[0], temp1);
    complex_mul(matrix1_real[1],matrix1_imag[1], matrix2_real[1], matrix2_imag[1], temp2);
    temp1_r = temp1[0];
    temp1_i = temp1[1];
    temp2_r = temp2[0];
    temp2_i = temp2[1];
    result_real[0] = temp1_r + temp2_r;
    result_imag[0] = temp1_i + temp2_i;

    complex_mul(matrix1_real[2],matrix1_imag[2], matrix2_real[0], matrix2_imag[0], temp1);
    complex_mul(matrix1_real[3],matrix1_imag[3], matrix2_real[1], matrix2_imag[1], temp2);
    temp1_r = temp1[0];
    temp1_i = temp1[1];
    temp2_r = temp2[0];
    temp2_i = temp2[1];
    result_real[1] = temp1_r + temp2_r;
    result_imag[1] = temp1_i + temp2_i;

    return 0;
}


