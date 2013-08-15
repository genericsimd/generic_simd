#include <stdio.h>
#include "complex_math.h"

float complex_mul(float complex1_real,
                  float complex1_imag,
                  float complex2_real,
                  float complex2_imag,
                  float *result)
{
    float result_real,result_imag;
    result_real = complex1_real*complex2_real-complex1_imag*complex2_imag;
    result_imag = complex1_real*complex2_imag+complex1_imag*complex2_real;
    result[0] = result_real;
    result[1] = result_imag;

    return 0;
}


float complex_add(float complex1_real,
                  float complex1_imag,
                  float complex2_real,
                  float complex2_imag,
                  float *result)
{
    float result_real,result_imag;
    result_real = complex1_real+complex2_real;
    result_imag = complex1_imag+complex2_imag;
    result[0] = result_real;
    result[1] = result_imag;

    return 0;
}


float complex_sub(float complex1_real,
                  float complex1_imag,
                  float complex2_real,
                  float complex2_imag,
                  float *result)
{
    float result_real,result_imag;
    result_real = complex1_real-complex2_real;
    result_imag = complex1_imag-complex2_imag;
    result[0] = result_real;
    result[1] = result_imag;

    return 0;
}


float complex_div(float complex1_real,
                  float complex1_imag,
                  float complex2_real,
                  float complex2_imag,
                  float *result)
{
    float temp;
    float result_real,result_imag;
    temp = complex2_real*complex2_real+complex2_imag*complex2_imag;
    result_real = (complex1_real*complex2_real+complex1_imag*complex2_imag)/temp;
    result_imag = (complex1_imag*complex2_real-complex1_real*complex2_imag)/temp;
    result[0] = result_real;
    result[1] = result_imag;

    return 0;
}

