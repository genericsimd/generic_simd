/*
 * SimpleTest.cpp
 *
 *  Created on: Jun 12, 2013
 *      Author: haichuan
 */


/*
 *  g++ -I../../include RGB2Gray.cpp -mvsx -flax-vector-conversions -Wno-int-to-pointer-cast -g -O2 -o RGB2Gray
 * */

#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <timing.h>
#ifdef __ALTIVEC__
#include <power_vsx4.h>
using namespace vsx;
#else
#ifdef __SSE4_2__
#include <sse4.h>
using namespace sse;
#else
#include <generic4.h>
using namespace generic;
#endif //__SSE4_2__
#endif //__ALTIVEC__


#define N (1048576)
//#define N (1000000)

//Doesn't work
//__attribute__((optimize("no-tree-vectorize")))

void
#ifdef __SSE4_2__
__attribute__((target("no-sse")))
#endif
serial_rgb2gray(float* ra, float* ga, float* ba, float* gray) {
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

void svec4_rgb2gray_ptr(float* ra, float* ga, float* ba, float* gray ) {

    for(int i = 0; i < N; i+=4) {
        svec4_f a = *(svec4_f*)(ra+i);
        svec4_f b = *(svec4_f*)(ga+i);
        svec4_f c = *(svec4_f*)(ba+i);
        svec4_f out = 0.3f * a  + 0.59f * b  + 0.11f * c ;
        *(svec4_f*)(gray+i) = out;
    }
}

#ifdef __ALTIVEC__
void intrinsics_rgb2gray(float* ra, float* ga, float* ba, float* gray ) {
    __vector float c1 = vec_splats(0.3f);
    __vector float c2 = vec_splats(0.59f);
    __vector float c3 = vec_splats(0.11f);

    for(int i = 0; i < N; i+=4) {
        __vector float a = vec_vsx_ld(0, ra+i);
        __vector float b = vec_vsx_ld(0, ga+i);
        __vector float c = vec_vsx_ld(0, ba+i);
        __vector float out = c1 * a  + c2 * b  +  c3 * c ;
        vec_vsx_st(out, 0, gray+i);
    }
}
#endif

void svec4_rgb2gray_fma(float* ra, float* ga, float* ba, float* gray) {
    for(int i = 0; i < N; i+=4) {
        svec4_f a = svec4_f::load((svec4_f*)(ra+i));
        svec4_f b = svec4_f::load((svec4_f*)(ga+i));
        svec4_f c = svec4_f::load((svec4_f*)(ba+i));
        svec4_f out = 0.3 * a;
        out = svec_madd(svec4_f(0.59), b, out);
        out = svec_madd(svec4_f(0.11), c, out);
        out.store((svec4_f*)(gray+i));
    }
}


float r[N+10000] POST_ALIGN(16);
float g[N+20000] POST_ALIGN(16);
float b[N+30000] POST_ALIGN(16);
float gray[N+40000] POST_ALIGN(16);

#define ITERATIONS 1000
int main (int argc, char* argv[])
{
    for(int i = 0; i < N; i++) {
        r[N] = random() % 256;
        g[N] = random() % 256;
        b[N] = random() % 256;
    }
    std::cout<< "Convert " << N << " pixels RGB to gray." << std::endl;

    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { serial_rgb2gray(r, g, b, gray);}
    double dt = get_elapsed_seconds();
    std::cout<< "serial version: " << dt << " seconds" << std::endl;

    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { svec4_rgb2gray(r, g, b, gray);}
    double dt2 = get_elapsed_seconds();
    std::cout<< "svec4 version: " << dt2 << " seconds" << std::endl;

    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { svec4_rgb2gray_ptr(r, g, b, gray); }
    double dt3 = get_elapsed_seconds();
    std::cout<< "svec4 ptr ld/st version: " << dt3 << " seconds" << std::endl;

#ifdef __ALTIVEC__
    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { intrinsics_rgb2gray(r, g, b, gray);}
    double dt5 = get_elapsed_seconds();
    std::cout<< "Intrinsics version: " << dt5 << " seconds" << std::endl;
#endif

    reset_and_start_stimer();
    for(int i = 0; i < ITERATIONS; i++) { svec4_rgb2gray_fma(r, g, b, gray); }
    double dt4 = get_elapsed_seconds();
    std::cout<< "svec4 fma version: " << dt4 << " seconds" << std::endl;


    return 0;
}

