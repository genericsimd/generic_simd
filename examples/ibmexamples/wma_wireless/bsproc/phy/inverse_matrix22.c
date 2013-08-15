
#include <stdio.h>
#include "inverse_matrix22.h"
#include "complex_math.h"

float inverse_matrix22 (float *matrixa_real, float *matrixa_imag,
                        float *inv_matrixa_real, float *inv_matrixa_imag)
{
    float x11_r, x12_r, x21_r, x22_r;
    float x11_i, x12_i, x21_i, x22_i;


    float temp1[2];
    float temp2[2];
    float H[2];
    float inv_matrixa0[2];
    float inv_matrixa1[2];
    float inv_matrixa2[2];
    float inv_matrixa3[2];

    float temp1_r, temp1_i;
    float temp2_r, temp2_i;
    float H_r, H_i;

    x11_r = matrixa_real[0];
    x12_r = matrixa_real[1];
    x21_r = matrixa_real[2];
    x22_r = matrixa_real[3];

    x11_i = matrixa_imag[0];
    x12_i = matrixa_imag[1];
    x21_i = matrixa_imag[2];
    x22_i = matrixa_imag[3];

    // H = x11* x22-x12*x21;
    complex_mul(x11_r,x11_i,x22_r,x22_i, temp1);
    complex_mul(x12_r,x12_i,x21_r,x21_i, temp2);

    temp1_r = temp1[0];
    temp1_i = temp1[1];
    temp2_r = temp2[0];
    temp2_i = temp2[1];

    complex_sub(temp1_r, temp1_i, temp2_r, temp2_i, H);

    H_r = H[0];
    H_i = H[1];

/*
    inv_matrixa[0] = x22/H;
    inv_matrixa[1] = -x12/H;
    inv_matrixa[2] = -x21/H;
    inv_matrixa[3] = x11/H;
*/
    complex_div(x22_r, x22_i, H_r, H_i, inv_matrixa0);
    complex_div(-x12_r, -x12_i, H_r, H_i, inv_matrixa1);
    complex_div(-x21_r, -x21_i, H_r, H_i, inv_matrixa2);
    complex_div(x11_r, x11_i, H_r, H_i, inv_matrixa3);

    inv_matrixa_real[0] = inv_matrixa0[0];
    inv_matrixa_real[1] = inv_matrixa1[0];
    inv_matrixa_real[2] = inv_matrixa2[0];
    inv_matrixa_real[3] = inv_matrixa3[0];

    inv_matrixa_imag[0] = inv_matrixa0[1];
    inv_matrixa_imag[1] = inv_matrixa1[1];
    inv_matrixa_imag[2] = inv_matrixa2[1];
    inv_matrixa_imag[3] = inv_matrixa3[1];

    return 0;
}

/*
float inverse_matrix22 (float *matrixa, float *inv_matrixa)
{
    float x11, x12, x21, x22;
    float H;

    x11 = matrixa[0];
    x12 = matrixa[1];
    x21 = matrixa[2];
    x22 = matrixa[3];

    H = x11* x22-x12*x21;

    inv_matrixa[0] = x22/H;
    inv_matrixa[1] = -x12/H;
    inv_matrixa[2] = -x21/H;
    inv_matrixa[3] = x11/H;

    return 0;
}
*/

