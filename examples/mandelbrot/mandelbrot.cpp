#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <power_vsx4.h>
//#include <generic4.h>
#include <timing.h>
#include <stdio.h>

using namespace vsx;
//using namespace generic;

/*
  g++ -I../../include mandelbrot.cc -mvsx -flax-vector-conversions -Wno-int-to-pointer-cast -O3 -o mandelbrot
 */

/* 
                Scalar version of mandelbrot 
*/
static int mandel(float c_re, float c_im, int count) {
    float z_re = c_re, z_im = c_im;
    int cci=0;
    for (cci = 0; cci < count; ++cci) {
      if (z_re * z_re + z_im * z_im > 4.f)
	break;

      float new_re = z_re*z_re - z_im*z_im;
      float new_im = 2.f * z_re * z_im;
      z_re = c_re + new_re;
      z_im = c_im + new_im;
    }
    return cci;
}

void mandelbrot_serial(float x0, float y0, float x1, float y1,
                       int width, int height, int maxIterations,
                       int output[])
{
    float dx = (x1 - x0) / width;
    float dy = (y1 - y0) / height;

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; ++i) {
            float x = x0 + i * dx;
            float y = y0 + j * dy;

            int index = (j * width + i);
            output[index] = mandel(x, y, maxIterations);
        }
    }
}


/*
              Generic Intrinsics
 */
void mandelbrot_generic(float x0, float y0, float x1, float y1,
                       int width, int height, int maxIterations,
                       int output[])
{
  typedef svec4_f     vfloat;
  typedef svec4_i32   vint;
  typedef svec4_u32   vuint;
  typedef svec4_i16   vshort;
  typedef svec4_i1    vbool;

  float dx = (x1 - x0) / width;
  float dy = (y1 - y0) / height;

  vfloat v_x0(x0);
  vfloat v_y0(y0);
  vfloat v_x1(x1);
  vfloat v_y1(y1);
  vint vci_4(4);
    
  vfloat v_w((float)width);
  vfloat v_h((float)height);

  vfloat v_dx = (v_x1 - v_x0) / v_w;
  vfloat v_dy = (v_y1 - v_y0) / v_h;
  
  for (int j = 0; j < height; j++) {
    vint v_j(j);
    vint v_i(0,1,2,3);
    vfloat v_i_f(0.0,1.0,2.0,3.0);
    vfloat v_j_f((float)j);
        
    //this is the 'parallel loop'
    for (int i = 0; i < width; i+=4) {
      //float x = x0 + i * dx;
      //float y = y0 + j * dy;
      vfloat v_x = v_x0 + (v_i_f * v_dx);
      vfloat v_y = v_y0 + (v_j_f * v_dy);
      
      //int index = (j * width + i);
      //vint v_index = svec_add(svec_mulo((vshort)v_j,(vshort)v_w),v_i);
      vint v_index = v_j * svec_cast<svec4_i32>(v_w) + v_i;
      
      //   //output[index] = mandel(x, y, maxIterations);
      
      //float z_re = x, z_im = y;
      vfloat v_z_re = v_x;
      vfloat v_z_im = v_y;
      
      int ci=0;
      //float ct_4=4.f;
      //float ct_2=2.f;
      vint vci(0);
      vbool vzero(0);
      vint vci_1(1);
      vfloat v_ct_4(4.f);
      vfloat v_ct_2(2.f);
      
      vbool v_mask(0xffff);
      
      //next stay the same      
      for (ci = 0; ci < maxIterations; ++ci) {
	//if (z_re * z_re + z_im * z_im > ct_4)
	//  break;
	vfloat v_m = v_z_re*v_z_re + v_z_im*v_z_im;
	vbool v_cmp = v_m > v_ct_4; 
	
	//v_mask = vec_andc(v_mask, v_cmp);
	v_mask = v_mask & (~v_cmp);

	int allexit = svec_all_true(v_cmp);
	
	if( allexit ) break;
	
	//here some threads will stop; how do we implement that 
	
	//float new_re = z_re*z_re - z_im*z_im;
	vfloat v_new_re = v_z_re*v_z_re - v_z_im*v_z_im;
	//float new_im = ct_2 * z_re * z_im;
	vfloat v_new_im = v_ct_2 * (v_z_re*v_z_im);
	
	//z_re = x + new_re;
	v_z_re = v_x+v_new_re;
	//z_im = y + new_im;
	v_z_im = v_y + v_new_im;
	
	vint vnci = vci + vci_1;
	vci = svec_select(v_mask, vnci, vci); 
      }
      //store vci
      //output[index] = ci;
      
      int index = (j * width + i);
      vci.store((vint*)(output+index));
      
      //increment vector i
      v_i   = v_i + vci_4;
      v_i_f = v_i_f + v_ct_4;
    }    
  }
}


