/*
 * SimpleTest.cpp
 *
 *  Created on: Jun 12, 2013
 *      Author: haichuan
 */


/*
 *  g++ -DLANES=4 -I../intrinsics RGB2Gray.cpp -DVSX4 -mvsx -flax-vector-conversions -Wno-int-to-pointer-cast -g -O2
 * */

#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <power_vsx4.h>
#include "../timing.h"




using namespace vsx;

#define DUMP(v) std::cout << #v << ":" << v << std::endl

#define N 1048576
void serial_rgb2gray(float* ra, float* ga, float* ba, float* gray) {
    for(int i = 0; i < N; i++) {
        gray[i] = 0.3f * ra[i] + 0.59f * ga[i] + 0.11f * ba[i];
    }
}

void svec4_rgb2gray(float* ra, float* ga, float* ba, float* gray ) {

    for(int i = 0; i < N; i+=4) {
        svec4_f a = svec4_f::load((svec4_f*)(ra+i));
        svec4_f b = svec4_f::load((svec4_f*)(ga+i));
        svec4_f c = svec4_f::load((svec4_f*)(ba+i));
        svec4_f out = 0.3f * a  + 0.59f * b  + 0.11f * c ;
        out.store((svec4_f*)(gray+i));
    }
}

void svec4_rgb2gray2(float* ra, float* ga, float* ba, float* gray) {
    for(int i = 0; i < N; i+=4) {
        svec4_f a = svec4_f::load((svec4_f*)(ra+i));
        svec4_f b = svec4_f::load((svec4_f*)(ga+i));
        svec4_f c = svec4_f::load((svec4_f*)(ba+i));
        svec4_f out = svec4_f::smear(0);
        out = svec_madd(svec4_f::smear(0.3), a, out);
        out = svec_madd(svec4_f::smear(0.59), b, out);
        out = svec_madd(svec4_f::smear(0.11), c, out);
        out.store((svec4_f*)(gray+i));
    }
}


float r[N] POST_ALIGN(16);
float g[N] POST_ALIGN(16);
float b[N] POST_ALIGN(16);
float gray[N] POST_ALIGN(16);

int main (int argc, char* argv[])
{
    for(int i = 0; i < N; i++) {
        r[N] = random() % 256;
        g[N] = random() % 256;
        b[N] = random() % 256;
    }
    reset_and_start_stimer();
    serial_rgb2gray(r, g, b, gray);
    double dt = get_elapsed_seconds();
    std::cout<< "serial version: " << dt << " seconds" << std::endl;

    reset_and_start_stimer();
    svec4_rgb2gray(r, g, b, gray);
    double dt2 = get_elapsed_seconds();
    std::cout<< "svec4 version: " << dt2 << " seconds" << std::endl;

    reset_and_start_stimer();
    svec4_rgb2gray2(r, g, b, gray);
    double dt3 = get_elapsed_seconds();
    std::cout<< "svec4 version 2: " << dt3 << " seconds" << std::endl;


    return 0;
}

