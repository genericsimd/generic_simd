/**
 * @brief Test the intrinsics' code mapping with our own code mapping
 * codegen.cpp
 *
 *  Created on: Jul 12, 2013
 *      Author: Haichuan
 */

#include <power_vsx4.h>
using namespace vsx;

static char mem[128] POST_ALIGN(16);
static svec4_i1* p_vi1 = (svec4_i1*)mem;
static svec4_i32* p_vi32 = (svec4_i32*)mem;
static svec4_i64* p_vi64 = (svec4_i64*)mem;
static svec4_f* p_vf = (svec4_f*)mem;
static svec4_d* p_vd = (svec4_d*)mem;

const svec4_i32 base_off(0,1,2,3);

FORCEINLINE svec4_d gather(double* base, svec4_i32 off) {
  int* off_addr = (int*)(&(off.v));

  double d0 = *(base + svec_extract(off, 0));
  double d1 = *(base + svec_extract(off, 1));
  double d2 = *(base + svec_extract(off, 2));
  double d3 = *(base + svec_extract(off, 3));
  return svec4_d(d0, d1, d2, d3);
}

svec4_d test_gather(int scale) {
  svec4_i32 off_ip = scale * base_off;
  return gather((double*)mem, off_ip);
}

FORCEINLINE svec4_d gather_opt(double* base, svec4_i32 off) {
  int* off_addr = (int*)(&(off.v));

  double d0 = *(base + off_addr[0]);
  double d1 = *(base + off_addr[1]);
  double d2 = *(base + off_addr[2]);
  double d3 = *(base + off_addr[3]);
  return svec4_d(d0, d1, d2, d3);
}

svec4_d test_gather_opt(int scale) {
  svec4_i32 off_ip = scale * base_off;
  return gather_opt((double*)mem, off_ip);
}


FORCEINLINE svec4_d gather_stride(double* base, int off0, int off1, int off2, int off3) {
  double d0 = *(base + off0);
  double d1 = *(base + off1);
  double d2 = *(base + off2);
  double d3 = *(base + off3);
  return svec4_d(d0, d1, d2, d3);
}

svec4_d test_gather_stride(int scale) {
  return gather_stride((double*)mem, 0, scale*1, scale*2, scale*3);
}

FORCEINLINE svec4_d gather_stride2(double* base, long long off, long long stride) {
  long long stride2 = stride * 2;
  double d0 = *(base + off);
  double d1 = *(base + off+stride);
  double d2 = *(base + off+stride2);
  double d3 = *(base + off+stride2+stride);
  return svec4_d(d0, d1, d2, d3);
}

svec4_d test_gather_stride2(int scale) {
  return gather_stride2((double*)mem, scale, (long long)scale);
}


FORCEINLINE svec4_d gather_stride3(double* base, long long stride) {
  double d0 = *base;
  base += stride;
  double d1 = *base;
  base += stride;
  double d2 = *base;
  base += stride;
  double d3 = *base;
  return svec4_d(d0, d1, d2, d3);
}

svec4_d test_gather_stride3(int scale) {
  return gather_stride3((double*)(mem+scale), (long long)scale);
}

FORCEINLINE svec4_d gather_stride4(double* base, long long off, long long stride) {
  base += off;
  double d0 = *base;
  base += stride;
  double d1 = *base;
  base += stride;
  double d2 = *base;
  base += stride;
  double d3 = *base;
  return svec4_d(d0, d1, d2, d3);
}

svec4_d test_gather_stride4(int scale) {
  return gather_stride4((double*)mem, scale, (long long)scale);
}

FORCEINLINE svec4_d gather_stride5(double* base, long long stride) {
  long long stride2 = stride * 2;
  double d0 = *(base);
  double d1 = *(base + stride);
  double d2 = *(base + stride2);
  double d3 = *(base + stride2+stride);
  return svec4_d(d0, d1, d2, d3);
}

svec4_d test_gather_stride5(int scale) {
  return gather_stride5((double*)(mem+scale), (long long)scale);
}


int test_access(svec4_i1 v) {
//  li 0,48
//   addi 9,1,-64
//   stxvw4x 34,9,0
//.LBE21:
//   .loc 1 20 0
//   lwa 3,-4(1)
//   blr

//  int i = v_i32[3];


//  li 0,48
//  addi 9,1,-64
//  stxvw4x 34,9,0
//.LBE18:
//  .loc 1 30 0
//  lwa 3,-4(1)
//  blr
//  int i = vec_extract(v_i32.v, 3);


//  li 0,100
//  ld 11,.LC1@toc(2)
//  stw 0,-16(1)
//  addi 10,1,-64
//.LBB28:
//.LBB29:
//.LBB30:
//  .file 2 "../include/power_vsx4.h"
//  .loc 2 1065 0
//  ld 9,.LC2@toc(2)
//.LBE30:
//.LBE29:
//.LBE28:
//  .loc 1 39 0
//  li 0,48
//  lxvw4x 33,0,11
//  lvewx 0,10,0
//  .loc 1 57 0
//  li 3,0
//  .loc 1 39 0
//  vperm 0,2,0,1
//.LVL1:
//.LBB33:
//.LBB32:
//.LBB31:
//  .loc 2 1065 0
//  stxvw4x 32,0,9
//.LBE31:
//.LBE32:
//.LBE33:
//  .loc 1 57 0
//  blr

  v[3] = 15;

  int r = v[2];


//  .loc 1 43 0
//  li 3,0
//.LBB31:
//.LBB30:
//.LBB29:
//  .loc 2 1065 0
//  stxvw4x 34,0,9
//.LBE29:
//.LBE30:
//.LBE31:
//  .loc 1 43 0
//  blr

//  vec_insert(100, v_i32.v, 3);
  v.store(p_vi1);

  return r;
}