#ifdef __ALTIVEC__
void mandelbrot_intrinsics(float x0, float y0, float x1, float y1,
                       int width, int height, int maxIterations,
                       int output[])
{
  typedef __vector float        vfloat;
  typedef __vector signed int   vint;
  typedef __vector unsigned int vuint;
  typedef __vector signed short vshort;
  typedef __vector bool int     vbool;

    float dx = (x1 - x0) / width;
    float dy = (y1 - y0) / height;

    vfloat v_x0={x0,x0,x0,x0};
    vfloat v_y0={y0,y0,y0,y0};
    vfloat v_x1={x1,x1,x1,x1};
    vfloat v_y1={y1,y1,y1,y1};
    vint vci_4 = {4,4,4,4};

    vfloat v_w={(float)width,(float)width,(float)width,(float)width};
    vfloat v_h={(float)height,(float)height,(float)height,(float)height};


    vfloat v_dx = vec_div( vec_sub(v_x1,v_x0), v_w);
    vfloat v_dy = vec_div( vec_sub(v_y1,v_y0), v_h);

    for (int j = 0; j < height; j++) {
      vint v_j = {j,j,j,j};
      vint v_i = {0,1,2,3};
      vfloat v_i_f = {0.0,1.0,2.0,3.0};
      vfloat v_j_f = {(float)j,(float)j,(float)j,(float)j};


      //this is the 'parallel loop'
      for (int i = 0; i < width; i+=4) {
    //float x = x0 + i * dx;
    //float y = y0 + j * dy;
    vfloat v_x = vec_add(v_x0,vec_mul(v_i_f,v_dx));
    vfloat v_y = vec_add(v_y0,vec_mul(v_j_f,v_dy));

    //int index = (j * width + i);
    vint v_index = vec_add(vec_mulo((vshort)v_j,(vshort)v_w),v_i);

    //   //output[index] = mandel(x, y, maxIterations);

    //float z_re = x, z_im = y;
    vfloat v_z_re = v_x;
    vfloat v_z_im = v_y;

    int ci=0;
    //float ct_4=4.f;
    //float ct_2=2.f;
    vint vci = {0,0,0,0};
    vbool vzero={0,0,0,0};
    vint vci_1 = {1,1,1,1};
    vfloat v_ct_4 = {4.f,4.f,4.f,4.f};
    vfloat v_ct_2 = {2.f,2.f,2.f,2.f};

    vbool v_mask = {0xffff,0xffff,0xffff,0xffff};

    //next stay the same

    for (ci = 0; ci < maxIterations; ++ci) {
      //if (z_re * z_re + z_im * z_im > ct_4)
      //  break;
      vfloat v_m = vec_add(vec_mul(v_z_re,v_z_re),vec_mul(v_z_im,v_z_im));
      vbool v_cmp = vec_cmpgt(v_m,v_ct_4);

      //v_mask = v_mask & !v_cmp
      v_mask = vec_andc(v_mask, v_cmp);

      int allexit = vec_all_ne(v_cmp, vzero);

      if( allexit ) break;

      //here some threads will stop; how do we implement that

      //float new_re = z_re*z_re - z_im*z_im;
      vfloat v_new_re = vec_sub((vec_mul(v_z_re,v_z_re)),(vec_mul(v_z_im,v_z_im)));
      //float new_im = ct_2 * z_re * z_im;
      vfloat v_new_im = vec_mul(v_ct_2,vec_mul(v_z_re,v_z_im));

      //z_re = x + new_re;
      v_z_re = vec_add(v_x,v_new_re);
      //z_im = y + new_im;
      v_z_im = vec_add(v_y, v_new_im);

      vint vnci = vec_add(vci,vci_1);
      vci = vec_sel(vci, vnci, v_mask);
    }
    //store vci
    //output[index] = ci;

    int index = (j * width + i);
    vec_st(vci, 0, output+index);

    //increment vector i
    v_i = vec_add(v_i, vci_4);
    v_i_f = vec_add(v_i_f, v_ct_4);
      }

    }
}
#endif


/* Write a PPM image file with the image of the Mandelbrot set */
static void
writePPM(int *buf, int width, int height, const char *fn) {
    FILE *fp = fopen(fn, "wb");
    fprintf(fp, "P6\n");
    fprintf(fp, "%d %d\n", width, height);
    fprintf(fp, "255\n");
    for (int i = 0; i < width*height; ++i) {
        // Map the iteration count to colors by just alternating between
        // two greys.
        char c = (buf[i] & 0x1) ? 240 : 20;
        for (int j = 0; j < 3; ++j)
            fputc(c, fp);
    }
    fclose(fp);
    printf("Wrote image file %s\n", fn);
}


static void
writePPM_d(int *buf, int width, int height, const char *fn) {
    for (int i = 0; i < width; ++i) {
      for (int j = 0; j < height; ++j) {
	int index = i*width+j;
	printf("%4d ", buf[index]); 
      }
      printf("\n");
    }
    printf("Wrote image file %s\n", fn);
}


int main() {
  unsigned int width = 768;
  unsigned int height = 512;

  //unsigned int width = 1024;
  //unsigned int height = 1024;

    float x0 = -2;
    float x1 = 1;
    float y0 = -1;
    float y1 = 1;

    int maxIterations = 10;
    int *buf = new int[width*height];

    //
    // Compute the image using the scalar and generic intrinsics implementations; report the minimum
    // time of three runs.
    //

    double minSerial = 1e30;
    for (int i = 0; i < 3; ++i) {
        reset_and_start_stimer();
        mandelbrot_serial(x0, y0, x1, y1, width, height, maxIterations, buf);
        double dt = get_elapsed_seconds();
        minSerial = std::min(minSerial, dt);
    }
    printf("[mandelbrot serial]:\t\t[%.4f] seconds\n", minSerial);
    writePPM(buf, width, height, "mandelbrot-serial.ppm");


    double minGeneric = 1e30;
    for (int i = 0; i < 3; ++i) {
        reset_and_start_stimer();
        mandelbrot_generic(x0, y0, x1, y1, width, height, maxIterations, buf);
        double dt = get_elapsed_seconds();
        minGeneric = std::min(minGeneric, dt);
    }
    printf("[mandelbrot generic simd]:\t\t[%.4f] seconds\n", minGeneric);
    writePPM(buf, width, height, "mandelbrot-generic.ppm");
    printf("[mandelbrot generic speedup]:\t\t%.2fx from GENERIC\n", minSerial/minGeneric);

#ifdef __ALTIVEC__
    double minIntrinsics = 1e30;
    for (int i = 0; i < 3; ++i) {
        reset_and_start_stimer();
        mandelbrot_intrinsics(x0, y0, x1, y1, width, height, maxIterations, buf);
        double dt = get_elapsed_seconds();
        minIntrinsics = std::min(minIntrinsics, dt);
    }
    printf("[mandelbrot intrinsics simd]:\t\t[%.4f] seconds\n", minIntrinsics);
    writePPM(buf, width, height, "mandelbrot-intrinsics.ppm");
    printf("[mandelbrot intrinsics speedup]:\t%.2fx from INTRINSICS\n", minSerial/minIntrinsics);
#endif
    return 0;
}