void test_broadcasts_64(svec4_d v_d) {
//  v_d[0] = 1.1;

//  li 0,16
//  .loc 2 1107 0
//  lfd 13,-16(1)
//  xxpermdi 0,13,13,0
//  stxvd2x 0,0,9
//  .loc 2 1108 0
//  stxvd2x 0,9,0
//  __vector double splat_d = vec_splat_p7(v_d.v[0], 0);
//  svec4_d nvd(splat_d, splat_d);


//  ld 9,.LC3@toc(2)
//.LBE118:
//.LBE117:
//.LBE116:
//.LBE115:
//  .loc 1 97 0
//  lxvd2x 0,11,0
//.LVL3:
//.LBB130:
//.LBB123:
//.LBB121:
//.LBB119:
//  .loc 2 1174 0
//  li 0,16
//.LVL4:
//.LBE119:
//.LBE121:
//.LBE123:
//.LBB124:
//.LBB125:
//.LBB126:
//.LBB127:
//.LBB128:
//  .file 3 "../include/power9_intrinsics.h"
//  .loc 3 746 0
//#APP
//# 746 "../include/power9_intrinsics.h" 1
//  xxpermdi 0, 0, 0, 0
//# 0 "" 2
//.LVL5:
//#NO_APP
//.LBE128:
//.LBE127:
//.LBE126:
//.LBE125:
//.LBE124:
//.LBB129:
//.LBB122:
//.LBB120:
//  .loc 2 1173 0
//  stxvd2x 0,0,9
//  .loc 2 1174 0
//  stxvd2x 0,9,0
  svec4_d nvd = v_d.broadcast(0);

  nvd.store(p_vd);
//  DUMP(nvd);

}


void test_broadcasts_32(svec4_i32 v_i32) {
//  li 0,48
//  addi 9,1,-80
//  stxvw4x 34,9,0
//  lwz 0,-24(1)
//.LVL3:
//.LBB66:
//.LBB67:
//.LBB68:
//.LBB69:
//  .loc 2 1065 0
//  addi 11,1,-80
//  ld 9,.LC3@toc(2)
//  stw 0,-16(1)
//  stw 0,-12(1)
//  stw 0,-8(1)
//  stw 0,-4(1)
//  li 0,64
//.LVL4:
//  lxvw4x 32,11,0
//  stxvw4x 32,0,9
//.LBE69:
//.LBE68:
//.LBE67:
//.LBE66:
//  .loc 1 99 0
//  blr

//  svec4_i32 vi = v_i32.broadcast(2);



//  li 0,48
//  addi 9,1,-80
//  stxvw4x 34,9,0
//  lwz 0,-24(1)
//.LVL3:
//.LBB46:
//.LBB47:
//.LBB48:
//  .loc 2 1065 0
//  addi 11,1,-80
//  ld 9,.LC3@toc(2)
//  stw 0,-16(1)
//  stw 0,-12(1)
//  stw 0,-8(1)
//  stw 0,-4(1)
//  li 0,64
//.LVL4:
//  lxvw4x 32,11,0
//  stxvw4x 32,0,9
//  v_i32[2] = 100;
 // svec4_i32 vi = svec4_i32(vec_splats(vec_extract(v_i32.v, 2)));


//  .loc 2 1065 0
//  ld 9,.LC3@toc(2)
//.LBE53:
//.LBE52:
//.LBE51:
//.LBB56:
//.LBB57:
//  .file 3 "../include/power9_intrinsics.h"
//  .loc 3 734 0
//#APP
//# 734 "../include/power9_intrinsics.h" 1
//  xxspltw 34, 34,2
//# 0 "" 2
//.LVL3:
//#NO_APP
//.LBE57:
//.LBE56:
//.LBB58:
//.LBB55:
//.LBB54:
//  .loc 2 1065 0
//  stxvw4x 34,0,9
//.LBE54:
//.LBE55:
//.LBE58:
//.LBE50:
//  .loc 1 156 0
//  blr

  svec4_i32 vi = svec4_i32(vec_splat_p7(v_i32.v, 2));

//  DUMP(vi);

  vi.store(p_vi32);
}


void test_splats(int i) {
  //integer
//  svec4_i32 i0(i+1);
//  i0.store(p_vi32);

//  //float
//  svec4_f f0(0.25f);
//  f0.store(p_vf+2);
//
//  //integer

//  .loc 2 1107 0
//  std 3,-16(1)
//  .loc 2 1108 0
//  li 0,16
//  .loc 2 1107 0
//  lfd 13,-16(1)
//  xxpermdi 0,13,13,0
//  stxvd2x 0,0,9
//  .loc 2 1108 0
  svec4_i64 i1(vec_splats((signed long long)(i+2)), vec_splats((signed long long)(i+2)));
  i1.store(p_vi64);

//
//  //float
//  svec4_f f1(vec_splats(0.5f));
//   f1.store(p_vf+3);
}




int main(int argc, char* argv[])
{
  int j = 0;
  svec4_i32 v_i32 =  * p_vi32;
  svec4_d v_d =  * p_vd;
    //test_splats(argc);
  svec4_i1 v_i1 =  * p_vi1;
    test_access(v_i1);
//  test_broadcasts_32(v_i32);
//  test_broadcasts_64(v_d);

    DUMP(test_gather(argc+1));
    DUMP(test_gather_opt(argc+1));
    DUMP(test_gather_stride2(argc+1));
    DUMP(test_gather_stride3(argc+1));
    DUMP(test_gather_stride4(argc+1));
    DUMP(test_gather_stride5(argc+1));

    return 0;
}


